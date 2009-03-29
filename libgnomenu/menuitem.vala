using Gtk;

namespace Gnomenu {
	/**
	 * fancy menu item used by GlobalMenu.PanelApplet
	 *
	 * Difference with Gtk.MenuItem:
	 *
	 * The various derived XXXMenuItem types are integrated
	 * into Gnomenu.MenuItem to allow easier tranformation
	 * from XML representation to Widgets.
	 *
	 * label and image are directly set on the MenuItem.
	 * each item has an id,
	 * associated with a position(after attached to a MenuShell)
	 *
	 * MenuItem also holds a back-reference to the toplevel 
	 * menu bar. When an item is activated, the activated signal
	 * on the menu bar is also invoked.
	 * 
	 */
	public class MenuItem : Gtk.MenuItem, Gnomenu.Item {
		public MenuItem() { }

		static int icon_width;
		static int icon_height;
		static construct {
			install_style_property(new 
					ParamSpecInt("indicator-size", 
						_("Indicator Size"),
						_("Size of check or radio indicator"), 
						0, int.MAX, 13, ParamFlags.READABLE));
			icon_size_lookup (IconSize.MENU, out icon_width, out icon_height);
			/*Load gtk-menu-images setting*/
			Gtk.ImageMenuItem item = new Gtk.ImageMenuItem();
		}

		construct {
			disposed = false;
			_item_type = ItemType.NORMAL;
			_cached_label_widget = new MenuLabel();
			_cached_image_widget = new Image();
			_cached_arrow_widget = new Arrow(gravity_to_arrow_type(_gravity), ShadowType.NONE);
			_use_underline = true;
			create_labels();
		}

		public override void dispose() {
			if(!disposed) {
				disposed = true;
				if(_image_widget != null) {
					_image_widget.unparent();
					_image_widget = null;
				}
			}
			base.dispose();
		}

		private int _max_width_chars = 30;
		public int max_width_chars {
			get {
				return _max_width_chars;
			}
			set {
				_max_width_chars = value;
				update_label_text();
			}
		}
		public Shell shell {get {
			return parent as Shell;
		}}
		public Shell sub_shell{get {
			return this.submenu as Shell;
		}}
		public Menu _submenu_cache = new Menu();
		public bool has_sub_shell {
			get {
				return this.submenu != null;
			}
			set {
				if(value) {
					this.submenu = _submenu_cache;
				} else {
					if(this.submenu != null) {
						this.submenu.popdown();
					}
					this.submenu = null;
				}
			}
		}

		/**
		 * the id of the menu item.
		 *
		 * id is used to construct the path to uniquely represent
		 * the item in the menubar.
		 */
		public string? item_id { 
			get { return _id; }
		   	set { 
				if(_id == value) return;
				_id = value; 
				if(_label == null)
				update_label_text();
			}
		}
		/**
		 * the label text in the item.
		 *
		 */
		public string? item_label {
			get { return _label; }
			set {
				if(_label == value) return;
				_label = value;
				update_label_text();
			}
		}
		/**
		 * the icon in the item.
		 * Notice that the specific meaning of 
		 * the string is not yet defined.
		 *
		 * For now, it can be any of the Gtk stock item
		 * names.
		 */
		public string? item_icon {
			get { return _icon; }
			set {
				if(_icon == value) return;
				_icon = value;
				update_image();
			}
		}
		/**
		 * the text to describe the accelerator key combination.
		 *
		 * Notice that this is nothing more than a text.
		 * The applet doesn't handle these accelerator keys.
		 */
		public string? item_accel_text {
			get { return _accel_text; }
			set {
				if(_accel_text == value) return;
				_accel_text = value;
				update_label_text();
			}
		}

		/**
		 * the font description of the label.
		 * It can be any valid PangoFontDescription string.
		 *
		 * e.g "bold", "Sans Bold".
		 *
		 * Anything more than "Bold" is not recommend
		 * since it interacts badly with themes.
		 */
		public string? item_font {
			get { return _font; }
			set {
				if(_font == value) return;
				_font = value;
				update_font();
			}
		}


