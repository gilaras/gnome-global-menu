%define base_version 	0.7.4
%define full_version	%{base_version}_svn2481

Name:		gnome-globalmenu
Version:	%{full_version}
Release:	4%{?dist}
Summary:	Global Menu for GNOME
Group:		User Interface/Desktops
License:	GPLv2+
URL:		http://code.google.com/p/gnome2-globalmenu/
Source0:		http://gnome2-globalmenu.googlecode.com/files/gnome-globalmenu-%{base_version}.tar.bz2
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXXX)
BuildRequires: libwnck-devel
BuildRequires: intltool
BuildRequires: libXres-devel
BuildRequires: gnome-panel-devel
BuildRequires: libnotify-devel
BuildRequires: gnome-menus-devel
Requires(post): ldconfig
Requires(postun): ldconfig
Requires(pre): GConf2
Requires(post): GConf2
Requires(preun): GConf2

%if 0%{?rhel5}
%else
BuildRequires: xfce4-panel-devel
%endif

%description
GNOME Global Menu is a centralized menu bar for all windows on a particular 
screen/session. This package extends GTK and gnome panel so that Global Menu 
can be enabled on all GTK applications.
This package contains shared data and libraries of various Global Menu 
packages. 
The Gtk Plugin Module of Global Menu adds global menu feature to any GTK 
applications on the fly without the need of modifying the source code.
gnome-applet-globalmenu or xfce-globalmenu-plugin should also be installed
depending on the desktop environment.

%package		devel
Summary:		Header files for writing Global Menu applets
Group:			User Interface/Desktops
Requires:		pkgconfig
Requires:		gnome-globalmenu = %{version}-%{release}
%description	devel
This package contains header files for writing Global Menu applet in Gtk.

%package -n 	gnome-applet-globalmenu
Summary:		GNOME panel applet of Global Menu
Group:			User Interface/Desktops
Requires:		gnome-panel
Requires:		gnome-globalmenu = %{version}-%{release}
%description -n	gnome-applet-globalmenu
The GNOME panel applet of Global Menu is a representation of Global Menu 
with GTK widgets. The applet can be inserted to the default top panel to 
provide access to the Global Menu of the applications. 
The applet also provides limited window management functionalities.

%if 0%{?rhel5}
%else
%package -n		xfce4-globalmenu-plugin
Summary:		XFCE panel applet of Global Menu
Group:			User Interface/Desktops
Requires:		xfce4-panel
Requires:		gnome-globalmenu = %{version}-%{release}
%description -n	xfce4-globalmenu-plugin
The XFCE panel applet of Global Menu is a representation of Global Menu 
with GTK widgets. The applet can be inserted to the default top panel 
to provide access to the Global Menu of the applications. 
%endif

%prep
%setup -q -n %{name}-%{base_version}


%build
%if 0%{?rhel5}
%configure --disable-schemas-install --disable-static --disable-tests --with-gnome-panel --without-xfce4-panel
%else
%configure --disable-schemas-install --disable-static --disable-tests --with-gnome-panel --with-xfce4-panel
%endif
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
%find_lang %{name}
rm -f $RPM_BUILD_ROOT/%{_libdir}/gtk-2.0/modules/libglobalmenu-gnome.la
rm -f $RPM_BUILD_ROOT/%{_libdir}/libgnomenu.la

%clean
rm -rf $RPM_BUILD_ROOT

%pre
if [ "$1" -gt 1 ] ; then
	export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
	gconftool-2 --makefile-uninstall-rule \
	%{_sysconfdir}/gconf/schemas/gnome-globalmenu.schemas >/dev/null || :
fi

%post
/sbin/ldconfig
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule \
		%{_sysconfdir}/gconf/schemas/gnome-globalmenu.schemas > /dev/null || :

%postun
/sbin/ldconfig
# comment

%preun 
if [ "$1" -eq 0 ] ; then
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-uninstall-rule \
		%{_sysconfdir}/gconf/schemas/gnome-globalmenu.schemas > /dev/null || :
		fi

