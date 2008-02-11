#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <string.h>

#include "gdksocket.h"
#include "gnomenu-marshall.h"

#ifndef GDK_WINDOWING_X11
#error ONLY X11 is supported. other targets are not yet supported.
#endif

#define LOG_FUNC_NAME g_message(__func__)

#define GDK_SOCKET_ATOM_STRING "GDK_SOCKET_MESSAGE"

enum {
	PROP_0,
	PROP_NAME,
	PROP_TARGET,
	PROP_STATUS
};
enum {
	DATA_ARRIVAL,
	CONNECT_REQ,
	SHUTDOWN,
	SIGNAL_MAX,
};
typedef struct _GdkSocketPrivate GdkSocketPrivate;
struct _GdkSocketPrivate {
	gboolean disposed;
	int foo;
};
typedef enum {
	GDK_SOCKET_BROADCAST, /*NOT IMPLEMENTED, because of the limitation of GdkFilter*/
	GDK_SOCKET_CONNECT_REQ,
	GDK_SOCKET_ACK,
	GDK_SOCKET_DATA,
	GDK_SOCKET_SHUTDOWN,
} GdkSocketHeaderType;

typedef struct _GdkSocketMessage {
	GdkSocketHeaderType header;
	GdkSocketNativeID source;
	gulong l[3];
} GdkSocketMessage;

#define GDK_SOCKET_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE(obj, GDK_TYPE_SOCKET, GdkSocketPrivate))

static GObject * _constructor
			(GType type, guint n_construct_properties, GObjectConstructParam *construct_params);
static void _dispose(GObject * object);
static void _finalize(GObject * object);
static void _data_arrival (GdkSocket * socket, gpointer data, guint size);
static void _connect_req (GdkSocket * socket, GdkSocketNativeID target);
static void _shutdown (GdkSocket * socket);

static void _set_property
			(GObject * object, guint property_id, const GValue * value, GParamSpec * pspec);
static void _get_property
			(GObject * object, guint property_id, GValue * value, GParamSpec * pspec);

static gboolean _raw_send(GdkSocket * self, GdkNativeWindow target, gpointer data, guint bytes);
static gboolean _raw_send_nosync(GdkSocket * self, GdkNativeWindow target, gpointer data, guint bytes);
static gboolean _raw_broadcast_by_name(GdkSocket * self, gchar * name, gpointer data, guint bytes);
static void _destroy_on_shutdown(GdkSocket * self, gpointer userdata);
G_DEFINE_TYPE (GdkSocket, gdk_socket, G_TYPE_OBJECT)
static gulong class_signals[SIGNAL_MAX] = {0};
static GdkFilterReturn 
	_window_filter_cb			(GdkXEvent* xevent, GdkEvent * event, gpointer data);

/**
 * gdk_socket_class_init:
 *
 * Initialize the class structure of #GdkSocket
 */
static void
gdk_socket_class_init(GdkSocketClass * klass){
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);
	GParamSpec * pspec;

	LOG_FUNC_NAME;

	g_type_class_add_private(gobject_class, sizeof (GdkSocketPrivate));

	gobject_class->dispose = _dispose;
	gobject_class->constructor = _constructor;
	gobject_class->finalize = _finalize;
	gobject_class->get_property = _get_property;
	gobject_class->set_property = _set_property;

	klass->data_arrival = _data_arrival;
	klass->connect_req = _connect_req;
	klass->shutdown = _shutdown;

	class_signals[DATA_ARRIVAL] =
/**
 * GdkSocket::data-arrival:
 * @self: the #GdkSocket that receives this signal.
 * @data: the received data. It is owned by @self and the signal handler 
 * 		should not free it.
 Gdk* @bytes: the length of received data.
 *
 * The ::data-arrival signal is emitted each time a message arrives to
 * the socket.
 */
		g_signal_new ("data-arrival",
			G_TYPE_FROM_CLASS (klass),
			G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GdkSocketClass, data_arrival),
			NULL /* accumulator */,
			NULL /* accu_data */,
			gnomenu_marshall_VOID__POINTER_UINT,
			G_TYPE_NONE /* return_type */,
			2     /* n_params */,
			G_TYPE_POINTER,
			G_TYPE_UINT
			);

	class_signals[CONNECT_REQ] =
