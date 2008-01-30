#include <gtk/gtk.h>

#include "serverhelper.h"
#include "gnomenu-marshall.h"
#include "gnomenu-enums.h"

#define LOG_FUNC_NAME g_message(__func__)

typedef struct _GnomenuServerHelperPrivate GnomenuServerHelperPrivate;

struct _GnomenuServerHelperPrivate {
	gboolean disposed;
};

#define GNOMENU_SERVER_HELPER_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE(obj, GNOMENU_TYPE_SERVER_HELPER, GnomenuServerHelperPrivate))

static void gnomenu_server_helper_dispose
			(GObject * self);
static void gnomenu_server_helper_finalize
			(GObject * self);
static GObject* gnomenu_server_helper_constructor
			(GType type, guint n_construct_properties, GObjectConstructParam *construct_params);
static void gnomenu_server_helper_init
			(GnomenuServerHelper * self);
static void gnomenu_server_helper_client_new
			(GnomenuServerHelper * self, GnomenuClientInfo * ci);
static void gnomenu_server_helper_client_realize
			(GnomenuServerHelper * self, GnomenuClientInfo * ci);
static void gnomenu_server_helper_client_unrealize
			(GnomenuServerHelper * self, GnomenuClientInfo * ci);
static void gnomenu_server_helper_client_destroy
			(GnomenuServerHelper * self, GnomenuClientInfo * ci);
static void gnomenu_server_helper_client_size_request
			(GnomenuServerHelper * self, GnomenuClientInfo * ci);
static void gnomenu_server_helper_data_arrival_cb
			(GdkSocket * socket, gpointer data, gint bytes, gpointer userdata);

G_DEFINE_TYPE (GnomenuServerHelper, gnomenu_server_helper, GDK_TYPE_SOCKET)

static void
gnomenu_server_helper_class_init(GnomenuServerHelperClass *klass){
	GObjectClass * gobject_class = G_OBJECT_CLASS(klass);
	LOG_FUNC_NAME;

	/*FIXME: unref the type at class_finalize*/
	klass->type_gnomenu_message_type = g_type_class_ref(GNOMENU_TYPE_MESSAGE_TYPE); 

	g_type_class_add_private(gobject_class, sizeof (GnomenuServerHelperPrivate));
	gobject_class->constructor = gnomenu_server_helper_constructor;
	gobject_class->dispose = gnomenu_server_helper_dispose;
	gobject_class->finalize = gnomenu_server_helper_finalize;

	klass->client_new = gnomenu_server_helper_client_new;
	klass->client_realize = gnomenu_server_helper_client_realize;
	klass->client_unrealize = gnomenu_server_helper_client_unrealize;
	klass->client_destroy = gnomenu_server_helper_client_destroy;
	klass->client_size_request = gnomenu_server_helper_client_size_request;

	klass->signals[GMS_SIGNAL_CLIENT_NEW] = 
/**
 * GnomenuServerHelper::client-new:
 * @self: the GnomenuServe who emits the signal.
 * @client_info: the client info. #GnomenuClientInfo. However only the socket_id field is defined.
 *
 * ::client-new signal is emitted when:
 * 	 server receives a  message indicates a new client is born;
 * 	 and server finished initializing internal data structures for the new client.
 */
		g_signal_new ("client-new",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GnomenuServerHelperClass, client_new),
		NULL, NULL,
			gnomenu_marshall_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);

	klass->signals[GMS_SIGNAL_CLIENT_REALIZE] =
/**
 * GnomenuServerHelper::client-realize:
 * 	@self: self.
 * 	@client_info: the client info.
 *
 * ::client-reailze signal is emitted when the client's window is realized.
 */
		g_signal_new ("client-realize",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GnomenuServerHelperClass, client_realize),
			NULL, NULL,
			gnomenu_marshall_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);

	klass->signals[GMS_SIGNAL_CLIENT_UNREALIZE] = 
/**
 * GnomenuServerHelper::client-unrealize:
 * 	@self: self.
 * 	@client_info: the client info.
 *
 * ::client-unreailze signal is emitted when the client's window is realized.
 */
		g_signal_new ("client-unrealize",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GnomenuServerHelperClass, client_unrealize),
			NULL, NULL,
			gnomenu_marshall_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);

	klass->signals[GMS_SIGNAL_CLIENT_DESTROY] = 
