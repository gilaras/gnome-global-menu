#include <config.h>
#include <gtk/gtk.h>

#if ENABLE_TRACING >= 1
#define LOG(fmt, args...) g_printerr("<IPCServer>::" fmt "\n",  ## args)
#else
#define LOG(fmt, args...)
#endif
#define LOG_FUNC_NAME LOG("%s", __func__)

#include <gdk/gdkx.h>
#include "ipc.h"
#include "ipcutils.h"
#include "ipcserver.h"
#include "ipcserverbase.h"
#include "ipccommand.h"
#include "ipcdispatcher.h"

static GStringChunk * string_list = NULL;
static GdkWindow * server_window = NULL;
static gboolean server_frozen = TRUE;

static GHashTable * client_hash = NULL;
static ClientDestroyCallback client_destroy_callback = NULL;
static ClientCreateCallback client_create_callback = NULL;
static gpointer callback_data = NULL;

const gchar * ipc_server_define_string(const gchar * string) {
	return g_string_chunk_insert_const(string_list, string);
}
static void client_info_free(gpointer data){
	ClientInfo * info = data;
	g_hash_table_remove(client_hash, info->cid);
//	gdk_window_destroy(info->window);
	g_datalist_clear(&info->events.installed);
	g_datalist_clear(&info->events.listening); /*FIXME: unref these events*/
	g_slice_free(ClientInfo, info);
}
static void installed_event_info_free(gpointer data){
	InstalledEventInfo * info = data;
	g_slice_free(InstalledEventInfo, info);
}
static void listening_event_info_free(gpointer data){
	ListeningEventInfo * info = data;
	g_slice_free(ListeningEventInfo, info);
}

static GdkFilterReturn default_filter (GdkXEvent * xevent, GdkEvent * event, gpointer data);

static gchar * ipc_server_call_client_xml(ClientInfo * info, gchar * xml){
	gchar * ret_xml;

	gdk_error_trap_push();
	ipc_set_property(info->xwindow, IPC_PROPERTY_SERVERCALL, xml);	
	ipc_send_client_message(GDK_WINDOW_XWINDOW(server_window), info->xwindow, IPC_CLIENT_MESSAGE_CALL);
	if(gdk_error_trap_pop()) {
		g_warning("could not set the property for calling the command, ignoring the command");
		goto no_prop_set;
	}
	ret_xml = ipc_wait_for_property(info->xwindow, IPC_PROPERTY_SERVERRETURN, TRUE);
	if(!ret_xml) {
		g_warning("No return value obtained");
		goto no_return_val;
	}
no_return_val:
no_prop_set:
	return ret_xml;

}
static gboolean ipc_server_call_client_command(IPCCommand * command) {
	ClientInfo * info = g_hash_table_lookup(client_hash, ipc_command_get_target(command));
	LOG("target = %s", ipc_command_get_target(command));
	g_return_val_if_fail(info, FALSE);
	gchar * xml = ipc_command_to_string(command);
	gchar * ret_xml = NULL;
	ret_xml = ipc_server_call_client_xml(info, xml);
	IPCCommand * returns = ipc_command_parse(ret_xml);
	g_free(xml);
	XFree(ret_xml);
	if(!returns) {
		g_warning("malformed return value");
		return FALSE;
	} else {
		ipc_command_steal(command, returns);
		return TRUE;
	}
	return FALSE;
}

