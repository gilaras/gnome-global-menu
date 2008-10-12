using GLib;
using Gtk;
using GMarkupDoc;
using Gnomenu;



public class Navigator :Gtk.Window{
	private ListView server_viewer;
	private ListView viewer;
	private MenuView viewer2;
	private RemoteDocument server;
	public Navigator() {
		type = Gtk.WindowType.TOPLEVEL;
	}
	construct {
		server = new RemoteDocument("org.gnome.GlobalMenu.Server", "/org/gnome/GlobalMenu/Server");
		server_viewer = new ListView(server);
		viewer = new ListView(null);
		viewer2 = new MenuView(null);
		Gtk.Paned vpan = new Gtk.VPaned();
		Gtk.Box vbox = new Gtk.VBox(false, 0);
		this.add(vpan);
		vpan.pack1(server_viewer, true, true);
		vpan.pack2(vbox, true, false);
		vbox.pack_start(viewer2, false, true, 0);
		vbox.pack_start_defaults(viewer);
		
		server.activated += (docu, node, detail)=> {
			string bus = (node as GMarkupDoc.Tag).get("bus");
			print("attatch to bus %s", bus);
			RemoteDocument doc = new RemoteDocument(bus, "/org/gnome/GlobalMenu/Application");
			doc.activated += (docu, node, detail) => {
				if((node as GMarkupDoc.Tag).tag == "menubar") {
					Section section = new Section(docu, node);
					viewer2.document = section;
				}
			};
			viewer.document = doc;
		};
	}
	public static int main(string[] args) {
		Gtk.init(ref args);
		MainLoop loop = new MainLoop(null, false);

		Navigator nav = new Navigator();
		nav.show_all();
		nav.delete_event += (widget) => {
			Gtk.main_quit();
			return false;
		};
		loop.run();
		return 0;	
	}

}
