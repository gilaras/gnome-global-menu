/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Yu Feng 2007 <rainwoodman@localhost.localdomain>
 * 
 * main.c is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * main.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with main.c.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <config.h>

#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#undef WNCK_I_KNOW_THIS_IS_UNSTABLE

#include <panel-applet.h>
/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include "typedefs.h"
#include "gtkmenuembed-x11.h"
#include "application.h"
#include "menuclients.h"
#include "menuserver.h"
#include "ui.h"
#include "preference.h"

typedef struct _ClientInfo{
	MenuClient * menu_client;
	GdkWindow * float_window;
	int x;
	int y;
} ClientInfo;
static void return_client_float_window(ClientInfo * client, Application * App){
	g_print("Return client float window\n");
	GlobalMenuNotify notify;
	notify.type = GM_NOTIFY_SET_VISIBLE;
	notify.SetVisible.visible = FALSE;
	menu_server_send_to(App->Server, client->menu_client, &notify);	
	gdk_window_reparent(client->float_window, 
		gtk_widget_get_root_window(GTK_WIDGET(App->Holder)), 0, 0);
}
static void steal_client_float_window(ClientInfo * client, Application * App){
	g_print("Steal client float window\n");
	GlobalMenuNotify notify;
	notify.type = GM_NOTIFY_SET_VISIBLE;
	notify.SetVisible.visible = TRUE;
	gdk_window_reparent(client->float_window, 
		GTK_WIDGET(App->Holder)->window, client->x, client->y);
	menu_server_send_to(App->Server, client->menu_client, &notify);	
}
static void active_window_changed_cb(WnckScreen* screen, WnckWindow *previous_window, Application * App){
	WnckWindow * active_window = NULL;
	Window active_xid = 0; 
	gboolean client_known = FALSE;
	ClientInfo * info;
	GList * node = NULL;

	if(App->ActiveTitle){
		g_free(App->ActiveTitle);
		App->ActiveTitle = NULL;
	}
	if(App->ActiveIcon){
		g_object_unref(App->ActiveIcon);
		App->ActiveIcon = NULL;
	}

	active_window = wnck_screen_get_active_window(screen);

	g_print("Active Window_changed\n");
	if(WNCK_IS_WINDOW(active_window)){
		active_xid = wnck_window_get_xid(active_window);
		App->ActiveTitle = g_strdup(wnck_window_get_name(active_window));
		App->ActiveIcon = wnck_window_get_icon(active_window);
		g_object_ref(App->ActiveIcon);
		g_print("Active XWin ID is %p\n", (gpointer) active_xid);
		for(node = g_list_first(App->Clients); node ; node = g_list_next(node)){
			if(((ClientInfo*)node->data)->menu_client->master_xid == active_xid){
				client_known = TRUE;
				break;
			}
		}
				
	}else {
		g_print("Active Window is not a window, that's Stupid!\n");
	}
	if(App->ActiveClient){
		return_client_float_window(App->ActiveClient, App);
		App->ActiveClient = NULL;
	}
	if(client_known){
		info = (ClientInfo*)node->data;
		g_print("Client menubar found\n");
		if(App->ActiveClient != info){
			if(App->ActiveClient)
				return_client_float_window(App->ActiveClient, App);
			steal_client_float_window(info, App);
			App->ActiveClient = info;
		}
	}
	ui_repaint_all(App);
}

static void window_opened_cb(WnckScreen* screen, WnckWindow *window, Application * App){
	g_print("Window opened: %s\n", wnck_window_get_name(window));
}
static void window_closed_cb(WnckScreen* screen, WnckWindow *window, Application * App){
	g_print("Window closed: %s\n", wnck_window_get_name(window));
}

