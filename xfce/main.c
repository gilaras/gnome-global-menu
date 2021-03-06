#include <gtk/gtk.h>
#include "libgnomenu/libgnomenu.h"

#include <libxfce4panel/xfce-panel-plugin.h>

static void xfce_panel_plugin_init (XfcePanelPlugin * plugin);

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL ( xfce_panel_plugin_init);

static void on_window_changed(GnomenuMonitor * monitor, ulong prev, XfcePanelPlugin * plugin);
static void null_log_handler(const gchar * domain, GLogLevelFlags level, const gchar* message, gpointer userdata) {
	return;
}
static void xfce_panel_plugin_init (XfcePanelPlugin * plugin) {
	g_log_set_handler("libgnomenu", G_LOG_LEVEL_DEBUG, null_log_handler, NULL);

	gtk_rc_parse_string(
		"style \"globalmenu_event_box_style\"\n"
		"{\n"
		"	GtkWidget::focus-line-width=0\n"
		"	GtkWidget::focus-padding=0\n"
		"}\n"
		"style \"globalmenu_menu_bar_style\"\n"
		"{\n"
		"	ythickness = 0\n"
		"	GtkMenuBar::shadow-type = none\n"
		"	GtkMenuBar::internal-padding = 0\n"
		"}\n"
		"class \"GtkPlug\" style \"globalmenu_event_box_style\"\n"
		"class \"GnomenuMenuBar\" style:highest \"globalmenu_menu_bar_style\"\n"
	);
	gtk_widget_reset_rc_styles(plugin);
	GnomenuMonitor * monitor = gnomenu_monitor_new();
	GnomenuGlobalMenu * menubar = gnomenu_global_menu_new();
	g_object_ref_sink(menubar);
	g_object_set_data_full(G_OBJECT(plugin), "monitor", monitor, g_object_unref);
	g_object_set_data_full(G_OBJECT(plugin), "menubar", menubar, g_object_unref);

	xfce_panel_plugin_add_action_widget(plugin, GTK_WIDGET(plugin));
	xfce_panel_plugin_set_expand(plugin, TRUE);

	xfce_panel_plugin_add_action_widget(plugin, GTK_WIDGET(menubar));

	gtk_container_add(GTK_CONTAINER(plugin), GTK_WIDGET(menubar));
	g_signal_connect(monitor, "window-changed", on_window_changed, plugin);
	gtk_widget_show(menubar);

}

static void on_window_changed(GnomenuMonitor * monitor, gulong prev, XfcePanelPlugin * plugin) {
	gulong xid = gnomenu_monitor_get_current_xid(monitor);
	GnomenuGlobalMenu * menubar = g_object_get_data(G_OBJECT(plugin),
			"menubar");
	gnomenu_global_menu_switch_to(menubar, xid);
}
