NULL =
SUBDIRS = tools vapi tests libcompat libgnomenu applet xfce gtk-modules po
DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install --disable-static

AM_CPPFLAGS = \
		$(NULL)

man_MANS = gnome-globalmenu.1

EXTRA_DIST = \
	$(man_MANS) \
	$(doc_DATA) \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	gnome-globalmenu.schemas.in \
	patch.sh \
	$(NULL)

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	$(schema_DATA) \
	$(NULL)

BUILT_SOURCES =

CLEANFILES = 

include $(top_srcdir)/Makefile.configmake
include $(top_srcdir)/Makefile.config

schemadir   = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files = gnome-globalmenu.schemas.in
schema_DATA = $(schema_in_files:.in=)
doc_DATA = README COPYING ChangeLog AUTHORS INSTALL README.XFCE README.GNOME
@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/$(schema_DATA)
else
install-data-local:
endif

rpm: 
	vim gnome-globalmenu.spec
	touch `find . -name '*.vala'`
	make dist
	cp $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 ~/rpmbuild/SOURCES
	rpmbuild -bb gnome-globalmenu.spec

srpm-fast:
	make dist
	cp $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 ~/rpmbuild/SOURCES
	rpmbuild -bs gnome-globalmenu.spec
rpm-fast:
	make dist
	cp $(PACKAGE_TARNAME)-$(PACKAGE_VERSION).tar.bz2 ~/rpmbuild/SOURCES
	rpmbuild -bb gnome-globalmenu.spec