static void client_new_cb(MenuServer * server, MenuClient * client, Application * App){
	ClientInfo * info = g_new0(ClientInfo, 1);
	GlobalMenuNotify notify;
	GtkAllocation * allocation;
	g_print("Applet: Client New:%p\n", (void*)client->client_xid);
	info->menu_client = client;
	info->float_window = gdk_window_foreign_new(client->float_xid);
	info->x = 0;
	info->y = 0;
/*since we don't want it be shown in the screen, perhaps its better to hide it in the menubar patch**/
	gdk_window_hide(info->float_window);
	allocation = &GTK_WIDGET(App->Holder)->allocation;
	notify.type = GM_NOTIFY_SIZE_ALLOCATE;
	notify.SizeAllocate.width = allocation->width;
	notify.SizeAllocate.height = allocation->height;
	g_message("client_new_cb: allocate->w, h = %d, %d", allocation->width,
			allocation->height);
	menu_server_send_to(App->Server, info->menu_client, &notify);
	App->Clients = g_list_append(App->Clients, info);
}
static void client_destroy_cb(MenuServer * server, MenuClient * client, Application * App){
	GList * node = NULL;
	ClientInfo * info = NULL;
	g_print("Applet: Client Destroy\n");
	for(node = g_list_first(App->Clients); node; node = g_list_next(node)){
		if(((ClientInfo*) node->data)->menu_client == client){
			info = (ClientInfo*) (node->data);
			break;
		}
	}	
	g_return_if_fail( node );
	g_return_if_fail( info );
/*****special deal if it is the current application********/
	if(info == App->ActiveClient) App->ActiveClient = NULL;
/* However, We need to tell our gdk this window no longer exists. I hope gdk_iwndow_unref will deal with it if the window still exists.*/
	gdk_window_unref(info->float_window);
	App->Clients = g_list_remove_all(App->Clients, info);
	g_free(info);
}
static void application_free(Application * App);
static gboolean main_window_destroy_cb(GtkWindow * MainWindow, Application * App){
	g_print("Main window destroyed.\n");
	application_free(App);
	return TRUE;
}
static gboolean main_window_delete_cb(GtkWindow * MainWindow, GdkEvent * event, Application * App){
	g_print("Main window delete-event.\n");
	return FALSE;
}
static void main_window_change_background_cb(PanelApplet * applet, PanelAppletBackgroundType type, GdkColor * color, GdkPixmap * pixmap, Application * App){
	GtkStyle * style;
	Window bg_xid;
	Atom color_atom;
	GlobalMenuNotify notify;
	gchar * color_name = NULL;
	g_print("Applet Background has changed\n");
/*Should notify the client!*/
	GdkWindow * window = GTK_WIDGET(App->Holder)->window;
	g_return_if_fail(window);
	switch(type){
		case PANEL_NO_BACKGROUND:{
			style = gtk_widget_get_style(GTK_WIDGET(App->MainWindow));
			gdk_window_set_back_pixmap(window, NULL, FALSE);
			gtk_widget_modify_bg(GTK_WIDGET(App->Holder), GTK_STATE_NORMAL, &style->bg[GTK_STATE_NORMAL]);
			color_name = gdk_color_to_string(&style->bg[GTK_STATE_NORMAL]);
			break;
		}
		case PANEL_COLOR_BACKGROUND:
			gdk_window_set_back_pixmap(window, NULL, FALSE);
			gtk_widget_modify_bg(GTK_WIDGET(App->Holder), GTK_STATE_NORMAL, color);
			color_name = gdk_color_to_string(color);
			break;
		case PANEL_PIXMAP_BACKGROUND:
			{
			GdkPixmap * holder_bg;
			GdkGC * gc;
			gint x, y;
			gint w, h;
			gdk_window_get_position(window, &x, &y);
			gdk_window_get_size(window, &w, &h);
			g_print("%d, %d, %d, %d\n", x, y, w, h);
			holder_bg = gdk_pixmap_new(pixmap, w, h, -1);
			gc = gdk_gc_new(pixmap);
			gdk_draw_drawable(holder_bg, gc, pixmap, x, y, 0, 0, w, h);
			gdk_window_set_back_pixmap(window, holder_bg, FALSE);
			g_object_unref(gc);
			bg_xid = GDK_PIXMAP_XID(holder_bg);
			}
			break;
	}
	gtk_widget_queue_draw(GTK_WIDGET(App->Holder));
	if(color_name){
		color_atom = gdk_x11_get_xatom_by_name(color_name);
		g_free(color_name);
	} else 
		color_atom = 0;
	notify.SetBackground.color_atom = color_atom;
	notify.SetBackground.pixmap_xid = bg_xid;
	notify.type = GM_NOTIFY_SET_BACKGROUND;
	menu_server_broadcast(App->Server, &notify);
}
static void main_window_change_orient_cb(PanelApplet * applet, PanelAppletOrient orient, Application * App){
	g_print("Applet Orientation has changed\n");
}

