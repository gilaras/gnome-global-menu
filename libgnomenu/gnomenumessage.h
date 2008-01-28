#ifndef GNOMENU_MESSAGE_H
#define GNOMENU_MESSAGE_H
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "gdksocket.h"

G_BEGIN_DECLS
/**
 * SECTION: gnomenumessage
 * @title: GnomenuMessage
 * @short_description: The underground messages for libgnomenu.
 * @see_also:	#GnomenuServer, #GnomenuClient #GdkSocket
 * @stablility: Unstable
 * @include:	libgnomenu/gnomenumessage.h
 *
 * gnomenumessage is the data type for the messages passed around in libgnomenu.
 */

/**
 * GnomenuMessageType:
 * @GNOMENU_MSG_ANY: 			#GnomenuMessageAny
 * @GNOMENU_MSG_CLIENT_NEW: 	#GnomenuMessageClientNew
 * @GNOMENU_MSG_CLIENT_DESTROY: #GnomenuMessageClientDestroy
 * @GNOMENU_MSG_SERVER_NEW: 	#GnomenuMessageServerNew
 * @GNOMENU_MSG_SERVER_DESTROY: #GnomenuMessageServerDestroy
 * @GNOMENU_MSG_SIZE_REQUEST: 	#GnomenuMessageSizeRequest
 * @GNOMENU_MSG_SIZE_ALLOCATE:	#GnomenuMessageSizeAllocate
 * @GNOMENU_MSG_SIZE_QUERY:		#GnomenuMessageSizeQuery
 * @GNOMENU_MSG_MAX:		no meaning
 *
 * type of a libgnomenu message.
 */
typedef enum { /*< prefix=GNOMENU >*/
	GNOMENU_MSG_ANY,
	GNOMENU_MSG_CLIENT_NEW,
	GNOMENU_MSG_CLIENT_DESTROY,
	GNOMENU_MSG_SERVER_NEW,
	GNOMENU_MSG_SERVER_DESTROY,
	GNOMENU_MSG_SIZE_ALLOCATE,
	GNOMENU_MSG_SIZE_REQUEST,
	GNOMENU_MSG_QUERY_REQUISITION,
	GNOMENU_MSG_MAX,
} GnomenuMessageType;

/**
 * GnomenuMessageAny:
 * @type:	#GNOMENU_MSG_ANY
 * @data:	detailed data of the message;
 *
 * An generic message. useless if not debugging.
 */
typedef struct {
	GnomenuMessageType type;
	gulong data[3];
} GnomenuMessageAny;

/**
 * GnomenuMessageClientNew:
 * @type: 	#GNOMENU_MSG_CLIENT_NEW
 * @socket_id:
 *
 * FIXME: should seperate into three different messages.
 */
typedef struct {
	GnomenuMessageType type;
	GdkSocketNativeID socket_id;
	GdkNativeWindow ui_window;
	GdkNativeWindow parent_window;
} GnomenuMessageClientNew;

/**
 * GnomenuMessageClientDestroy:
 * @type: #GNOMENU_MSG_CLIENT_DESTROY
 * @socket_id:
 *
 */
typedef struct {
	GnomenuMessageType type;
	GdkSocketNativeID socket_id;
} GnomenuMessageClientDestroy;
/**
 * GnomenuMessageServerNew:
 * 	@type: #GNOMENU_MSG_SERVER_NEW
 * 	@socket_id:
 * 	@container_window:	perhaps this is useless in current implement;
 * 		However, if we are moving to Etolite's WildMenu alike implementation(
 * 		i.e. the client takes care of everything)
 * 		This field will be useful.
 */
typedef struct {
	GnomenuMessageType type;
	GdkSocketNativeID socket_id;
	GdkNativeWindow container_window;
} GnomenuMessageServerNew;
/**
 * GnomenuMessageServerDestroy:
 * @type: #GNOMENU_MSG_SERVER_DESTROY
 * @socket_id:
 */
typedef struct {
	GnomenuMessageType type;
	GdkSocketNativeID socket_id;
} GnomenuMessageServerDestroy;

/**
 * GnomenuMessageSizeQuery:
 * @type: #GNOMENU_MSG_SIZE_QUERY
 * @socket_id: native socket id of the server.
 *
 * A size query message begins a sequency of size allocation operation
 * between the server and client.
 *
 * 1. server sends a #GnomenuMessageSizeQuery to the client.
 *
 * 2. client receives the message, calculates its requisition, then send
 *    a #GnomenuMessageSizeRequest to server.
 *
 * 3. server receives the #GnomenuMessageSizeRequest message, allocate
 * 	  the allocation for the client, and send a #GnomenuMessageSizeAllocate
 * 	  to the client.
 */
typedef struct {
	GnomenuMessageType type;
	GdkSocketNativeID socket_id;
} GnomenuMessageSizeQuery;

/**
 * GnomenuMessageSizeRequest:
 * @type: #GNOMENU_MSG_SIZE_REQUEST
 * @socket_id: the native socket id for the client's socket.
 * @width:
 * @height:
 *
 * See #GnomenuMessageQuerySize for a complete description of a size allocation chain.
 */
typedef struct {
	GnomenuMessageType type;
	GdkSocketNativeID socket_id;
	gint	width;
	gint 	height;
} GnomenuMessageSizeRequest;

/**
 * GnomenuMessageSizeAllocate
 * @type: #GNOMENU_MSG_SIZE_ALLOCATE
 * @width:
 * @height:
 *
 * See #GnomenuMessageQuerySize for a complete description of a size allocation chain.
 */
typedef struct {
	GnomenuMessageType type;
	gint width;
	gint height;
} GnomenuMessageSizeAllocate;

/**
 * GnomenuMessage:
 *
 * This structure wraps every kind of gnomenu message into a union.
 */
typedef struct _GnomenuMessage GnomenuMessage;
struct _GnomenuMessage {
	union {
		GnomenuMessageAny any;
		GnomenuMessageClientNew client_new;
		GnomenuMessageClientDestroy client_destroy;
		GnomenuMessageServerNew server_new;
		GnomenuMessageServerDestroy server_destroy;
		GnomenuMessageSizeRequest	size_request;
		GnomenuMessageSizeAllocate	size_allocate;
		GnomenuMessageSizeQuery	size_query;

	};
};
#define GNOMENU_TYPE_MESSAGE gnomenu_message_get_type()
GType gnomenu_message_get_type (void) ;
G_END_DECLS
#endif
