namespace Gdk {
[CCode (cname = "GDK_WINDOW_XID", cheader_filename="gdk/gdkx.h")]
public static ulong XWINDOW(Gdk.Drawable drawable);
}

[CCode (cprefix = "Gtk", lower_case_cprefix = "gtk_")]
namespace GtkCompat {
	[CCode (cheader_filename = "gtk/gtk.h", cname="GtkNotebook")]
	public class Notebook : Gtk.Container, Atk.Implementor, Gtk.Buildable {
		public int page_num(Gtk.Widget child);
	}
	[CCode (cheader_filename = "gtk/gtk.h")]
	public class Container : Gtk.Widget, Atk.Implementor, Gtk.Buildable {
		[CCode (cname = "gtk_container_forall")]
		public void forall_children (Gtk.Callback callback);
		public virtual void forall (bool include_internals, Callback callback, void* data);
	}
	[CCode (cheader_filename = "gtk/gtk.h")]
	public class Widget : Gtk.Object, Atk.Implementor, Gtk.Buildable {
		public virtual signal void style_set (Gtk.Style? previous_style);
		public virtual signal void hierarchy_changed (Gtk.Widget? old_toplevel);
	}

	public static delegate void Callback(Gtk.Widget widget, void * data);

	[CCode (cheader_filename = "gtk/gtk.h")]
	public class TreeView : Gtk.Container, Atk.Implementor, Gtk.Buildable {
		public int insert_column_with_data_func(int pos, 
						string title, Gtk.CellRenderer cell, 
						Gtk.TreeCellDataFunc func, 
						GLib.DestroyNotify? dnotify);
	}
	[CCode (cheader_filename = "gtk/gtk.h")]
	public class Style : GLib.Object {
		public virtual Gtk.Style copy ();
	}
}

[CCode (cprefix = "Pango", lower_case_cprefix = "pango_")]
namespace PangoCompat {
	[Compact]
	[CCode (copy_function = "pango_font_description_copy", cheader_filename = "pango/pango.h", free_function="pango_font_description_free")]
	public class FontDescription {
		public static Pango.FontDescription from_string (string str);
	}
}
