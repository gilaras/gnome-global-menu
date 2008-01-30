#ifndef GNOMENU_SERVER_H
#define GNOMENU_SERVER_H

#include "gdksocket.h"
#include "gnomenumessage.h"

G_BEGIN_DECLS
/**
 * SECTION: serverhelper
 * @short_description: Menu server herlper class.
 * @see_also: #GtkMenuBar, #GtkGlobalMenuBar, #GdkSocket.
 * @stablility: Unstable
 * @include: libgnomenu/serverhelper.h
 *
 * GnomenuServerHelper provides fundanmental messaging mechanism for a menu server
 * 
 */

#define GNOMENU_TYPE_SERVER_HELPER	(gnomenu_server_helper_get_type())
#define GNOMENU_SERVER_HELPER(obj) 	(G_TYPE_CHECK_INSTANCE_CAST((obj), GNOMENU_TYPE_SERVER_HELPER, GnomenuServerHelper))
#define GNOMENU_SERVER_HELPER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GNOMENU_TYPE_SERVER_HELPER, GnomenuServerHelperClass))
#define GNOMENU_IS_SERVER_HELPER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOMENU_TYPE_SERVER_HELPER))
#define GNOMENU_IS_SERVER_HELPER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOMENU_TYPE_SERVER_HELPER))
#define GNOMENU_SERVER_HELPER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), GNOMENU_TYPE_SERVER_HELPER, GnomenuServerHelperClass))

typedef struct _GnomenuServerHelperClass GnomenuServerHelperClass;

/**
 * GnomenuServerHelper:
 *  @clients: A List of all the clients, each of which is GnomenuClientInfo
 *
 * GnomenuServerHelper provides fundanmental messaging mechanism for a menu server
 */
typedef struct _GnomenuServerHelper GnomenuServerHelper;
struct _GnomenuServerHelper{
	GdkSocket parent;
/*< public >*/
	GList * clients;
};

/**
 * GnomenuServerHelperClientInfo:
 * @socket_id: the native id of the #GdkSocket the client is using. This value is
 * 			returned by #gdk_socket_get_native;
 * @ui_window: the container window which contains all the ui elements of the menu;
 * 			A menu server application can steal it and arrange its position.
 * @parent_window: the parent window of the menubar. The menu server application take
 * 		care of the changing of the focused window, and takes care of switching 
 * 		the active menu. 
 *
 * This structure is where #GnomenuServerHelper stores client infomation;
 */
typedef struct _GnomenuClientInfo GnomenuClientInfo;

struct _GnomenuClientInfo {
	GdkSocketNativeID socket_id;
	GdkNativeWindow ui_window;
	GdkNativeWindow parent_window;
};

enum {
	GMS_SIGNAL_CLIENT_NEW,
	GMS_SIGNAL_CLIENT_DESTROY,
	GMS_SIGNAL_SIZE_REQUEST,
	GMS_SIGNAL_MAX
};
/**
 * GnomenuServerHelperClass:
 * @menu_create: the virtual function invoked.
 */
struct _GnomenuServerHelperClass {
	GdkSocketClass parent;
/*< private >*/	
	guint signals[GMS_SIGNAL_MAX];
	GType * type_gnomenu_message_type;

	void (*client_new)(GnomenuServerHelper * self, GnomenuClientInfo * client_info);
	void (*client_destroy)(GnomenuServerHelper * self, GnomenuClientInfo * client_info);
	void (*client_size_request)(GnomenuServerHelper * self, GnomenuClientInfo * client_info, GtkRequisition * req);
};
/**
 * GNOMENU_SERVER_NAME:
 *
 * Name of the socket
 */
#define GNOMENU_SERVER_NAME "GNOME MENU SERVER"

G_END_DECLS
#endif
