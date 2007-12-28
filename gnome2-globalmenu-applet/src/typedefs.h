typedef gulong HandlerID;
typedef Window XWindowID;

typedef enum {
	GTK_MB_MESSAGE_RESIZE,
} GtkMenuBarMessage;

typedef struct _Application Application;
typedef struct {
	gchar * Title;
	enum {
		MENUBAR_LOCAL,
		MENUBAR_REMOTE
	} Type;
	gboolean IsDead; /*Set if Socket is destroyed*/
	union {
		struct {
			gboolean IsDummy;
			GtkWidget * Widget;
		};/* local menubars*/
		struct {
			GtkContainer * Container;
			GdkWindow * Window;
			XWindowID MasterWID;
			GdkPixbuf * Icon;
			gboolean IsStolen;
		}; /*for remote menubars*/
	}/* Menubar*/;
	struct {
	gint x;
	gint y;
	gint w;
	gint h;
	}/* Geometry*/;
	struct {
		HandlerID destroy;
	} Handlers;
	struct _Application * App;
} ClientEntry;

typedef struct {
		GCallback label_area_action_cb;
		GCallback forward_action_cb; 
		GCallback backward_action_cb;
} UICallbacks;

struct _Application {
	GtkContainer * MainWindow;
	enum AppMode {
		APP_STANDALONE,
		APP_APPLET
	} Mode;
	struct {
		GtkImage * ClientIcon;
		GtkLabel * TitleLabel;
		GtkLayout * Layout;
		GtkNotebook * Notebook; /*menu collection, contains all the sockets*/
		GtkEventBox * Backward;
		GtkEventBox * Forward;
	}; /*Widgets*/
	GHashTable * Clients; /*hashed by Client's window id*/
	WnckScreen * Screen;
	ClientEntry * ActiveClient;
};
