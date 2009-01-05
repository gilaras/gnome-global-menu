[CCode (cheader_filename = "dyn-patch.h")]
namespace DynPatch {
[CCode (cname = "dyn_patch_init")]
	public void init();
[CCode (cname = "dyn_patch_uninit")]
	public void uninit();
[CCode (cname = "dyn_patch_set_menubar_r")]
	public void set_menubar_r(Gtk.Widget widget, Gtk.MenuBar? menubar);
}