/**
 * GdkSocket::connect-request:
 * @self: the #GdkSocket that receives this signal.
 * @data: the received data. It is owned by @self and the signal handler 
 * 		should not free it.
 * @bytes: the length of received data.
 *
 * The ::data-arrival signal is emitted each time a message arrives to
 * the socket.
 */
		g_signal_new ("connect-request",
			G_TYPE_FROM_CLASS (klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GdkSocketClass, connect_req),
			NULL /* accumulator */,
			NULL /* accu_data */,
			gnomenu_marshall_VOID__UINT,
			G_TYPE_NONE /* return_type */,
			1     /* n_params */,
			G_TYPE_UINT
			);
	class_signals[SHUTDOWN] =
/**
 * GdkSocket::shutdown
 * @self:
 * 
 * invoked when the socket is shutdown from the other peer.
 */
		g_signal_new ("shutdown",
			G_TYPE_FROM_CLASS (klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GdkSocketClass, shutdown),
			NULL,
			NULL,
			gnomenu_marshall_VOID__VOID,
			G_TYPE_NONE,
			0);

/**
 * GdkSocket:name:
 *
 * the name of the socket
 */
	g_object_class_install_property (gobject_class, 
			PROP_NAME,
			g_param_spec_string ("name",
						"GdkSocket name prop",
						"Set GdkSocket's name",
						"GdkSocket",
						G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));
			
/**
 * GdkSocket:target:
 *
 * the target to connect. #GdkSocketNativeID
 */
	g_object_class_install_property (gobject_class, 
			PROP_TARGET,
			g_param_spec_uint ("target",
						"GdkSocket target prop",
						"Set GdkSocket's target",
						0, G_MAXUINT, 0, 
						G_PARAM_READWRITE));
/**
 * GdkSocket::status
 *
 * the status of the socket. #GdkSocketNativeID
 */
	g_object_class_install_property (gobject_class, 
			PROP_STATUS,
			g_param_spec_enum ("status",
						"GdkSocket status prop",
						"Set GdkSocket's status",
						gdk_socket_status_get_type(),
						GDK_SOCKET_DISCONNECTED,
						G_PARAM_READWRITE));
}

static GObject* _constructor(GType type, guint n_construct_properties,
		GObjectConstructParam *construct_params){
	GObject *obj;
	GdkSocket *socket;
	GdkSocketPrivate * priv;
	GdkWindowAttr attr;
	GdkWindowAttributesType mask;
	gchar *name;

	obj = ( *G_OBJECT_CLASS(gdk_socket_parent_class)->constructor)(type,
			n_construct_properties,
			construct_params);
	socket = GDK_SOCKET(obj);

	priv = GDK_SOCKET_GET_PRIVATE(socket);

	socket->display = gdk_display_get_default();
	attr.title = socket->name;
	attr.wclass = GDK_INPUT_ONLY;
	attr.window_type = GDK_WINDOW_TEMP;
	mask = GDK_WA_TITLE;

	socket->window = gdk_window_new(NULL, &attr, mask);

	socket->queue = g_queue_new();
	socket->acks = 0;

	gdk_window_add_filter(socket->window, _window_filter_cb, socket);

	priv->disposed = FALSE;

	return obj;
}

static void
gdk_socket_init(GdkSocket * self){
}

/**
 * gdk_socket_new:
 * @name: the name of the newly created socket, do not need to be unique.
 *
 * Creates a new #GdkSocket.
 *
 * Returns: a newly created the #GdkSocket in #GDK_SOCKET_NEW state.
 */
GdkSocket *
gdk_socket_new(char * name){
	GdkSocket * socket = NULL;

	socket = g_object_new(GDK_TYPE_SOCKET, "name", name, "target", 0,
				"status", GDK_SOCKET_DISCONNECTED, NULL);

	return socket;
}

GdkSocket *
gdk_socket_accept(GdkSocket * self, GdkSocketNativeID target){
	GdkSocket * rt;
	if(self->status == GDK_SOCKET_LISTEN){
		GdkSocketMessage ack;
		gchar * newname = g_strconcat(self->name, "_ACCEPGT", NULL);
		rt = g_object_new(GDK_TYPE_SOCKET, "name", newname, "target", target,
				"status", GDK_SOCKET_CONNECTED, NULL);
		g_free(newname);
		g_signal_connect(rt, "shutdown", _destroy_on_shutdown, NULL);
/*Issue the first ACK message.*/
		ack.header = GDK_SOCKET_ACK;
		ack.source = gdk_socket_get_native(rt);
		_raw_send(rt, rt->target, &ack, sizeof(ack));
		return rt;
	} else{
		g_error("the socket is not listening, how can you ACCEPT?");
		return NULL;
	}

}
/**
 * _dispose:
 * 	@object: The #GdkSocket to be disposed.
 *
 *  Should release all of the ref counts and set a disposed state of the object
 *  TODO: unref all the resources and set state to #GDK_SOCKET_DISPOSED.
 */
