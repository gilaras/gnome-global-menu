/* libpanelapplet-2.0.vala
 *
 * Copyright (C) 2007  Jürg Billeter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * Author:
 * 	Jürg Billeter <j@bitron.ch>
 */

[CCode (cheader_filename = "panel-applet.h")]
namespace Panel {
	public class Applet : Gtk.EventBox {
		[CCode (has_construct_function = false)]
		public Applet ();
		public string get_preferences_key ();
		public void add_preferences(string schema_dir) throws GLib.Error;
		public static int factory_main (string iid, GLib.Type applet_type, AppletFactoryCallback callback);
		public void set_background_widget (Gtk.Widget widget);
		public AppletBackgroundType get_background (out Gdk.Color color, out Gdk.Pixmap pixmap);
		[NoArrayLength]
		public void setup_menu (string xml, BonoboUI.Verb[] verb_list, void* data);

		public virtual signal void change_background (AppletBackgroundType type, Gdk.Color? color, Gdk.Pixmap? pixmap);
		public virtual signal void change_orient (AppletOrient orient);
		public virtual signal void change_size (uint size_hint);

		public AppletOrient orient {get;}
		public AppletFlags flags {get; set;}
		public uint size {get;}
		public bool locked_down {get;}
		public void request_focus(uint32 timestamp);

		public bool gconf_get_bool(string key) throws GLib.Error;
		public void gconf_set_bool(string key, bool value) throws GLib.Error;
		public weak string gconf_get_string(string key) throws GLib.Error;
		public void gconf_set_string(string key, string value) throws GLib.Error;
		public int gconf_get_int(string key) throws GLib.Error;
		public void gconf_set_int(string key) throws GLib.Error;
	}
	[CCode (cprefix = "PANEL_APPLET_ORIENT_")]
	public enum AppletOrient {
		UP,
		DOWN,
		LEFT,
		RIGHT
	}
	[CCode (cprefix = "PANEL_")]
	public enum AppletBackgroundType {
		NO_BACKGROUND,
		COLOR_BACKGROUND,
		PIXMAP_BACKGROUND
	}

	[CCode (cprefix = "PANEL_APPLET_")]
	public enum AppletFlags {
		FLAGS_NONE,
		EXPAND_MAJOR,
		EXPAND_MINOR,
		HAS_HANDLE
	}

	public delegate bool AppletFactoryCallback (Applet applet, string iid);
}