		/**
		 * set/get the type string of the item.
		 * Valid values can be found in
		 *
		 * { @link item_type_to_string }
		 *
		 */
		public ItemType item_type {
			get { return _item_type; }
			set {
				if(value == _item_type) return;
				ItemType old_type = _item_type;
				_item_type = value;
				switch(_item_type) {
					case ItemType.SEPARATOR:
						remove_child();
					break;
					case ItemType.NORMAL:
					case ItemType.IMAGE:
					case ItemType.CHECK:
					case ItemType.RADIO:
						if(old_type != ItemType.NORMAL
						&& old_type != ItemType.IMAGE
						&& old_type != ItemType.RADIO
						&& old_type != ItemType.CHECK
						) {
							remove_child();
							create_labels();
							update_label_text();
						}
					break;
					case ItemType.ARROW:
						if(old_type != ItemType.ARROW) {
							remove_child();
							create_arrow();
							update_arrow_type();
						}
					break;
					case ItemType.ICON:
						if(old_type != ItemType.ICON) {
							remove_child();
							_icon_widget = new Image();
							_icon_widget.visible = true;
							add(_icon_widget);
							update_image();
						}
					break;
				}
				if(_item_type == ItemType.IMAGE) {
					_image_widget = _cached_image_widget;
					_image_widget.sensitive = true;
					_image_widget.set_parent(this);
					_image_widget.visible = _show_image;
					update_image();
				} else {
					if(_image_widget != null) {
						_image_widget.unparent();
						_image_widget = null;
					}
				}
				queue_resize();
			}
		}
		public bool item_use_underline {
			get {
				return _use_underline;
			}
			set {
				if(_use_underline == value) return;
				_use_underline = value;
				update_label_underline();
			}
		}

		/**
		 * get/set the state of the item. It can be either triggled or not,
		 * or in a tristate.
		 *
		 * { @link item_state_to_string }
		 */
		public ItemState item_state {
			get { return _item_state; }
			set {
				if(value == _item_state) return;
				_item_state = value; 
				queue_draw();
			}
		}
		private bool _item_visible;
		public bool item_visible {
			get { return _item_visible;}
			set { _item_visible = value;
				visible = _item_visible && !_truncated;}
		}
		public bool item_sensitive {
			get { return sensitive;}
			set { sensitive = value;}
		}
		public Gravity gravity {
			get { return _gravity; }
			set {
				if(_gravity == value) return;
				_gravity = value;
				if(Item.type_has_label(_item_type))
					get_label_widget().gravity = value;
				update_arrow_type();
			}
		}
		
		/*used by MenuShell truncate and the serializer.*/
		public bool truncated {
			set {
				_truncated = value;
				visible = _item_visible && !truncated;
			}
			get {
				return _truncated;
			}
		}

		private bool disposed;

		private bool _truncated;

		private string _path; /*merely a buffer*/
		private string _font;
		private string _label;
		private string _icon;
		private string _accel_text;
		private string _id;
		private Gravity _gravity;
		private ItemType _item_type;
		private ItemState _item_state;
		private bool _use_underline = false;

		private Image _cached_image_widget;
		private Arrow _cached_arrow_widget;
		private MenuLabel _cached_label_widget;

		private bool _show_image {
			get {
				bool rt = false;
				get_settings().get("gtk-menu-images", &rt, null);
				return rt;
			}
		}

		public Gtk.Image? image {
			get {
				if(_item_type == ItemType.IMAGE)
					return _image_widget;
				if(_item_type == ItemType.ICON)
					return _icon_widget;
				return null;
			}
		}
		private Gtk.Image _image_widget;
		private Gtk.Image _icon_widget;

		private Gtk.PackDirection pack_direction {
			get {
				if(parent is Gtk.MenuBar) {
					return (parent as Gtk.MenuBar).child_pack_direction;
				}
				return PackDirection.LTR;
			}
		}
		public override void toggle_size_request(void* requisition) {
			int toggle_spacing = 0;
			int indicator_size = 0;
			style_get("toggle-spacing", &toggle_spacing,
				"indicator-size", &indicator_size, null);
			switch(_item_type) {
				case ItemType.CHECK:
				case ItemType.RADIO:
					*((int*) requisition ) = indicator_size + toggle_spacing;
				break;
				case ItemType.IMAGE:
					if(!_show_image) {
						*((int*) requisition) = 0;
						break;
					}
					if(image != null && _icon != null) {
						Requisition req;
						image.size_request(out req);
						if(pack_direction == PackDirection.LTR ||
							pack_direction == PackDirection.RTL )
						*((int*) requisition ) = req.width + toggle_spacing;
						if(pack_direction == PackDirection.TTB ||
							pack_direction == PackDirection.BTT )
						*((int*) requisition ) = req.height+ toggle_spacing;
					} else {
						/*no image*/
						*((int*) requisition ) = 0;
					}
				break;
				default:
					*((int*) requisition ) = 0;
				break;
			}
		}