static void
_dispose(GObject * object){
	GdkSocket * self = GDK_SOCKET(object);
	GdkSocketPrivate * priv;
	LOG_FUNC_NAME;
	priv = GDK_SOCKET_GET_PRIVATE(self);
	if(! priv->disposed){
		gdk_window_remove_filter(self->window, _window_filter_cb, self);
		g_object_unref(self->window);
		priv->disposed = TRUE;	
	}
	G_OBJECT_CLASS(gdk_socket_parent_class)->dispose(object);
}
/**
 * _finalize:
 * @object: the #GdkSocket to be finalized.
 *
 *  free all the resoursed occupied by @object
 **/
static void
_finalize(GObject * object){
	GdkSocket * self = GDK_SOCKET(object);
	LOG_FUNC_NAME;
	g_free(self->name);
	g_queue_free(self->queue);
	G_OBJECT_CLASS(gdk_socket_parent_class)->finalize(object);
}

/**
 * gdk_socket_get_native:
 * 	@self: the #GdkSocket this method acts on.
 *
 * find out the native id of the socket
 *
 * Returns: the native id of this socket. It happens to be the Window ID of the
 * 	@GdkWindow the GdkSocket wraps, in current implement.
 */
GdkSocketNativeID gdk_socket_get_native(GdkSocket * self){
	return GDK_WINDOW_XWINDOW(self->window);
}

gboolean gdk_socket_connect(GdkSocket * self, GdkSocketNativeID target){
	GdkSocketMessage msg;
	LOG_FUNC_NAME;
	if(self->status != GDK_SOCKET_DISCONNECTED){
		g_warning("Can change to connected state from DISCONNECTED state");
		return FALSE;
	}
	msg.header = GDK_SOCKET_CONNECT_REQ;
	msg.source = gdk_socket_get_native(self);
	return _raw_send(self, target, &msg, sizeof(msg));
}
gboolean gdk_socket_listen(GdkSocket * self){
	self->status = GDK_SOCKET_LISTEN;
/* FIXME: should I broadcast some message here or invoke a signal?*/
	return TRUE;
}
/**
 * gdk_socket_send:
 * @data: you can free the data after calling this function
 */
gboolean gdk_socket_send(GdkSocket * self, gpointer data, guint bytes){
	GdkSocketMessage * msg = g_new0(GdkSocketMessage, 1);
	LOG_FUNC_NAME;
	if(bytes >12){
		g_error("%s: Can not send more than 12 bytes", __func__);
		return FALSE;
	}
	if(self->status != GDK_SOCKET_CONNECTED){
		g_error("Not connected, can not send");
		return FALSE;
	}
	msg->header = GDK_SOCKET_DATA;
	msg->source = gdk_socket_get_native(self);
	g_memmove(msg->l, data, bytes);
	g_print("status = %d\n", self->status);
	g_print("data = %*s", bytes, data);
	g_print("ACKS = %d\n", self->acks);
	if(self->acks <= 0){
		g_queue_push_tail(self->queue, 	msg);
	} else {
		_raw_send(self, self->target, msg, sizeof(GdkSocketMessage));
		self->acks--;
		g_free(msg);
	}
	return TRUE;
}
/**
 * gdk_socket_shutdown:
 *
 * Shutdown a socket connection
 */
void gdk_socket_shutdown(GdkSocket * self) {
	GdkSocketMessage msg;
	LOG_FUNC_NAME;
	if(self->status == GDK_SOCKET_CONNECTED){
		GdkSocketMessage  * q_msg;
		self->status = GDK_SOCKET_DISCONNECTED;
		self->acks = 0;
		while(q_msg = g_queue_pop_head(self->queue)){
			g_free(q_msg);
		}
		msg.header = GDK_SOCKET_SHUTDOWN;
		msg.source = gdk_socket_get_native(self);
		_raw_send(self, self->target, &msg, sizeof(msg));
		
	} else{
		g_warning("Not connected, can not shutdown");
	}
}
/**
 * gdk_socket_window_filter_cb
 *
 * internal filter function related to this X11 implement of #GdkSocket
 */
