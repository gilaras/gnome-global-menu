#include <config.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "application.h"
//workaround a weird bug in xfce4 includes
#undef _
#undef Q_

/*
 * Standard gettext macros.
 */
#include "intl.h"

static gboolean size_changed(GtkWidget* plugin, gint size) {
	return TRUE;
}

static void
xfce_applet_construct(XfcePanelPlugin *plugin)
{
	g_print("constructing plugin\n");
	g_signal_connect(G_OBJECT(plugin), "size-changed", 
		G_CALLBACK(size_changed), NULL);
	application_new(GTK_CONTAINER(plugin));
}

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL(xfce_applet_construct)

/*
vim:ts=4:sw=4
*/