		/**
		 *
		 * The parent handler is first invoked.
		 * Then the check box or the radio box
		 * is drawn.
		 *
		 */
		public override bool expose_event(Gdk.EventExpose event) {
			base.expose_event(event);
			int toggle_spacing = 0;
			int indicator_size = 0;
			int horizontal_padding = 0;
			style_get(
				"toggle-spacing", &toggle_spacing,
				"indicator-size", &indicator_size,
 			    "horizontal-padding", &horizontal_padding,
				null);
			ShadowType shadow_type = item_state_to_shadow_type(_item_state);
			/*FIXME: alignment !*/
			int x = 0;
			int y = 0;
			int xoffset = (toggle_size - indicator_size)/2;
			int yoffset = (allocation.height - indicator_size)/2;
			int spacing = toggle_spacing /2;
			switch(get_direction()) {
				case Gtk.TextDirection.LTR:
					x = allocation.x + xoffset + spacing;
					y = allocation.y + yoffset;
				break;
				case Gtk.TextDirection.RTL:
					x = allocation.x + allocation.width - toggle_size - xoffset - spacing;
					y = allocation.y + yoffset;
				break;
			}
			switch(_item_type) {
				case ItemType.CHECK:
					Gtk.paint_check(style,
						window,
						(StateType)state,
						shadow_type,
						event.area, 
						this,
						"check",
						x,
						y,
						indicator_size,
						indicator_size);
				break;
				case ItemType.RADIO:
					Gtk.paint_option(style,
						window,
						(StateType)state,
						shadow_type,
						event.area, 
						this,
						"option",
						x,
						y,
						indicator_size,
						indicator_size);
				break;
			}
			return false;
		}