static GdkFilterReturn 
	_window_filter_cb(GdkXEvent* gdkxevent, GdkEvent * event, gpointer user_data){
	GdkSocket * self = GDK_SOCKET(user_data);
	XEvent * xevent = gdkxevent;
	if(xevent->type == ClientMessage){
		if(xevent->xclient.message_type ==
            gdk_x11_get_xatom_by_name_for_display(self->display, GDK_SOCKET_ATOM_STRING)){

			GdkSocketMessage * msg =(GdkSocketMessage*) xevent->xclient.data.l;
			switch (msg->header){
				case GDK_SOCKET_DATA:
					g_message("DATA");
					if(self->status == GDK_SOCKET_CONNECTED){
						g_print("msg->source =%d , self->target = %d\n", msg->source, self->target);
						if(msg->source == self->target){ 
							guint bytes = 12; /*on x11 we always round off to 12 bytes*/
							/*FIXME: bytes should be squeezed in to GdkSocketMessage.*/
							GdkSocketMessage ack;
							ack.header = GDK_SOCKET_ACK;
							ack.source = gdk_socket_get_native(self);
							_raw_send(self, self->target, &ack, sizeof(ack));
							{
							gpointer data = g_memdup(msg->l, bytes); 
							g_signal_emit(G_OBJECT(self), 
							class_signals[DATA_ARRIVAL],
							0,
							data,
							bytes);
							}
						}
					} else
					g_warning("Wrong socket status, ignore DATA");
					return GDK_FILTER_REMOVE;
				break;
				case GDK_SOCKET_ACK:
					g_message("ACK");
					if(self->status == GDK_SOCKET_DISCONNECTED){
					/*These two simple lines is essential, we establish a connection here.*/
						self->status = GDK_SOCKET_CONNECTED;
						self->target = msg->source;
						{
					/*Then we send an ACK to the other peer to allow it begin data transfer*/
							GdkSocketMessage ack;
							ack.header = GDK_SOCKET_ACK;
							ack.source = gdk_socket_get_native(self);
							_raw_send(self, self->target, &ack, sizeof(ack));
						}
					}
					if(self->status == GDK_SOCKET_CONNECTED){
						if(msg->source == self->target){
							self->acks++;
							if(!g_queue_is_empty(self->queue)){
								GdkSocketMessage * data_msg = g_queue_pop_head(self->queue);
								_raw_send(self, self->target, data_msg, sizeof(GdkSocketMessage));
								self->acks--;
							}
						}
					} else
					g_warning("Wrong socket status(%d), ignore ACK", self->status);
					return GDK_FILTER_REMOVE;
				break;
				case GDK_SOCKET_CONNECT_REQ:
					if(self->status == GDK_SOCKET_LISTEN){
						g_signal_emit(self, 
								class_signals[CONNECT_REQ], 
								0,
								msg->source);
					}else
						g_warning("Wrong socket status, ignore CONNECT_REQ");
					return GDK_FILTER_REMOVE;
				break;
				case GDK_SOCKET_BROADCAST:	
					if(self->status != GDK_SOCKET_CONNECTED){
					g_warning("Not implemented BROADCAST");
					} else
					g_warning("Wrong socket status, ignore DATA");
					return GDK_FILTER_REMOVE;
				break;
				case GDK_SOCKET_SHUTDOWN:
					if(self->status != GDK_SOCKET_CONNECTED){
						g_warning("SHUTDOWN a non connected socket");
					} else{
						GdkSocketMessage * queue_message;
						while(queue_message = g_queue_pop_head(self->queue)){
					/*FIXME: maybe sending all these message will be better than freeing them*/
							g_free(queue_message);
						}
						self->acks = 0;
						self->status == GDK_SOCKET_DISCONNECTED;
						g_signal_emit(self,
								class_signals[SHUTDOWN],
								0);
					}
				break;
				default:
					g_error("Should never reach here!");
			}
		}
	} 
	return GDK_FILTER_CONTINUE;
}

