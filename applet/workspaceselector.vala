using GLib;
using Gtk;
using Wnck;

extern Gdk.Pixbuf*
_wnck_gdk_pixbuf_get_from_pixmap (Gdk.Pixbuf*   dest,
                                  Gdk.Pixmap*   xpixmap,
                                  int          src_x,
                                  int          src_y,
                                  int          dest_x,
                                  int          dest_y,
                                  int          width,
                                  int          height);
                                  
class WorkspaceSelector : Gtk.Window {
	private Wnck.Window? target;
	private const int WIDTH = 128;
	private GLib.List<WorkspaceItem> workspaces;
	
	public WorkspaceSelector(Wnck.Window? window) {
		target = window;
        do_menu();
	}
	construct {
		this.name = "WorkspaceSelector";
        this.set_type_hint(Gdk.WindowTypeHint.MENU);
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
        this.leave_notify_event += on_leave_notify;
        this.button_press_event += on_button_press;
		this.button_release_event += on_button_release;
	}
	
	private bool on_leave_notify(WorkspaceSelector wss, Gdk.EventCrossing event) {
		if (event.state==0)
			this.destroy();
		return false;
	}
	private bool on_button_press(WorkspaceSelector wss, Gdk.EventButton event) {
		return false;
	}
	
	private bool on_button_release(WorkspaceSelector wss, Gdk.EventButton event) {
		if (selected_item == null) return false;
		
		/* move the window! */
		int x, y, w, h;
		(target as WnckCompat.Window).get_geometry(out x, out y, out w, out h);
		x += selected_item.viewport_x - target.get_workspace().get_viewport_x();
		y += selected_item.viewport_y - target.get_workspace().get_viewport_y();
		
		/* TOFIX: x and y are always shifted... perhaps by window decoration size? */
		
		target.set_geometry(Wnck.WindowGravity.CURRENT,
							Wnck.WindowMoveResizeMask.X | Wnck.WindowMoveResizeMask.Y,
							x, y, w, h);
		
		/* then exit */
		this.destroy();
		return false;
	}
	private void do_menu() {
		VBox rows = new VBox(true, 0);
		HBox row;
		int nth_vdesktop = 0;				   
		for (int nrow=0; nrow<nrows; nrow++) {
			row = new HBox(true, 0);
			rows.pack_start(row, true, true, 0);
			for (int ncol=0; ncol<ncols; ncol++) {
				WorkspaceItem wi = new WorkspaceItem.with_pixbuf(get_mini_screenshot(nth_vdesktop));
				row.pack_start(wi, true, true, 0);
				wi.viewport_x = ncol * screen_width;
				wi.viewport_y = nrow * screen_height;
				wi.current = ((current_row == nrow) && (current_column == ncol));
				workspaces.append(wi);
				nth_vdesktop++;
			}
		}
		
		this.add(rows);
		this.show_all();
	}
	private Gdk.Pixbuf get_mini_screenshot(int nth_vdesktop) {
		/* TODO:
		 * Render also windows in selected Desktop */

		Gdk.Pixmap *p = (Gdk.Pixmap*)target.get_screen().get_background_pixmap();
		
		/* create fullsize Pixbuf */
		Gdk.Pixbuf ss = _wnck_gdk_pixbuf_get_from_pixmap(null,
                                              			 p,
                                              			 0, 0,
                                              			 0, 0,
                                              			 -1, -1);
		
		/* created thumbnail Pixbuf */			   
		Gdk.Pixbuf pb = new Gdk.Pixbuf(Gdk.Colorspace.RGB,
									   false,
									   8,
									   WIDTH,
									   (int)((double)WIDTH/(double)screen_width * screen_height));

		
		/* resize */
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
	public int current_column {
		get {
			return target.get_workspace().get_viewport_x() / screen_width;
		}
	}
	public int current_row {
		get {
			return target.get_workspace().get_viewport_y() / screen_height;
		}
	}
	public WorkspaceItem selected_item {
		get {
			foreach(weak WorkspaceItem wi in workspaces)
				if (wi.selected) return wi;
			return null;
		}
	}
}


/* WorkspaceItem */

class WorkspaceItem : Gtk.EventBox {
	public int viewport_x;
	public int viewport_y;
	
	private Gtk.Image _image;
	private int _margin = 6;
	private bool _selected = false;
	private bool _current = false;
	
	public WorkspaceItem() {
		_image = null;
		this.show();
	}
	public WorkspaceItem.with_pixbuf(Gdk.Pixbuf pixbuf) {
		image = new Gtk.Image.from_pixbuf(pixbuf);
		this.show_all();
	}
	construct {
		this.style = rc_get_style(new Gtk.Menu());
		this.leave_notify_event += on_leave_notify;
		this.enter_notify_event += on_enter_notify;
	}
	
	public Gtk.Image image {
		get { return _image; }
		set {
			if (_image!=null)
				remove(_image);
			_image = value;
			if (_image!=null)
				add(_image);
			update();
		}
	}
	public int margin {
		get { return _margin; }
		set {
			_margin = value;
			update();
		}
	}
	public bool selected {
		get { 
			return _selected;
		}
		set {
			_selected = value;
			if (_selected)
				set_state(Gtk.StateType.SELECTED); else
				if (!current) set_state(Gtk.StateType.NORMAL);
		}
	}
	public bool current {
		get { return _current; }
		set {
			_current = value;
			if (_current) set_state(Gtk.StateType.SELECTED);
		}
	}
	private void update() {
		if (image==null) return;
		this.width_request = image.pixbuf.width + margin * 2;
		this.height_request = image.pixbuf.height + margin * 2;
	}
	
	private bool on_leave_notify(WorkspaceItem wi, Gdk.EventCrossing event) {
		if (event.state==0)
			selected = false;
		return false;
	}
	
	private bool on_enter_notify(WorkspaceItem wi, Gdk.EventCrossing event) {
		selected = true;
		return false;
	}
}