/**
 * GnomenuServerHelper::client-destroy:
 * @self: the GnomenuServe who emits the signal.
 * @client_info: the client info. #GnomenuClientInfo
 *
 * ::client-destroy signal is emitted when:
 * 	 server receives a  message indicates a client is die;
 * 	 and before server disposing internal data structures for the dead client.
 */
		g_signal_new ("client-destroy",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GnomenuServerHelperClass, client_destroy),
		NULL, NULL,
			gnomenu_marshall_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER);
	klass->signals[GMS_SIGNAL_SIZE_REQUEST] = 
/**
 * GnomenuServerHelper::size-request:
 * @self: the GnomenuServe who emits the signal.
 * @client_info: the client info. #GnomenuClientInfo
 * @requisition: the size requisition. Don't free it. #GtkRequisition
 * @allocation:  the allocation the server want to assign the the client,
 * 		The initial value assigned as the @requisition.
 *
 * #GNOMENU_MSG_SIZE_QUERY, \\
 * #GnomenuClientHelper::size-query, \\
 * #GNOMENU_MSG_SIZE_REQUEST, \\
 * #GnomenuServerHelper::size-request,
 * #GNOMENU_MSG_SIZE_ALLOCATE, \\
 * #GnomenuClientHelper::size-allocate, \\
 *
 * are chained up to finish a size allocation. See #GnomenuMessageSizeQuery for some other 
 * desciptions.
 */
		g_signal_new ("size-request",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
			G_STRUCT_OFFSET (GnomenuServerHelperClass, client_size_request),
			NULL, NULL,
			gnomenu_marshall_VOID__POINTER,
			G_TYPE_NONE,
			1,
			G_TYPE_POINTER
			);
}

static void
gnomenu_server_helper_init(GnomenuServerHelper * self){
	LOG_FUNC_NAME;
}

/**
 * gnomenu_server_helper_new:
 * 
 * create a new menu server object
 */
GnomenuServerHelper * 
gnomenu_server_helper_new(){
	GnomenuServerHelper * self;
	LOG_FUNC_NAME;
	self = g_object_new(GNOMENU_TYPE_SERVER_HELPER, "name", GNOMENU_SERVER_NAME, NULL);
	return self;
}

static void gnomenu_server_helper_dispose(GObject * object){
	GnomenuServerHelper *self;
	GnomenuServerHelperPrivate * priv;
	LOG_FUNC_NAME;
	self = GNOMENU_SERVER_HELPER(object);
	priv = GNOMENU_SERVER_HELPER_GET_PRIVATE(self);
	
	if(! priv->disposed){
		priv->disposed = TRUE;
	}
	G_OBJECT_CLASS(gnomenu_server_helper_parent_class)->dispose(object);
}

static void gnomenu_server_helper_client_info_free(gpointer  p, gpointer data){
	g_free(p);
}
static void gnomenu_server_helper_finalize(GObject * object){
	GnomenuServerHelper *self;
	LOG_FUNC_NAME;
/**
 * FIXME: perhaps this is buggy
 */
	self = GNOMENU_SERVER_HELPER(object);
	g_list_foreach(self->clients, gnomenu_server_helper_client_info_free, NULL);
	g_list_free(self->clients);
	G_OBJECT_CLASS(gnomenu_server_helper_parent_class)->finalize(object);
}

/** 
 * gnomenu_server_helper_data_arrival_cb:
 *
 * 	callback, invoked when the embeded socket receives data
 */
