#include <gtk/gtk.h>
#include "application.h"
#include "menuserver.h"
#include "log.h"

static void _s_window_destroy(Application * app, GtkWidget * widget);
static void _s_active_client_changed(Application * app, MenuServer * server);

Application * application_new(GtkContainer * window){
	LOG();	
	Application * app = g_new0(Application, 1);
	GtkBox * box = gtk_hbox_new(FALSE, 0); 
	/*This thing is ugly, (consider a vertical menu layout), we need a new alignment widget
 * 	which is similiar to GtkMenuBar(respecting directions) */

	app->window = window;
	app->menu_bar_area = gtk_fixed_new();
	gtk_fixed_set_has_window(app->menu_bar_area, TRUE);
	app->label = gtk_label_new("");
	app->icon = gtk_image_new();

	//gtk_box_pack_start_defaults(box, app->icon);
	gtk_box_pack_start(box, app->label, FALSE, FALSE, 0);
	gtk_box_pack_start(box, app->menu_bar_area, TRUE, TRUE, 0);

	gtk_container_add(app->window, box);
	app->server = menu_server_new(app->menu_bar_area);

	g_signal_connect_swapped(G_OBJECT(app->window), 
		"destroy",
        G_CALLBACK(_s_window_destroy), app);
	g_signal_connect_swapped(G_OBJECT(app->server),
		"active-client-changed",
		G_CALLBACK(_s_active_client_changed), app);
	return app;
}
void application_destroy(Application * app){
	LOG();
	menu_server_destroy(app->server);
	g_free(app);
}
static void _s_active_client_changed(Application * app, MenuServer * server){
	WnckWindow * window = menu_server_get_client_parent(server, server->active);
	WnckApplication * application = wnck_window_get_application(window);
	gchar * name = wnck_application_get_name(application);
	gtk_label_set_text(app->label, name);
}

static void _s_window_destroy(Application * app, GtkWidget * widget){
	LOG();
	application_destroy(app);
}

/*
vim:ts=4:sw=4
*/