static void _data_arrival(GdkSocket * self,
	gpointer data, guint size){
	LOG_FUNC_NAME;
	g_free(data);
}
static void _connect_req(GdkSocket * self,
	GdkSocketNativeID target){
	LOG_FUNC_NAME;
}
static void _shutdown (GdkSocket * self){
	LOG_FUNC_NAME;
}
static void 
_get_property( GObject * object, guint property_id, GValue * value, GParamSpec * pspec){
	GdkSocket * self = GDK_SOCKET(object);
	switch (property_id){
		case PROP_NAME:
			g_value_set_string(value, self->name);
		break;
		case PROP_TARGET:
			g_value_set_uint(value, self->target);
		break;
		case PROP_STATUS:
			g_value_set_enum(value, self->status);
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}

static void 
_set_property( GObject * object, guint property_id, const GValue * value, GParamSpec * pspec){
	GdkSocket * self = GDK_SOCKET(object);
	switch (property_id){
		case PROP_NAME:
			g_free(self->name);
			self->name = g_value_dup_string(value);
		break;
		case PROP_TARGET:
			self->target = g_value_get_uint(value);
		break;
		case PROP_STATUS:
			self->status = g_value_get_enum(value);
		break;
		default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	}
}



/**
 * gdk_socket_raw_send:
 * @self: the #GdkSocket this method acts on.
 * @target: the native window id of target socket. In current implement, this
 * 		identifies the native window id of #GdkSocket::window.
 * @data: the data buffer. After calling this function, the buffer can be freed.
 * @bytes: the length of data wanted to send.
 *
 * This method of #GdkSocket sends a messange to the target socket whose id 
 * is @target. Whether or not the target is a GdkSocket is not checked
 * (and impossible to check). The length of data has is limited by XClientMessage
 * to be 20 bytes. (or even fewer if in the future the socket adds a header to the
 * message. Note: this is a raw message that is not reliable and do not use it directly!
 *
 * Returns: if successful, return TRUE else return FALSE.
 * SeeAlso: #gdk_socket_send_nosync
 */
static gboolean _raw_send(GdkSocket * self, 
		GdkNativeWindow target, 
		gpointer data, 
		guint bytes){

	gboolean rt;
	rt = _raw_send_nosync(self, target, data, bytes);
    gdk_display_sync (self->display); /*resolve the message, sync the state*/
	return rt;
}
/**
 * gdk_socket_raw_send_nosync:
 * @self: the #GdkSocket this method acts on.
 * @target: the native window id of target socket. In current implement, this
 * 		identifies the native window id of #GdkSocket::window.
 * @data: the data buffer. After calling this function, the buffer can be freed.
 * @bytes: the length of data wanted to send.
 *
 * This function don't call #gdk_display_sync at the end. See #gdk_socket_send. 
 *
 * Returns: if sucessful, TRUE; else FALSE.
 */
static gboolean _raw_send_nosync(GdkSocket * self, GdkNativeWindow target, gpointer data, guint bytes){
	LOG_FUNC_NAME;
    XClientMessageEvent xclient;
	if(bytes > 20){
		g_error("GdkSocket: Can not send more than 20 bytes");
		return FALSE; /*X can not send more information*/
	}

    memset (&xclient, 0, sizeof (xclient));
    xclient.window = target; /*Though X11 places no interpretation of this field, GDK interpretes this field at the target window.*/
    xclient.type = ClientMessage;
    xclient.message_type = gdk_x11_get_xatom_by_name_for_display (self->display, GDK_SOCKET_ATOM_STRING);
    xclient.format = 32;
	memcpy(&xclient.data.l, data, bytes);
    gdk_error_trap_push ();
    XSendEvent (GDK_DISPLAY_XDISPLAY(self->display),
          target,
          False, NoEventMask, (XEvent *)&xclient);
    return gdk_error_trap_pop () == 0;
}

/**
 * _gdk_socket_find_targets:
 * @self: self
 * @name: the name for the targets
 * 
 * Find every possible @GdkSocket on this display with the name @name.
 *
 * Returns: a GList contains the list of those sockets' native ID. 
 * It is the caller's obligation to free the list.
 */
static GList * _gdk_socket_find_targets(GdkSocket * self, gchar * name){
	GList * window_list = NULL;
	GdkScreen * screen;
	GdkWindow * root_window;

    Window root_return;
    Window parent_return;
    Window * children_return;
    unsigned int nchildren_return;
    unsigned int i;

	screen = gdk_drawable_get_screen(self->window);
	g_return_val_if_fail(screen != NULL, NULL);

	root_window = gdk_screen_get_root_window(screen);
	g_return_val_if_fail(root_window != NULL, NULL);

    gdk_error_trap_push();
    XQueryTree(GDK_DISPLAY_XDISPLAY(self->display),
        GDK_WINDOW_XWINDOW(root_window),
        &root_return,
        &parent_return,
        &children_return,
        &nchildren_return);
    if(gdk_error_trap_pop()){
        g_warning("%s: XQueryTree Failed", __func__);
        return NULL;
    }
	g_return_val_if_fail(nchildren_return != 0, NULL);
		
	for(i = 0; i < nchildren_return; i++){
        Atom type_return;
        Atom type_req = gdk_x11_get_xatom_by_name_for_display (self->display, "UTF8_STRING");
        gint format_return;
        gulong nitems_return;
        gulong bytes_after_return;
        guchar * wm_name;
        gint rt;
        gdk_error_trap_push();
        rt = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (self->display), children_return[i],
                          gdk_x11_get_xatom_by_name_for_display (self->display, "_NET_WM_NAME"),
                          0, G_MAXLONG, False, type_req, &type_return,
                          &format_return, &nitems_return, &bytes_after_return,
                          &wm_name);
		if(!gdk_error_trap_pop()){
			if(rt == Success && type_return == type_req){
			if(g_str_equal(name, wm_name)){
				window_list = g_list_append(window_list, (gpointer) children_return[i]);
			}
		}
		}else{
			g_warning("%s:XGetWindowProperty Failed",__func__);
		}
	}
	XFree(children_return);
	g_message("%s:length=%d", __func__, g_list_length(window_list));
	return window_list;
}

