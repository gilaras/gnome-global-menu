using GLib;
using Panel;
using PanelCompat;
using Gtk;
public extern int system(string? cmd);

public class GlobalMenuApplet : PanelCompat.Applet {
	static GlobalMenuApplet the_applet;
	static const string APPLET_NAME = "globalmenu-panel-applet";
	static const string APPLET_VERSION = "0.6";
	static const string APPLET_ICON = "gnome-fs-home";
	static const string GCONF_SCHEMA_DIR = "/schemas/apps/globalmenu-panel-applet/prefs";
	static const string[] APPLET_AUTHORS = {"Coding:",
						"Yu Feng <rainwoodman@gmail.com>",
						"Mingxi Wu <fengshenx@gmail.com>",
						"bis0n.lives <bis0n.lives@gmail.com>",
						"Luca Viggiani <lviggiani@gmail.com>",
						"",
						"Packaging:",
						"sstasyuk <sstasyuk@gmail.com>",
						"David Watson <dwatson031@gmail.com>",
						"Valiant Wing <Valiant.Wing@gmail.com>"};
	
	static const string[] APPLET_ADOCUMENTERS = {"Pierre Slamich <pierre.slamich@gmail.com>"};
	private Wnck.Screen screen;
	private Gnomenu.MenuBar menubar;
	private Gtk.Box box;
	private PanelExtra.Switcher switcher;
	
