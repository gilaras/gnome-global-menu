/*******************************
 * GConfDialog
 * Author: Luca Viggiani
 * Version: 0.1
 * Date: 2009-12-07
 * License: GPL
 * Description: creates a gconf settings back end dialog 
 * 				automatically from a settings key root.
 * Usage: new GConfDialog (string key, string dialog_title)
 * Params:
 * 	    string key: the gconf key to connect to (i.e /apps/panel/applets/applet_46/prefs)
 * 		string dislog_title: text to be shown on window title bar
 *		string[] subkeys: used to choose the subkeys to hadle (comprising display order).
 * 			  if null all subkeys will be retieved.
 * 
 * TODO: Only BOOL and INT data types are implemented. STRING is implemented only partially (read)
 *******************************/

using GLib;
using Gtk;
using GConf;

	public class GConfDialog : Gtk.Dialog {

		private GConf.Client _default_client;

		public GConfDialog(string dialog_title) {
			title = dialog_title;
			icon_name = "gtk-preferences";
		}

		construct {
			_default_client = GConf.Client.get_default();
			//vbox.width_request = 320;
			add_button(Gtk.STOCK_CLOSE, Gtk.ResponseType.CLOSE);
			add_button(Gtk.STOCK_HELP, Gtk.ResponseType.HELP);
		}

		public void add_key_group(string group_name, string[] keys) {
			var frame = new Gtk.Frame(group_name);
			var alignment = new Gtk.Alignment(0.0f, 0.0f, 1.0f, 1.0f);
			var box = new Gtk.VBox(false, 0);
			var label_widget = frame.label_widget;
			label_widget.modify_font(Pango.FontDescription.from_string("Bold"));
			frame.border_width = 10;
			frame.add(alignment);
			alignment.add(box);
			alignment.left_padding = 10;
//			frame.shadow_type = Gtk.ShadowType.NONE;
			try {
			foreach(weak string key in keys) {
				var widget = create_proxy_widget(key);
				box.pack_start(widget, false, false, 5);
			} } catch(GLib.Error e) {
				warning("%s", e.message);
			}
			vbox.pack_start(frame, true, false, 0);
			frame.show_all();
		}

		private Gtk.Widget create_proxy_widget(string key) throws GLib.Error {
			GConf.Entry entry = _default_client.get_entry(key, null, true);
       		GConf.Schema schema = safely_get_schema(entry);
			weak string tooltip = schema.get_long_desc();

			Gtk.Box row = new Gtk.HBox(false, 0);
       		
       		Gtk.Label label = new Gtk.Label(schema.get_short_desc());
       		label.justify = Gtk.Justification.LEFT;
			label.tooltip_text = tooltip;
       			
       		Gtk.Widget action_widget;
			Gtk.Widget render_widget;
       		switch(schema.get_type()) {
       			case ValueType.BOOL:
       				var checkbox = new Gtk.CheckButton();
       				checkbox.active = _default_client.get_bool(entry.key);
       				checkbox.clicked += onCheckButtonActivated;
					checkbox.add(label);
					render_widget = checkbox;
					action_widget = render_widget;
       				break;
       			case ValueType.STRING:
       				var entrybox = new Gtk.Entry();
					var box = new Gtk.HBox(false, 0);
       				entrybox.text = _default_client.get_string(entry.key);
       				/* TODO: connect changed signal to eventhandler */
					box.pack_start(label, false, false, 2);
					box.pack_start(entrybox, false, false, 2);
					action_widget = entrybox;
					render_widget = box;
       				break;
       			case ValueType.INT:
					var box = new Gtk.HBox(false, 0);
       				var spin = new Gtk.SpinButton.with_range(-100, 200, 1);
       				spin.value = _default_client.get_int(entry.key);
       				spin.value_changed += onSpinButtonValueChanged;
					box.pack_start(label, false, false, 2);
					box.pack_start(spin, false, false, 2);
					action_widget = spin;
					render_widget = box;
       				break;
       			default:
       				return new Gtk.EventBox();
			}
			action_widget.tooltip_text = tooltip;
			action_widget.set_data("gconf-entry", entry);
			action_widget.set_data("gconf-schema", schema);
				
			Gtk.Button reset = new Gtk.Button.from_stock(Gtk.STOCK_CLEAR);
			reset.tooltip_text = _("Reset to the default value");
//			reset.set_image(new Gtk.Image.from_stock("gtk-clear", Gtk.IconSize.SMALL_TOOLBAR));
			reset.set_data("gconf-entry", entry);
			reset.set_data("gconf-schema", schema);
			reset.set_data("target", action_widget);

			reset.clicked += onResetButtonPressed;
				
			row.pack_start(render_widget, false, false, 2);
			row.pack_end(reset, false, false, 2);
				
				
			return row;
		}
		private void onCheckButtonActivated(Gtk.CheckButton widget) {
			var entry = (GConf.Entry)widget.get_data("gconf-entry");
			try {
				_default_client.set_bool(entry.key, widget.active);
			} catch (GLib.Error e) {
				warning("%s", e.message);	
			}
		}
		
		private void onSpinButtonValueChanged(Gtk.SpinButton widget) {
			var entry = (GConf.Entry)widget.get_data("gconf-entry");
			try {
				_default_client.set_int(entry.key, (int)widget.value);
			} catch (GLib.Error e) {
				warning("%s", e.message);
			}
		}
		
		/**
		 * Safely obtain a schema for the entry.
		 * If no schema is found, return a fake schema.
		 */
		private GConf.Schema safely_get_schema(GConf.Entry entry) {
			weak string schema_name = entry.get_schema_name();

			if(schema_name == null) {
				critical("schema not found for entry %s", entry.get_key());
				GConf.Schema rt = new GConf.Schema();
				rt.set_short_desc(entry.get_key());
				rt.set_long_desc("This key lacks a schema");
				rt.set_type(ValueType.STRING);
				return rt;
			} else {
				return _default_client.get_schema(schema_name);
			}
		}
		private void onResetButtonPressed(Gtk.Button widget) {
			weak GConf.Schema schema = (GConf.Schema)widget.get_data("gconf-schema");
			weak GConf.Value default_value = schema.get_default_value();
			var target = (Gtk.Widget)widget.get_data("target");
			switch(schema.get_type()) {
       			case ValueType.BOOL:
       				var checkbutton = target as Gtk.CheckButton;
					checkbutton.active = default_value.get_bool();
       				break;
       			case ValueType.STRING:
					var entrybox = target as Gtk.Entry;
       				entrybox.text = default_value.get_string();
       				break;
       			case ValueType.INT:
       				var spin = target as Gtk.SpinButton;
					spin.value = default_value.get_int();
       				break;
				}
		}
	}