%files -f %{name}.lang
%defattr(-,root,root,-)
%doc %{_datadir}/doc/gnome-globalmenu/*
%{_sysconfdir}/gconf/schemas/gnome-globalmenu.schemas
%{_datadir}/pixmaps/globalmenu.png
%{_libdir}/libgnomenu-%{base_version}.so.2
%{_libdir}/libgnomenu-%{base_version}.so.2.0.0
%{_mandir}/man1/gnome-globalmenu.1.gz
%{_libdir}/gtk-2.0/modules/libglobalmenu-gnome.so
%{_libdir}/gtk-2.0/modules/libglobalmenu-gnome-%{base_version}.so

%files devel
%defattr(-,root,root,-)
%{_includedir}/libgnomenu/*.h
%{_libdir}/pkgconfig/libgnomenu.pc
%{_libdir}/libgnomenu.so
%dir %{_includedir}/libgnomenu

%files -n gnome-applet-globalmenu
%defattr(-,root,root,-)
%{_libdir}/bonobo/servers/GlobalMenu_PanelApplet.server
%{_libexecdir}/GlobalMenu.PanelApplet

%if 0%{?rhel5}
%else
%files -n xfce4-globalmenu-plugin
%defattr(-,root,root,-)
%{_datadir}/xfce4/panel-plugins/GlobalMenu_XFCEPlugin.desktop
%{_libexecdir}/xfce4/panel-plugins/GlobalMenu.XFCEPlugin
%{_datadir}/pixmaps/globalmenu-xfce.png
%endif

%changelog
* Sun Mar 8 2009 Feng Yu <rainwoodman@gmail.com> - 0.7.4-4
- Remove the -common package, merge it to the main package.

* Sun Mar 8 2009 Feng Yu <rainwoodman@gmail.com> - 0.7.4-3
- Rename the applet/plugin subpackages according to https://bugzilla.redhat.com/show_bug.cgi?id=480279#c14

* Sun Mar 8 2009 Feng Yu <rainwoodman@gmail.com> - 0.7.4-2
- Changes according to https://bugzilla.redhat.com/show_bug.cgi?id=480279#c14
- Use *.h and _% dir in devel package.
- Requires pkgconfig in devel package.
- Added versions to sub-package requirements.
- Removed gnome-menus and gtk2 from requires
- Moved GConf2 requires to -common.
- Added ldconfig requires to -common.
- Merge gtkmodule to -common.
- Moved libgnomenu.so to -devel
- Added doc tag in common

* Sun Mar 8 2009 Feng Yu <rainwoodman@gmail.com>
- Update to version 0.7.4.
- Generate the versioned .spec file from .spec.in with configure

* Thu Feb 14 2009 Feng Yu <rainwoodman@gmail.com>
- Valentine's day. 
- Update to version (post) 0.7.3.
- Generate the versioned .spec file from .spec.in with configure
- Build on rhel5.

* Thu Jan 8 2009 Feng Yu <rainwoodman@gmail.com>
- Add XFCE4 plugin

* Thu Jan 8 2009 Feng Yu <rainwoodman@gmail.com>
- Spawn into sub packages.

* Wed Dec 17 2008 Feng Yu <rainwoodman@gmail.com>
- The module is ready for loaded/unloaded by GtkSettings. Because an issue with scim-bridge, the gconf key keeps disabling the module by default.

* Tue Dec 15 2008 Feng Yu <rainwoodman@gmail.com>
- gnome-settings-daemon for the module. (gconf-key: apps/gnome-settings-daemon/gtk-modules)

* Tue Dec 15 2008 Feng Yu <rainwoodman@gmail.com>
- vala 0.5.2

* Sun Dec 14 2008 Feng Yu <rainwoodman@gmail.com>
- GConf for applet 

* Sun Dec 10 2008 Feng Yu <rainwoodman@gmail.com>
- RGBA support for popup menus

* Sun Dec 5 2008 Feng Yu <rainwoodman@gmail.com>
- update to 0.7
- use the default vim template for spec files.

* Sun Oct 7 2008 Feng Yu <rainwoodman@gmail.com>
- installing libglobalmenu-gnome to gtk-2.0/modules

* Sun Oct 7 2008 Feng Yu <rainwoodman@gmail.com>
- Update to svn 1351
- Divide into sub packages

* Sun Oct 5 2008 Feng Yu <rainwoodman@gmail.com>
- Update to 0.6

* Sun Mar 23 2008 Feng Yu <rainwoodman@gmail.com>
- change to macros for building x86-64 rpms

* Fri Mar 14 2008 Feng Yu <rainwoodman@gmail.com>
- remove language pack
- move shared files to gnome-globalmenu

* Wed Mar 12 2008 Feng Yu <rainwoodman@gmail.com>
- Separated .gmo files
- Added description for packages.

* Sun Mar 9 2008 Feng Yu <rainwoodman@gmail.com>
- Properly install doc.
- Added French locale 

* Fri Mar 7 2008 Feng Yu <rainwoodman@gmail.com>
- Install doc
- Added depencency
- Added the mo file.
- distribute gtk2-aqd patch

* Fri Mar 5 2008 Feng Yu <rainwoodman@gmail.com>
- Enable schemas.

* Fri Feb 29 2008 Feng Yu <rainwoodman@gmail.com>
- Split into many sub packages.

* Mon Jan 14 2008 Feng Yu <rainwoodman@gmail.com>
- Added schema

* Wed Jan 09 2008 Feng Yu <rainwoodman@gmail.com>
- Added description

# vim:ts=4:sw=4