static void gnomenu_server_helper_data_arrival_cb(GdkSocket * socket, 
		gpointer data, gint bytes, gpointer userdata){
	GnomenuMessage * message = data;
	GEnumValue * enumvalue = NULL;
	guint * signals = NULL;
	GnomenuClientInfo * ci = NULL;
	GnomenuServerHelper * self = GNOMENU_SERVER_HELPER(socket);

	LOG_FUNC_NAME;
	g_assert(bytes >= sizeof(GnomenuMessage));
	/*initialize varialbes*/
	signals = GNOMENU_SERVER_HELPER_GET_CLASS(self)->signals;
	ci = gnomenu_server_helper_find_client_by_socket_id(self, message->client_destroy.socket_id);

	enumvalue = g_enum_get_value(
					GNOMENU_SERVER_HELPER_GET_CLASS(self)->type_gnomenu_message_type, 
					message->any.type);
	g_message("message arrival: %s", enumvalue->value_name);

	switch(enumvalue->value){
		case GNOMENU_MSG_CLIENT_NEW:
			if(ci){
				g_warning("client already recorded, "
						"(silently) remove it and continue");
				self->clients = g_list_remove_all(self->clients, ci);
			}
			ci = g_new0(GnomenuClientInfo, 1);
			ci->stage = GNOMENU_CI_STAGE_NEW;
			ci->size_stage = GNOMENU_CI_STAGE_RESOLVED;
			ci->socket_id = message->client_new.socket_id;
			g_signal_emit(G_OBJECT(self), 
					signals[GMS_SIGNAL_CLIENT_NEW],
					0,
					ci);
			break;
		case GNOMENU_MSG_CLIENT_REALIZE:
			if(!ci) {
				g_warning("an non managed client is realized, ignore and continue");
			}
			if(ci->stage == GNOMENU_CI_STAGE_REALIZED){
				g_warning("already realized. forget about the old one."
				"");
				/*FIXME: Shall we Simulate an unrealize signal?*/
			}
			ci->ui_window = message->client_realize.ui_window;
			ci->parent_window = message->client_realize.parent_window;
			ci->stage = GNOMENU_CI_STAGE_REALIZED;
			g_signal_emit(G_OBJECT(self),
					signals[GMS_SIGNAL_CLIENT_REALIZE],
					0, ci);
			break;
		case GNOMENU_MSG_CLIENT_UNREALIZE:
			if(!ci) {
				g_warning("an non managed client is unrealized, ignore and continue");
			}
			if(ci->stage != GNOMENU_CI_STAGE_REALIZED){
				g_warning("unrealize a not realized client? ignore it");
				break;
			}
			ci->ui_window = NULL;
			ci->parent_window = NULL;
			ci->stage = GNOMENU_CI_STAGE_UNREALIZED;
			g_signal_emit(G_OBJECT(self),
					signals[GMS_SIGNAL_CLIENT_UNREALIZE],
					0, ci);
			break;
		case GNOMENU_MSG_CLIENT_DESTROY:
			if(!ci) {
				g_warning("Destroying an non-exist client,"
					"(silently) ignore this message and continue");
				break;
			} 
			if(ci->stage != GNOMENU_CI_STAGE_UNREALIZED){
				ci->stage = GNOMENU_CI_STAGE_DESTROYED;
				g_warning("an unrealize message is missing, stimulate one here");
			/*NOTE: however in this unrealize signal ci->stage will be DESTROYED*/
				g_signal_emit(G_OBJECT(self),
					signals[GMS_SIGNAL_CLIENT_UNREALIZE],
					0, ci);
			}
			ci->stage = GNOMENU_CI_STAGE_DESTROYED;
			g_signal_emit(G_OBJECT(self), 
					signals[GMS_SIGNAL_CLIENT_DESTROY],
					0, ci);
			break;
		case GNOMENU_MSG_SIZE_REQUEST:
			if(!ci){
				g_warning("client not found,"
					"(silently) ignore this message and continue");
				break;
			} /*else*/
			if(ci->size_stage != GNOMENU_CI_STAGE_QUERYING){
				g_message("this resize queue is issued by the client");
			}
			{
			/*NOTE: here we set some sanity value for the allocation*/
				ci->allocation.width =
				ci->requisition.width = message->size_request.width;
				ci->allocation.height =
				ci->requisition.height = message->size_request.height;
				ci->size_stage = GNOMENU_CI_STAGE_RESPONSED;
				g_signal_emit(G_OBJECT(self),
						signals[GMS_SIGNAL_SIZE_REQUEST],
						0, ci);
			}
		break;
		default:
		g_warning("unknown message, ignore and continue");
	}
}

/*public methods*/
/**
 * gnomenu_server_helper_client_compare_by_socket_id:
 *
 * Return: TRUE if socket_id matches. FALSE if else.
 */
static gboolean gnomenu_server_helper_client_compare_by_socket_id(
	const GnomenuClientInfo * p1,
	const GnomenuClientInfo * p2){
	return !( p1 && p2 && p1->socket_id == p2->socket_id);
}
/**
 * gnomenu_server_helper_client_compare_by_parent_window:
 *
 * Return: TRUE if socket_id matches. FALSE if else.
 */
static gboolean gnomenu_server_helper_client_compare_by_parent_window(
	const GnomenuClientInfo * p1,
	const GnomenuClientInfo * p2){
	return !( p1 && p2 && p1->parent_window == p2->parent_window);
}

