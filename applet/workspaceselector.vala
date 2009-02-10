using GLib;
using Gtk;
using Wnck;

class WorkspaceSelector : Gtk.Window {
	private Wnck.Window? target;
	private const int WIDTH = 128;
	
	public WorkspaceSelector(Wnck.Window? window) {
		target = window;
		this.name = "WorkspaceSelector";
        this.set_type_hint(Gdk.WindowTypeHint.DOCK);
        this.window_position = Gtk.WindowPosition.CENTER_ALWAYS;
        this.resizable = false;
        this.allow_grow = false;
        this.decorated = false;
        this.skip_pager_hint = true;
        this.skip_taskbar_hint = true;
        if ((this.child != null)) {
            this.child.show_all();
        }
        this.default_width = 100;
        this.default_height = 100;
        this.show();

        this.leave_notify_event += on_leave_notify;
        
        do_menu();
	}
	
	private bool on_leave_notify(WorkspaceSelector wss, Gdk.EventCrossing event) {
		this.destroy();
		return false;
	}
	
	private void do_menu() {
		VBox rows = new VBox(true, 0);
		HBox row;
		int nth_vdesktop = 0;				   
		for (int nrow=0; nrow<nrows; nrow++) {
			row = new HBox(true, 0);
			rows.pack_start(row, true, true, 2);
			for (int ncol=0; ncol<ncols; ncol++) {
				Gtk.Image i = new Gtk.Image.from_pixbuf(get_mini_screenshot(nth_vdesktop));
				row.pack_start(i, true, true, 2);
				nth_vdesktop++;
			}
		}
		
		this.add(rows);
		this.show_all();
	}
	private Gdk.Pixbuf get_mini_screenshot(int nth_vdesktop) {
		/* TOFIX:
		 * This will always return what is being shown on the screen
		 * which is not a true screen capture of the Desktop window
		 * but what is actually being rendered (i.e. the Desktop cube rotating).
		 * This function should return the actual screen show of selected 
		 * virtual Desktop (i.e. Compiz cube face) */
		
		weak Gdk.Window root = Gdk.get_default_root_window();
			
		int x, y, width, height, depth;
		root.get_geometry(out x, out y, out width, out height, out depth);
		
		Gdk.Pixbuf ss = new Gdk.Pixbuf(Gdk.Colorspace.RGB,
									   false,
									   8,
									   width,
									   height);
									   
		Gdk.Pixbuf pb = new Gdk.Pixbuf(Gdk.Colorspace.RGB,
									   false,
									   8,
									   WIDTH,
									   (int)((double)WIDTH/(double)width * height));
		
		Gdk.pixbuf_get_from_drawable(ss,
									 root,
									 root.get_colormap(),
									 0, 0,
									 0, 0,
									 width, height);
									 
		double ratio = (double)pb.width / (double)ss.width;
		ss.scale(pb,
				 0, 0,
				 pb.width, pb.height,
				 0, 0,
				 ratio, ratio,
				 Gdk.InterpType.BILINEAR); 
		return pb;
	}
	public int nrows {
		get {
			return workspace_height / screen_height;
		}
	}
	public int ncols {
		get {
			return workspace_width / screen_width;
		}
	}
	public int screen_width {
		get {
			return target.get_screen().get_width();
		}
	}
	public int screen_height {
		get {
			return target.get_screen().get_height();
		}
	}
	public int workspace_width {
		get {
			return target.get_workspace().get_width();
		}
	}
	public int workspace_height {
		get {
			return target.get_workspace().get_height();
		}
	}
}