    public static bool factory (Panel.Applet applet, string iid) {
        ((GlobalMenuApplet) applet).create ();
        return true;
    }
	public static void message(string msg) {
		Gtk.MessageDialog m = new Gtk.MessageDialog(null,
							    Gtk.DialogFlags.MODAL,
							    Gtk.MessageType.INFO,
							    Gtk.ButtonsType.OK,
							    msg);
		m.run();
		m.destroy();
	}
    private static void on_about_clicked (BonoboUI.Component component,
                                          void* user_data, string cname) {
       	var dialog = new Gtk.AboutDialog();
       	dialog.program_name = APPLET_NAME;
		dialog.version = APPLET_VERSION;
		dialog.website = "http://code.google.com/p/gnome2-globalmenu";
		dialog.website_label = "Project Home";
		dialog.wrap_license = false;
		dialog.license = GPL.Licenses.V2;
		dialog.logo_icon_name = APPLET_ICON;
		dialog.authors = APPLET_AUTHORS;
		dialog.documenters = APPLET_ADOCUMENTERS;
       	dialog.run();
       	dialog.destroy();
    }
    private static void on_help_clicked (BonoboUI.Component component,
                                          void* user_data, string cname) {
       	var dialog = new Gtk.AboutDialog();
       	dialog.program_name = APPLET_NAME;
		dialog.version = APPLET_VERSION;
		dialog.website = "http://code.google.com/p/gnome2-globalmenu/w/list";
		dialog.website_label = "On-line help";
		dialog.logo_icon_name = "gtk-help";
       	dialog.run();
       	dialog.destroy();
    }
    private static void on_preferences_clicked (BonoboUI.Component component,
                                          void* user_data, string cname) {
       	//system("gconf-editor " + the_applet.get_preferences_key() + " &");
		GtkExtra.GConfDialog gcd = new GtkExtra.GConfDialog(the_applet.get_preferences_key(), "Applet preferences");
		gcd.run();
		gcd.destroy();
    }
	private void on_active_window_changed (WnckCompat.Screen screen, Wnck.Window? previous_window){
		weak Wnck.Window window = (screen as Wnck.Screen).get_active_window();
			if((window != previous_window) && (window is Wnck.Window)) {
				weak Wnck.Window transient_for = window.get_transient();
				if(transient_for != null) window = transient_for;
				string xid = window.get_xid().to_string();
				menubar.switch(xid);
			}
	}
    private void on_change_background (GlobalMenuApplet applet, Panel.AppletBackgroundType bgtype,
                                       Gdk.Color? color, Gdk.Pixmap? pixmap) {
        Gtk.Style style = (Gtk.rc_get_style(this.menubar) as GtkCompat.Style).copy();
			switch(bgtype){
				case Panel.AppletBackgroundType.NO_BACKGROUND:
					Gtk.Style def_style = Gtk.rc_get_style(this);
					this.menubar.set_style(def_style);
					this.menubar.queue_draw();
				
					this.switcher.set_style(def_style);
					this.switcher.queue_draw();
					
					return;
				break;
				case Panel.AppletBackgroundType.COLOR_BACKGROUND:
					style.bg_pixmap[(int)StateType.NORMAL] = null;
					style.bg[(int)StateType.NORMAL] = color;
				break;
				case Panel.AppletBackgroundType.PIXMAP_BACKGROUND:
					style.bg_pixmap[(int)StateType.NORMAL] = pixmap;
				break;
			}
			this.menubar.set_style(style);
			this.menubar.queue_draw();
					
			this.switcher.set_style(style);
			this.switcher.queue_draw();
    }
	private void get_prefs() {
		switcher.show_label = this.gconf_get_bool("show_name");
		switcher.show_icon = this.gconf_get_bool("show_icon");
		switcher.max_size = this.gconf_get_int("title_max_width");
		switcher.show_window_actions = this.gconf_get_bool("show_window_actions");
		switcher.show_window_list = this.gconf_get_bool("show_window_list");
		switcher.refresh();
	}
    private void create () {
    	the_applet = this;
		this.add_preferences(GCONF_SCHEMA_DIR);
		GConf.Client.get_default().value_changed += (key, value) => {
			this.get_prefs();
		};
		
    	this.set_flags(Panel.AppletFlags.EXPAND_MINOR | Panel.AppletFlags.HAS_HANDLE | Panel.AppletFlags.EXPAND_MAJOR );
        change_background += on_change_background;

		menubar = new Gnomenu.MenuBar();
		menubar.set_name("PanelMenuBar");
		box = new Gtk.HBox(false, 0);
		menubar.show_tabs = false;

		string menu_definition = 
		    "<popup name=\"button3\">" +
		        "<menuitem debuname=\"Preferences\" verb=\"Preferences\" _label=\"_Preferences\" pixtype=\"stock\" pixname=\"gtk-preferences\"/>" +
		        "<menuitem debuname=\"Help\" verb=\"Help\" _label=\"_Help\" pixtype=\"stock\" pixname=\"gtk-help\"/>" +
		     	"<menuitem debuname=\"About\" verb=\"About\" _label=\"_About...\" pixtype=\"stock\" pixname=\"gtk-about\"/>" +
		    "</popup>";
		    
		var verbPreferences = BonoboUI.Verb ();
		verbPreferences.cname = "Preferences";
		verbPreferences.cb = on_preferences_clicked;
		
		var verbAbout = BonoboUI.Verb ();
		verbAbout.cname = "About";
		verbAbout.cb = on_about_clicked;
		
		var verbHelp = BonoboUI.Verb ();
		verbHelp.cname = "Help";
		verbHelp.cb = on_help_clicked;
		
		var verbs = new BonoboUI.Verb[] { verbAbout, verbHelp, verbPreferences };
		setup_menu (menu_definition, verbs, null);

		switcher = new PanelExtra.Switcher();
		box.pack_start(switcher, false, true, 0);
		
		box.pack_start(menubar, true, true, 0);
		this.add(box);
		screen = Wnck.Screen.get_default();
		(screen as WnckCompat.Screen).active_window_changed += on_active_window_changed;
		
        show_all();
        
        get_prefs();
    }

    public static int main (string[] args) {
        var program = Gnome.Program.init ("GlobalMenuApplet", "0.6", Gnome.libgnomeui_module,
                                          args, "sm-connect", false);
                                          
        Gtk.rc_parse_string("""
			style "globalmenu_event_box_style"
			{
			 	GtkWidget::focus-line-width=0
			 	GtkWidget::focus-padding=0
			}
			style "globalmenu_menu_bar_style"
			{
				ythickness = 0
				GtkMenuBar::shadow-type = none
				GtkMenuBar::internal-padding = 0
			}
			class "GtkEventBox" style "globalmenu_event_box_style"
			class "GnomenuMenuBar" style:highest "globalmenu_menu_bar_style"
""");

        return Panel.Applet.factory_main ("OAFIID:GlobalMenu_PanelApplet_Factory",
                                    typeof (GlobalMenuApplet), factory);
    }
}