/**
 * gnomenu_server_helper_find_client_by_socket_id:
 *
 * Find a client by socket_id
 */
GnomenuClientInfo * 
gnomenu_server_helper_find_client_by_socket_id(
		GnomenuServerHelper * self,
		GdkSocketNativeID socket_id){

	GnomenuClientInfo ci_key;
	GList * found;
	ci_key.socket_id = socket_id;
	
	found = g_list_find_custom(self->clients, 
		&ci_key, (GCompareFunc)gnomenu_server_helper_client_compare_by_socket_id);
	if(found) return found->data;
	return NULL;
}
/**
 * gnomenu_server_helper_find_client_by_parent_window:
 *
 * Find a client by parent_window
 */
GnomenuClientInfo * 
gnomenu_server_helper_find_client_by_parent_window(
		GnomenuServerHelper * self,
		GdkNativeWindow parent_window){

	GnomenuClientInfo ci_key;
	GList * found;
	ci_key.parent_window = parent_window;
	
	found = g_list_find_custom(self->clients, 
		&ci_key, (GCompareFunc)gnomenu_server_helper_client_compare_by_parent_window);
	if(found) return found->data;
	return NULL;
}
void
gnomenu_server_helper_client_queue_resize(GnomenuServerHelper * self, GnomenuClientInfo * ci){
	GnomenuMessage msg;
	LOG_FUNC_NAME;
	msg.any.type = GNOMENU_MSG_SIZE_QUERY;
	msg.size_query.socket_id = gdk_socket_get_native(self);
	ci->size_stage = GNOMENU_CI_STAGE_QUERYING;
	gdk_socket_send(self, ci->socket_id, &msg, sizeof(msg));
}
/* virtual functions for signal handling*/
static void 
gnomenu_server_helper_client_new(GnomenuServerHelper * self, GnomenuClientInfo * ci){
	GnomenuMessage msg;
	LOG_FUNC_NAME;

	self->clients = g_list_prepend(self->clients, ci);

/*then we tell the client about us*/
	msg.any.type = GNOMENU_MSG_SERVER_NEW;
	msg.server_new.socket_id = gdk_socket_get_native(self);
	gdk_socket_send(self, ci->socket_id, &msg, sizeof(msg));
}
static void 
gnomenu_server_helper_client_realize(GnomenuServerHelper * self, GnomenuClientInfo * ci){
	LOG_FUNC_NAME;
	/*Do nothing */
}
static void 
gnomenu_server_helper_client_unrealize(GnomenuServerHelper * self, GnomenuClientInfo * ci){
	LOG_FUNC_NAME;
	/*Do nothing */
}
static void 
gnomenu_server_helper_client_destroy(GnomenuServerHelper * self, GnomenuClientInfo * ci){
	LOG_FUNC_NAME;
	self->clients = g_list_remove_all(self->clients, ci);
}
static void 
gnomenu_server_helper_client_size_request(GnomenuServerHelper * self, GnomenuClientInfo * ci){
	GnomenuMessage msg;
	LOG_FUNC_NAME;
/*TODO: send the allocation to the client*/
	msg.any.type = GNOMENU_MSG_SIZE_ALLOCATE;

	msg.size_allocate.socket_id = gdk_socket_get_native(self);
	msg.size_allocate.width = ci->allocation.width;
	msg.size_allocate.height = ci->allocation.height;
	
	gdk_socket_send(GDK_SOCKET(self), ci->socket_id, 
		&msg, sizeof(msg));
	ci->size_stage = GNOMENU_CI_STAGE_RESOLVED;
}

static GObject* gnomenu_server_helper_constructor(
		GType type, guint n_construct_properties, GObjectConstructParam *construct_params){
	GObject *obj;
	GnomenuServerHelper *self;
	GnomenuServerHelperPrivate * priv;
		
	obj = ( *G_OBJECT_CLASS(gnomenu_server_helper_parent_class)->constructor)(type,
			n_construct_properties,
			construct_params);
	self = GNOMENU_SERVER_HELPER(obj);

	priv = GNOMENU_SERVER_HELPER_GET_PRIVATE(self);
	self->clients = NULL;
	g_signal_connect(G_OBJECT(self), "data-arrival", G_CALLBACK(gnomenu_server_helper_data_arrival_cb), NULL);
	priv->disposed = FALSE;

	return obj;
}
