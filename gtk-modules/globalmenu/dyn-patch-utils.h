
typedef enum {
	DISCOVER_MODE_INIT,
	DISCOVER_MODE_UNINIT
} DiscoverMode;

void dyn_patch_attach_menubar(GtkWindow * window, GtkMenuBar * menubar);
void dyn_patch_detach_menubar(GtkWindow * window, GtkMenuBar * menubar);

void dyn_patch_queue_changed(GtkMenuBar * menubar, GtkWidget * widget);
void dyn_patch_set_menubar_r(GtkWidget * head, GtkMenuBar * menubar);
GtkMenuBar * dyn_patch_get_menubar(GtkWidget * widget);
void dyn_patch_discover_menubars(DiscoverMode mode);

