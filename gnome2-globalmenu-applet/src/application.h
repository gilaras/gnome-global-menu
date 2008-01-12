struct _WnckScreen;
struct _ClientInfo;
struct _Application {
	GtkContainer * MainWindow;
	struct _MenuServer * Server;
	struct {
		GtkImage * ClientIcon;
		GtkLabel * TitleLabel;
		GtkFixed * Holder; /*since it is the only widget i know who can has a window*/
		GtkEventBox * Backward;
		GtkEventBox * Forward;
	}; /*Widgets*/
	struct {
		GdkPixmap * Background;
		GdkColor Color;
	} AppletProperty;
	struct _WnckScreen * Screen;
	struct _ClientInfo * ActiveClient;
	GList * Clients;
};

typedef struct _Application Application;
