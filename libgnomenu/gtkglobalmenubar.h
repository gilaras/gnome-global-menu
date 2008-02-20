/*
 * GtkGlobalMenuBar
 * */
#ifndef __GTK_GLOBAL_MENU_BAR_H__
#define __GTK_GLOBAL_MENU_BAR_H__
/**
 * SECTION: gtkglobalmenubar
 * 	@short_description: Global Menu Bar widget for GTK, 
 * 		intended to replace GtkMenuBar
 * 	@see_also: #GtkMenuBar, #gtk_legacy_menu_bar_new
 * 	@stability: Unstable
 * 	@include: libgnomenu/gtkglobalmenubar.h
 *
 * GtkGlobalMenuBar is the widget providing a global menu bar to 
 * the application. It is intended to (be able to) replace 
 * #GtkMenuBar transparently under most
 * situation. It has to be added as a child widget of a top level window
 * to make its full use. Usually you don't want to simplely use
 * #gtk_global_menu_bar_new. If you want to patch GTK, for the sake of
 * making every application that are built against 
 * #GtkMenuBar benifit from #GtkGlobalMenuBar, 
 * use #gtk_legacy_menu_bar_new instead. That function deals 
 * with quirks of various
 * hackish GTK applications.
 *
 * The solution provided by libgnomenu is via remote widget or widget embeding
 * . So maybe in the future we can seperate these code out and redesign the
 * GTK widget embedding solution (i.e #GtkPlug and #GtkSocket), which is 
 * difficult to build widget upon.
 *
 * #GtkGlobalMenuBar plays two roles. 
 *
 * 1. It allocates an 'widget->window' whose parent
 * 	(or parent of parrent, whatever level) is the toplevel widget.
 * 	The events received from this window will be translated and dispatched
 * 	from the ordinary GTK mechanism, which provides compatibility with
 * 	the original #GtkMenuBar, and which is the essential to receive keyboard
 * 	events (Alt+xxx) from the applications toplevel window.
 *
 * 2. It creates an #GnomenuClientHelper, which is a subclass of #GdkSocket
 * 	that understands widget embedding messages and Global Menu messages
 * 	(seperating them will be one of the future topic). These messages are
 * 	then dispatched via #GnomenuClientHelper's signals. This gives the
 * 	privilege for the menu server to control the clients behavior.
 *
 * 	But be careful here. This might be a security hole for a privilege raising
 * 	. Suppose a global menu
 * 	client is running as root other process
 * 	without root access can control a root process in some way. This behavior
 * 	is by definition and it is
 * 	dangerous.  However as GTk is supposed to not deal with these circumstances
 * 	(refer to the discussions around GTK_MODULES environment variable),
 * 	and the messages we are dealing is really nothing more than 12 bytes
 * 	(under X11), it might not be a big hole, or not even a hole.
 *
 */

#include <gdk/gdk.h>
#include <gtk/gtkmenubar.h>
#include "clienthelper.h"

G_BEGIN_DECLS


#define	GTK_TYPE_GLOBAL_MENU_BAR               (gtk_global_menu_bar_get_type ())
#define GTK_GLOBAL_MENU_BAR(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_GLOBAL_MENU_BAR, GtkGlobalMenuBar))
#define GTK_GLOBAL_MENU_BAR_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_GLOBAL_MENU_BAR, GtkGlobalMenuBarClass))
#define GTK_IS_GLOBAL_MENU_BAR(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_GLOBAL_MENU_BAR))
#define GTK_IS_GLOBAL_MENU_BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GLOBAL_MENU_BAR))
#define GTK_GLOBAL_MENU_BAR_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_GLOBAL_MENU_BAR, GtkGlobalMenuBarClass))

typedef struct _GtkGlobalMenuBar       GtkGlobalMenuBar;
typedef struct _GtkGlobalMenuBarClass  GtkGlobalMenuBarClass;

/**
 * GtkGlobalMenuBar:
 * 	@parent: the parent object. 
 *	@helper: the helper for it to play the role as a GNOME menu client.
 *		This is actually an interface. However since at the begining I
 *		was not sure if interface can have default behavior in GLib. 
 *		I decided to follow the favoring 'composition' than 'inheriting'
 *		rule here.
 *	@container: as the name indicates. #container is the parent window
 *		for the sub widgets(which turns out to be #GtkMenuItem s) 
 *		of this #GtkMenuShell. 
 *	@floater: this is a floating toplevel window which act as the 
 *		container of the #container window in detached mode.
 *  @allocation: this is the allocation the menu bar received from
 *  	the menu server. It should never be confused with the allocation
 *  	field in the #GtkWidget interface.
 *  @requisition: this is the requisition the menu bar request from 
 *  	the menu server. It should never be confused with the requisition
 *  	field in the #GtkWidget interface.
 *
 *  I think some of these variables can be moved in to the Private
 *  data structure, especially the #allocation and #requisition, since
 *  The #GtkWidget interface also has those and we have different 
 *  semantic meanings with the #GtkWidget ones. The original purpose
 *  to keep them here is ease to refer to. But with my cute GET_OBJECT
 *  macro in the .c file, referring to a Private variable don't require
 *  much more typings than a public member.
 */
struct _GtkGlobalMenuBar
{
	GtkMenuBar parent;
	GnomenuClientHelper * helper;
	GdkWindow * container;
	GdkWindow * floater;
	GtkAllocation allocation;
	GtkRequisition requisition;
};

/**
 * GtkGlobalMenuBarClass:
 *
 * This is shameless copied from gtkmenubar.c .
 * Even those _gtk_reserved. and the style of using
 * the asternoid is also the GTK way instead of my way.
 */
struct _GtkGlobalMenuBarClass
{
  GtkMenuBarClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


GtkWidget * gtk_global_menu_bar_new             (void);

G_END_DECLS


#endif /* __GTK_GLOBAL_MENU_BAR_H__ */