static void holder_resize_cb(GtkWidget * widget, GtkAllocation * allocation, Application * App){
	g_print("Holder resizesd.\n");
	GlobalMenuNotify notify;
	g_print("holder_resize_cb:Broadcast message to all clients: %d, %d, %d, %d\n", *allocation);
	if (App->ActiveClient) {
		notify.type = GM_NOTIFY_SIZE_ALLOCATE;
		notify.SizeAllocate.width = allocation->width;
		notify.SizeAllocate.height = allocation->height;
		menu_server_broadcast(App->Server, &notify);
	}

}
static gboolean label_area_action_cb(GtkWidget * widget, GdkEventButton* button, Application * App){
	g_print("client icon action.\n");
	if(button->button == 1){
		gboolean rt;
		g_signal_emit_by_name(G_OBJECT(App->MainWindow), "popup_menu", &rt);
	}
	return TRUE;
}
static gboolean backward_action_cb(GtkWidget * widget, GdkEventButton * button, Application * App){
	GtkAllocation *allocation;
	GlobalMenuNotify notify;
	g_print("backward action.\n");
	if(App->ActiveClient){
		App->ActiveClient->x -= 10;
 
		memset(&notify, 0 ,sizeof(notify));
		allocation = &(GTK_WIDGET(App->Holder))->allocation;
		notify.type = GM_NOTIFY_SIZE_ALLOCATE;
		notify.SizeAllocate.width = allocation->width - App->ActiveClient->x;
		notify.SizeAllocate.height = allocation->height;
		menu_server_send_to(App->Server, App->ActiveClient->menu_client, &notify);
 
		gdk_window_move(App->ActiveClient->float_window, 
				App->ActiveClient->x,
				App->ActiveClient->y);
	}
	ui_repaint_all(App);
	return TRUE;
}
static gboolean forward_action_cb(GtkWidget * widget, GdkEventButton * button, Application * App){
	GtkAllocation *allocation;
	GlobalMenuNotify notify;
	g_print("forward action.\n");
	if(App->ActiveClient){
		App->ActiveClient->x += 10;
		allocation = &(GTK_WIDGET(App->Holder))->allocation;
		if ( allocation->width - App->ActiveClient->x > 0)
		{
			memset(&notify, 0 ,sizeof(notify));
			notify.type = GM_NOTIFY_SIZE_ALLOCATE;
			notify.SizeAllocate.width = allocation->width - App->ActiveClient->x;
			notify.SizeAllocate.height = allocation->height;
			menu_server_send_to(App->Server, App->ActiveClient->menu_client, &notify);
		}
		gdk_window_move(App->ActiveClient->float_window, 
				App->ActiveClient->x,
				App->ActiveClient->y);
	}
	ui_repaint_all(App);
	return TRUE;
}
static void popup_menu_cb(BonoboUIComponent * uic, Application * App, gchar * cname){
	g_message("%s: cname = %s", __func__, cname);
	if(g_str_equal(cname, "About")) ui_show_about(App);
	if(g_str_equal(cname, "Preference")) preference_show_dialog(App);
}
static Application * application_new(GtkContainer * mainwindow){
	Application * App = g_new0(Application, 1);
	GdkScreen * gdkscreen = NULL;
	UICallbacks callback_table;
	
	preference_load_conf(App);

	App->Server = menu_server_new();
	App->Clients = NULL;
	App->ActiveClient = NULL;
	App->ActiveTitle = NULL;
	App->ActiveIcon = NULL;

	menu_server_set_user_data(App->Server, App);
	menu_server_set_callback(App->Server, 
		MS_CB_CLIENT_NEW, 
		(MenuServerCallback) client_new_cb);
	menu_server_set_callback(App->Server, 
		MS_CB_CLIENT_DESTROY, 
		(MenuServerCallback) client_destroy_cb);
	App->MainWindow = mainwindow;

	g_signal_connect(G_OBJECT(App->MainWindow), "destroy",
		G_CALLBACK(main_window_destroy_cb), App);
/******The delete-event not useful any more, since panel-applet dont receive it. *******/
	g_signal_connect(G_OBJECT(App->MainWindow), "delete-event",
		G_CALLBACK(main_window_delete_cb), App);
	g_signal_connect(G_OBJECT(App->MainWindow), "change-background",
			G_CALLBACK(main_window_change_background_cb), App);
	g_signal_connect(G_OBJECT(App->MainWindow), "change-orient",
			G_CALLBACK(main_window_change_orient_cb), App);
	panel_applet_set_background_widget(App->MainWindow, App->MainWindow);
/**The Screen***/
	gdkscreen = gtk_widget_get_screen(GTK_WIDGET(App->MainWindow));
	g_print("GDK SCREEN number is %d\n", gdk_screen_get_number(gdkscreen));
	App->Screen = wnck_screen_get(gdk_screen_get_number(gdkscreen));
	g_print("App->Screen is %p\n",App->Screen);

	g_signal_connect(G_OBJECT(App->Screen), "active-window-changed", 
		G_CALLBACK(active_window_changed_cb), App);
	g_signal_connect(G_OBJECT(App->Screen), "window-opened", 
		G_CALLBACK(window_opened_cb), App);
	g_signal_connect(G_OBJECT(App->Screen), "window-closed", 
		G_CALLBACK(window_closed_cb), App);

/******Create the UI *****/
	callback_table.label_area_action_cb = G_CALLBACK(label_area_action_cb);
	callback_table.forward_action_cb = G_CALLBACK(forward_action_cb);
	callback_table.backward_action_cb = G_CALLBACK(backward_action_cb);
	callback_table.holder_resize_cb = G_CALLBACK(holder_resize_cb);
	callback_table.popup_menu_cb = (BonoboUIVerbFn)popup_menu_cb;

	ui_create_all(App, &callback_table);

/**********All done**********/

	gtk_widget_show_all(GTK_WIDGET(mainwindow));
	/*I think it is not nessary since we have an app icon already. Maybe can let user choose whether use TitleLabel or Icon*/
	gtk_widget_hide(GTK_WIDGET(App->TitleLabel)); 
	/*if we have registered the signals of App->Screen, all clients can be discovered in this function. if we haven't, here we can not find any windows.*/
	
	menu_server_start(App->Server);
	return App;
}
static void application_free(Application * App){
	g_signal_handlers_disconnect_by_func(App->Screen, active_window_changed_cb, App);
	g_signal_handlers_disconnect_by_func(App->Screen, window_opened_cb, App);
	g_signal_handlers_disconnect_by_func(App->Screen, window_closed_cb, App);
	if(App->ActiveClient){ /*Perhaps not useful, since when we reach here,
							 the a delete-event has been sent to
                             activeclient->float_window*/
		/*So don't return active client, just forget it and let the application itself handle it*/
		//return_client_float_window(App->ActiveClient, App);
		App->ActiveClient = NULL;
	}
	menu_server_shutdown(App->Server);
	menu_server_free(App->Server);
	g_free(App);
}