gboolean ipc_server_boot(ClientCreateCallback cccb, ClientDestroyCallback cdcb, gpointer data) {
	string_list = g_string_chunk_new(0);
	gdk_x11_grab_server();
	GdkNativeWindow old_server = ipc_find_server();
	if(old_server) return FALSE;
	GdkWindowAttr attr;
	attr.title = IPC_SERVER_TITLE;
	attr.wclass = GDK_INPUT_ONLY;
	server_window = gdk_window_new(NULL, &attr, GDK_WA_TITLE);
	gdk_window_set_events(server_window, GDK_STRUCTURE_MASK || gdk_window_get_events(server_window));
	gdk_window_add_filter(server_window, default_filter, NULL);
	server_frozen = FALSE;
	gdk_flush();
	XSync(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), FALSE);
	gdk_x11_ungrab_server();

	client_hash = g_hash_table_new(g_str_hash, g_str_equal);
	client_destroy_callback = cdcb;	
	client_create_callback = cccb;	
	callback_data = data;	
	return TRUE;
}
void ipc_server_freeze() {
	server_frozen = TRUE;
}
void ipc_server_thaw() {
	server_frozen = FALSE;
}
static void client_message_call(ClientInfo * info, XClientMessageEvent * client_message) {
	Display * display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default()) ;
	gpointer data;
//	gdk_x11_grab_server();
	data = ipc_get_property(info->xwindow, IPC_PROPERTY_CALL);
	if(!data) {
		g_warning("could not obtain call information, ignoring the call");
		goto no_prop;
	}
	GList * commands = ipc_command_list_parse(data);
	XFree(data);
	if(!commands){
		g_warning("malformed command, ignoring the call");
		goto parse_fail;
	}
	GList * node;
	for(node = commands; node; node=node->next){
		IPCCommand * command = node->data;
		if(info->cid != SVR_STR(ipc_command_get_source(command))) {
			g_warning("unknown client, ignoring the call: cid = %s from =%s", 
					info->cid, ipc_command_get_source(command));
			goto unknown_client;
		}
		if(g_str_equal("SERVER", ipc_command_get_target(command))) {
			if(!ipc_dispatcher_call_cmd(command)) {
				IPCFail(command);
				g_warning("command was not successfull, ignoring the call");
			}
		} else {
			if(!ipc_server_call_client_command(command)){
				IPCFail(command);
			}
		}
	}
	gchar * ret = ipc_command_list_to_string(commands);
	gdk_error_trap_push();

	ipc_set_property(info->xwindow, IPC_PROPERTY_RETURN, ret);
	if(gdk_error_trap_pop()) {
		g_warning("could not set the property for returing the command");
	}
	g_free(ret);
unknown_client:
call_fail:
	ipc_command_list_free(commands);
parse_fail:
no_prop:
//	gdk_x11_ungrab_server();
	return;
}

