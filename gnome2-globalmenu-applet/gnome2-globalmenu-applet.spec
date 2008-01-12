Name: 		gnome2-globalmenu-applet
Version: 	0.3
Release:	3%{?dist}
Summary:	Global menubar applet for Gnome2

Group:		User Interface/Desktops
License:	GPLv2+
URL:		http://code.google.com/p/gnome2-globalmenu/

Source0:	http://gnome2-globalmenu.googlecode.com/files/gnome2-globalmenu-applet-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires:	gtk2-aqd
Requires:	libwnck

%description
A panel applet, if the user has installed gtk2-aqd, the menubar of current activated window will be shown in the applet instead of the window; the menubar of unactivated window will disappear.

%prep
%setup

%build
%configure
make
#cat GNOME_GlobalMenuApplet.server.sample | sed "s;APP_LOCATION;"%{_libexecdir}";" > GNOME_GlobalMenuApplet.server

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install
#install -d %{buildroot}%{_libdir}/bonobo/servers
#install -p GNOME_GlobalMenuApplet.server %{buildroot}%{_libdir}/bonobo/servers

%clean
rm -rf %{buildroot}

%files
%defattr(-, root, root)

%doc %{_docdir}/%{name}/README 
%doc %{_docdir}/%{name}/AUTHORS
%doc %{_docdir}/%{name}/COPYING 
%doc %{_docdir}/%{name}/ChangeLog
%doc %{_docdir}/%{name}/INSTALL 
%doc %{_docdir}/%{name}/NEWS
%doc %{_docdir}/%{name}/GNOME_GlobalMenuApplet.server.sample
#%doc README AUTHORS COPYING ChangeLog INSTALL NEWS
#%doc GNOME_GlobalMenuApplet.server.sample
%{_libexecdir}/gnome2-globalmenu-applet
%{_libdir}/bonobo/servers/GNOME_GlobalMenuApplet.server

%changelog 
* Wed Jan 09 2008 Feng Yu <rainwoodman@gmail.com>
- Added description