#define FACTORY_IID "OAFIID:GNOME_GlobalMenuApplet2_Factory"
#define APPLET_IID "OAFIID:GNOME_GlobalMenuApplet2"

#define APP_NAME "globalmenu-applet"
#define APP_VERSION "2"

static gboolean globalmenu_applet_factory (PanelApplet *applet,
                                        const gchar *iid,
                                        gpointer data){
	Application * App;
  if (g_str_equal(iid, APPLET_IID)){
	panel_applet_set_flags(applet, 
		PANEL_APPLET_EXPAND_MAJOR | PANEL_APPLET_EXPAND_MINOR | PANEL_APPLET_HAS_HANDLE);
    App = application_new(GTK_CONTAINER(applet));
    return TRUE;
  } else {
	return FALSE;
	}
}

int main (int argc, char *argv [])
{
	GnomeProgram *program;
	GOptionContext *context;
	int           retval;
#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	
	context = g_option_context_new("");
	program = gnome_program_init (APP_NAME, APP_VERSION,
			LIBGNOMEUI_MODULE,
			argc, argv,
			GNOME_PARAM_GOPTION_CONTEXT, context,	
			GNOME_CLIENT_PARAM_SM_CONNECT, FALSE,	
			GNOME_PARAM_NONE);
	retval = panel_applet_factory_main (FACTORY_IID, PANEL_TYPE_APPLET, globalmenu_applet_factory, NULL);
	g_object_unref (program);
	return retval;
}