static GdkFilterReturn client_filter(GdkXEvent * xevent, GdkEvent * event, gpointer data){
	ClientInfo * info = data;
	if(((XEvent *)xevent)->type == DestroyNotify) {
		XDestroyWindowEvent * dwe = (XDestroyWindowEvent *) xevent;
		LOG("client %s is down!", info->cid);
		gdk_window_remove_filter(info->window, client_filter, info);
		if(client_destroy_callback)
			client_destroy_callback(info->cid, callback_data);
		client_info_free(data);
	} else {
	}
	return GDK_FILTER_CONTINUE;
}
static void client_message_nego(ClientInfo * unused, XClientMessageEvent * client_message) {
	static guint id = 1000;
	GdkNativeWindow src = * ((GdkNativeWindow *) (&client_message->data.b));
	Display * display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default()) ;
	gchar * identify = g_strdup_printf("%d", id++);
	
	ClientInfo * client_info = g_slice_new0(ClientInfo);
	client_info->xwindow = src;
	g_datalist_init(&(client_info->events.listening));
	g_datalist_init(&(client_info->events.installed));
	gdk_x11_grab_server();
	client_info->window = gdk_window_lookup(src);
	if(!client_info->window) client_info->window = gdk_window_foreign_new(src);
	client_info->cid = SVR_STR(identify);
	gdk_error_trap_push();
	
	ipc_set_property(src, IPC_PROPERTY_CID, identify);

	g_hash_table_insert(client_hash, client_info->cid, client_info);
	gdk_window_set_events(client_info->window, gdk_window_get_events(client_info->window) | GDK_STRUCTURE_MASK);
	gdk_window_add_filter(client_info->window, client_filter, client_info);
	if(gdk_error_trap_pop()) {
		g_warning("could not set the identify during NEGO process");
	}
	gdk_x11_ungrab_server();
	if(client_create_callback)
		client_create_callback(client_info->cid, callback_data);
}
static void get_info_from_xwindow_foreach(gpointer key, gpointer value, gpointer foo[]){
	ClientInfo * info = value;
	if(info->xwindow == *((GdkNativeWindow *)foo[0])) foo[1] = info;
}
static ClientInfo * get_info_from_xwindow(GdkNativeWindow window){
	gpointer foo[] = {
		&window,
		NULL	
	};
	g_hash_table_foreach(client_hash, get_info_from_xwindow_foreach, foo);
	return foo[1];
}
static GdkFilterReturn default_filter (GdkXEvent * xevent, GdkEvent * event, gpointer data){
	if(server_frozen) return GDK_FILTER_CONTINUE;
	XClientMessageEvent * client_message = (XClientMessageEvent *) xevent;
#define GET_INFO \
			GdkNativeWindow src = * ((GdkNativeWindow *) (&client_message->data.b)); \
			ClientInfo * info = get_info_from_xwindow(src);

	switch(((XEvent *)xevent)->type) {
		case ClientMessage:
			if(client_message->message_type == gdk_x11_atom_to_xatom(IPC_CLIENT_MESSAGE_CALL)) {
				GET_INFO;
				client_message_call(info, client_message);
				return GDK_FILTER_REMOVE;
			} else
			if(client_message->message_type == gdk_x11_atom_to_xatom(IPC_CLIENT_MESSAGE_NEGO)){
				client_message_nego(NULL, client_message);
				return GDK_FILTER_REMOVE;
			}
		return GDK_FILTER_CONTINUE;
	}
	return GDK_FILTER_CONTINUE;
}
static void ipc_server_send_client_message(GdkNativeWindow client_xwindow, GdkAtom message_type) {
	ipc_send_client_message(GDK_WINDOW_XWINDOW(server_window), client_xwindow, message_type);
}
static gboolean ipc_server_send_event_to(GdkNativeWindow xwindow, IPCEvent * event) {
	Display * display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default()) ;
	gchar * data = ipc_event_to_string(event);
	if(!data) {
		g_critical("Could not format the event");
		return FALSE;
	}
	gdk_error_trap_push();
	gdk_x11_grab_server(); /*ensure the client is not taking away the unprocessed event*/
	gchar * old_event_data = ipc_get_property(xwindow, IPC_PROPERTY_EVENT);
	gchar * new_event_data;
	if(old_event_data) {
		new_event_data = g_strdup_printf("%s\n%s", old_event_data, data);
		XFree(old_event_data);
	} else
		new_event_data = g_strdup(data);
	LOG("event_data: %s", new_event_data);
	g_free(data);
	ipc_set_property(xwindow, IPC_PROPERTY_EVENT, new_event_data);
	gdk_x11_ungrab_server();
	g_free(new_event_data);
	ipc_server_send_client_message(xwindow, IPC_CLIENT_MESSAGE_EVENT);
	LOG("sending to %X", xwindow);
	if(gdk_error_trap_pop()) {
		g_warning("could not set the property for the event");
		goto set_prop_fail;
	}
	return TRUE;
set_prop_fail:
	return FALSE;
}
/**
 * ipc_server_send_event:
 *
 * FIXME: respect the filters.
 *
 */
gboolean ipc_server_send_event(IPCEvent * event) {
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, client_hash);
	ClientInfo * info;
	const gchar * cid;
	while(g_hash_table_iter_next(&iter, (gpointer*) &cid, (gpointer*)&info)){
		LOG("sending to %s(%s)", info->cid, cid);
		ipc_server_send_event_to(info->xwindow, event);
	}
	return TRUE;
}