		/**
		 * Overriding Gtk.Container.forall.
		 *
		 * There is a noterious problem in Vala Gtk bindings, causing
		 * the ccode function differs from GtkContainerClass.forall.
		 *
		 * Therefore this function's ccode is patched with patch.sh
		 * to avoid the problems. the dummy variable include_internals
		 * should be kept here.
		 *
		 */
		public override void forall(bool include_internals, Gtk.Callback callback) {
			if(include_internals) {
				/*_image_widget still can be null
				 * if item type is IMAGE,
				 * -- forall can be called during the construction
				 *  of the widget by remove_child()*/
				if(_item_type == ItemType.IMAGE
				&& _image_widget != null)
					callback(_image_widget);
			}
			base.forall(include_internals, callback);
		}
		public override void activate() {
			(toplevel_shell as MenuBar).activate(this);
		}
		private void update_show_image() {
			if(_image_widget != null) {
				_image_widget.visible = _show_image;
			}
			queue_resize();
		}
		private static void show_image_notify_r(Gtk.Widget widget, Gtk.Settings settings) {
			if(widget is MenuItem) {
				MenuItem item = widget as MenuItem;
				item.update_show_image();
			} else {
				if(widget is Container) {
					List<weak Gtk.Widget> children = (widget as Container).get_children();
					foreach(Gtk.Widget child in children)
						show_image_notify_r(child, settings);
				}
			}
		}
		private static void show_image_notify(Gtk.Settings settings) {
			List<weak Gtk.Window> toplevels = Gtk.Window.list_toplevels();
			foreach(Gtk.Container c in toplevels) {
				show_image_notify_r(c, settings);
			}
		}
		public override void screen_changed(Gdk.Screen? previous_screen) {
			if(!has_screen()) return;
			Gtk.Settings settings = get_settings();
			if(settings.get_data("gnomenu-menu-item-connection") == null) {
				settings.notify["gtk-menu-images"] += show_image_notify;
				/*set it to non-null value*/
				settings.set_data("gnomenu-menu-item-connection", settings);
				/*if there is no such connection yet, 
				 * this widget must have been the first
				 * item attached to the screen, therefore there is no
				 * need to do a show_image_notify to all widgets.
				 * */
			}
			/*If the connection to settings is already established,
			 * updating this widget is sufficient*/
			update_show_image();
		}
		public override void size_request(out Requisition req) {
			if(_item_type == ItemType.IMAGE) {
				Requisition image_req;
				_image_widget.size_request(out image_req); 
				/*Then throw it away*/
			}
			if(_item_type == ItemType.ICON) {
				int horizontal_padding = 0;
				style_get ("horizontal-padding", 
						&horizontal_padding,
						null);
				_icon_widget.size_request(out req);
				req.width += (int)border_width * 2 + horizontal_padding * 2;
				req.height += (int)border_width * 2;
			} else {
				base.size_request(out req);	
			}
		}
		public override void size_allocate(Gdk.Rectangle a) {
			Gdk.Rectangle ca = {0, 0, 0, 0};
			if(_item_type == ItemType.ICON) {
				int horizontal_padding = 0;
				style_get ("horizontal-padding", 
						&horizontal_padding,
						null);
				ca.x = a.x + (int)border_width + horizontal_padding;
				ca.y = a.y + (int)border_width;
				ca.width = a.width - (int)border_width * 2 - horizontal_padding;
				ca.height = a.height - (int)border_width * 2;
				_icon_widget.size_allocate(ca);
				if((get_flags() & WidgetFlags.REALIZED) != 0) {
					event_window.move_resize(allocation.x,
							allocation.y,
							allocation.width, allocation.height);
				}
				allocation = (Allocation) a;
			} else {
				base.size_allocate(a);
			}
			if(_item_type == ItemType.IMAGE) {
				/*FIXME: alignment !*/
				int toggle_spacing = 0;
				Requisition icon_req;
				_image_widget.get_child_requisition(out icon_req);
				style_get(
					"toggle-spacing", &toggle_spacing,
					null);
				ca.width = icon_req.width;
				ca.height = icon_req.height;
				switch(pack_direction) {
					case PackDirection.LTR:
					case PackDirection.RTL:
						int xoffset = (toggle_size - icon_req.width + toggle_spacing)/2;
						int yoffset = (a.height - icon_req.height)/2;
						ca.y = a.y + yoffset;
						if(get_direction() == TextDirection.LTR
						&& pack_direction == PackDirection.LTR)
							ca.x = a.x + xoffset;
						else
							ca.x = a.x + a.width - ca.width - xoffset;
					break;
					case PackDirection.TTB:
					case PackDirection.BTT:
						int yoffset = (toggle_size - icon_req.height + toggle_spacing)/2;
						int xoffset = (a.width- icon_req.width)/2;
						ca.x = a.x + xoffset;
						if(get_direction() == TextDirection.LTR
						&& pack_direction == PackDirection.TTB)
							ca.y = a.y + yoffset;
						else
							ca.y = a.y + a.height - ca.height- yoffset;
					break;
				}
				_image_widget.size_allocate(ca);
			}
		}
		private void update_font() {
			Pango.FontDescription desc;
			if(_font != null) 
				desc = Pango.FontDescription.from_string(_font);
			else 
				desc = null;
			weak Widget bin_child = get_child();
			bin_child.modify_font(desc);
		}
		private void update_label_text() {
			if(!Item.type_has_label(_item_type)) return;
			string text;
			text = _label;
			if(text == null)
				text = item_path;

			MenuLabel label = get_label_widget();;
			assert(label != null);
			label.label = text;
			label.accel = _accel_text;
			label.max_width_chars = _max_width_chars;
		}
		private void update_label_underline() {
			if(!Item.type_has_label(_item_type)) return;
			MenuLabel label = get_label_widget();;
			assert(label != null);
			label.use_underline = _use_underline;
		}
		private void update_image() {
			if(_item_type != ItemType.IMAGE
			&& _item_type != ItemType.ICON) return;
			if(_icon != null && _icon.has_prefix("theme:")) {
				weak string icon_name = _icon.offset(6);
				image.set_from_icon_name(icon_name, IconSize.MENU);
			} else 
			if(_icon != null && _icon.has_prefix("file:")) {
				weak string filename = _icon.offset(5);
				image.set_from_file(filename);
			} else 
			if(_icon != null && _icon.has_prefix("pixbuf:")) {
				weak string b64data = _icon.offset(7);
				int len = 0;
				uchar [] data = Base64.decode(b64data, out len);
				Gdk.Pixdata pixdata = {0};
				pixdata.deserialize(data);
				Gdk.Pixbuf pixbuf = Gdk.Pixbuf.from_pixdata(pixdata, true);
				image.set_from_pixbuf(pixbuf);
			} else 
			if(_icon != null && _icon.has_prefix("custom:")) {
				/*Do nothing*/
			} else {
				image.set_from_stock(_icon, IconSize.MENU);
			}
		}
		public override void parent_set(Gtk.Widget? old_parent) {
			if(parent != null)
				update_label_text();
		}
		private void create_labels() {
			assert(Item.type_has_label(_item_type));
			_cached_label_widget.visible = true;
			_cached_label_widget.sensitive = true;
			_cached_label_widget.gravity = gravity;
			add(_cached_label_widget);
			update_font();	
			update_label_underline();
		}
		private weak MenuLabel? get_label_widget() {
			weak MenuLabel label = get_child() as MenuLabel;
			return label;
		}
		private void remove_child() {
			Gtk.Widget child = get_child();
			if(child != null) remove(child);
		}
		private void create_arrow() {
			assert(_item_type == ItemType.ARROW);
			add(_cached_arrow_widget);
			_cached_arrow_widget.visible = true;
			_cached_arrow_widget.sensitive = true;
			update_arrow_type();
		}
		private void update_arrow_type() {
			if(_item_type != ItemType.ARROW) return;
			(get_child() as Arrow).set(gravity_to_arrow_type(_gravity), ShadowType.NONE);
		}
		public static ShadowType item_state_to_shadow_type(ItemState state) {
			switch(state) {
				case ItemState.TOGGLED:
					return ShadowType.IN;
				case ItemState.UNTOGGLED:
					return ShadowType.OUT;
				case ItemState.TRISTATE:
				default:
					return ShadowType.ETCHED_IN;
			}
		}
	}
}
