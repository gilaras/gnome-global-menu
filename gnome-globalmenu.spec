%define base_version 0.4
%define svn_version svn500
Name: 		gnome-globalmenu
Version: 	%{base_version}.%{svn_version}
Release:	1%{?dist}
Summary:	Global menu bar widget and library for GTK/GNOME2

Group:		User Interface/Desktops
License:	GPLv2+
URL:		http://code.google.com/p/gnome2-globalmenu/

Source0:	http://gnome2-globalmenu.googlecode.com/files/gnome-globalmenu-%{base_version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{base_version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:	gtk2
BuildRequires:	libwnck
BuildRequires:	gnome-panel-devel
BuildRequires:	bonobo-activation
BuildRequires:	xfce4-panel-devel
%description


%package -n libgnomenu
Summary: libgnomenu Provides global menu widget GtkGlobalMenuBar 
Group:		User Interface/Desktops
%description -n libgnomenu

%package -n gnome-globalmenu-applet
Summary: gnome-panel applet for global menu.
Group:		User Interface/Desktops
%description -n gnome-globalmenu-applet

%package -n xfce-globalmenu-plugin
Summary: xfce-panel plugin for global menu.
Group:		User Interface/Desktops
%description -n xfce-globalmenu-plugin

%package -n gnomenu-server
Summary: standalone mene server for global menu.
Group:		User Interface/Desktops
%description -n gnomenu-server

%prep
%setup -q -n gnome-globalmenu-%{base_version}

%build
%configure
make

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%clean
rm -rf %{buildroot}

%pre
#export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
#gconftool-2 --makefile-uninstall-rule %{_sysconfdir}/gconf/schemas/gnome2-globalmenu-applet.schema  >& /dev/null || :

%post
#export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
#gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/gnome2-globalmenu-applet.schema >& /dev/null || :

%preun
#export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
#gconftool-2 --makefile-uninstall-rule %{_sysconfdir}/gconf/schemas/gnome2-globalmenu-applet.schema >& /dev/null || :


%files
%defattr(-, root, root)
/etc/libgnomenu.conf
/usr/include/libgnomenu/clienthelper.h
/usr/include/libgnomenu/gdksocket.h
/usr/include/libgnomenu/gtkglobalmenubar.h
/usr/include/libgnomenu/messages.h
/usr/include/libgnomenu/quirks.h
/usr/include/libgnomenu/serverhelper.h
/usr/lib/libgnomenu.a
/usr/lib/libgnomenu.la
/usr/lib/libgnomenu.so
/usr/lib/libgnomenu.so.0
/usr/lib/libgnomenu.so.0.0.0
/usr/lib/pkgconfig/libgnomenu.pc
/usr/share/doc/gnome-globalmenu/AUTHORS
/usr/share/doc/gnome-globalmenu/COPYING
/usr/share/doc/gnome-globalmenu/ChangeLog
/usr/share/doc/gnome-globalmenu/INSTALL
/usr/share/doc/gnome-globalmenu/NEWS
/usr/share/doc/gnome-globalmenu/README
/usr/share/xfce4/panel-plugins/xfce-globalmenu-plugin.desktop
/usr/lib/bonobo/servers/GNOME_GlobalMenuApplet.server

%files -n gnomenu-server
/usr/libexec/globalmenu-server

%files -n gnome-globalmenu-applet
/usr/libexec/gnome-globalmenu-applet

%files -n xfce-globalmenu-plugin
/usr/libexec/xfce-globalmenu-plugin

%changelog 
* Mon Jan 14 2008 Feng Yu <rainwoodman@gmail.com>
- Added schema
* Wed Jan 09 2008 Feng Yu <rainwoodman@gmail.com>
- Added description