/**
 * gdk_socket_send_by_name:
 * @self: you understand.
 * @name: the target socket's name.
 * @data: the data buffer. After calling this function, the buffer can be freed.
 * @bytes: the length of data wanted to send.
 *
 * this method find out the all the #GdkSocket with name @name and calls 
 * #gdk_socket_send_nosync to the * first (successfully sended) target
 *
 * Returns:  TRUE if the message is successfully sent to a target.
 * 	FALSE if the message is not sent to anywhere.
 */
static gboolean 
	_raw_send_by_name(GdkSocket * self, gchar * name, gpointer data, guint bytes){
	GList * window_list = _gdk_socket_find_targets(self, name);
	GList * node;
	gboolean rt = FALSE;
	for(node = g_list_first(window_list); node; node = g_list_next(node)){
		if(gdk_socket_send_nosync(self, (GdkNativeWindow)node->data, data, bytes)){
			rt = TRUE;
			break;
		}
	}
    gdk_display_sync (self->display); /*resolve the message, sync the state*/
	g_list_free(window_list);
	return rt;
}

/**
 * gdk_socket_raw_broadcast_by_name:
 * @self: you understand.
 * @name: the target socket's name.
 * @data: the data buffer. After calling this function, the buffer can be freed.
 * @bytes: the length of data wanted to send.
 *
 * this method find out the all the #GdkSocket with name @name and calls #gdk_socket_send_nosync
 * on first target.
 *
 * Returns:  TRUE if the message is successfully sent to at least one target.
 * 	FALSE if the message is not sent to anywhere.
 *
 * FIXME: 
 *    seems won't work properly if I don't trace the g_message. 
 *    Messages are sent but except for the first socket, #gdk_socket_window_filter 
 *    don't receive them. Perhaps this means we need to get rid of the GdkWindow.
 *
 * TODO:
 *    Figure out why. 
 */
static gboolean
_raw_broadcast_by_name(GdkSocket * self, gchar * name, gpointer data, guint bytes){
	GList * window_list = _gdk_socket_find_targets(self, name);
	GList * node;
	gboolean rt = FALSE;
	gboolean rt1;
	for(node = g_list_first(window_list); node; node = g_list_next(node)){
		rt1 =  gdk_socket_send_nosync(self, (GdkNativeWindow)node->data, data, bytes);
		g_message("%s, sent to one target, %d", __func__, rt1);
		rt = rt || rt1;
	}
    gdk_display_sync (self->display); /*resolve the message, sync the state*/
	g_list_free(window_list);
	return rt;
}
static void _destroy_on_shutdown( GdkSocket * self, gpointer userdata){
	g_object_unref(self);
}
