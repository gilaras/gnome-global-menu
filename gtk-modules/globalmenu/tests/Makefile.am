include $(top_srcdir)/Makefile.decl

VALASOURCES = test-serializer.vala test-unload.vala

if ENABLE_TESTS
noinst_PROGRAMS =  $(VALASOURCES:.vala=)
TEST_PROGS += $(noinst_PROGRAMS)
endif 

STAMP = $(VALASOURCES:.vala=.c)
SUFFIXES = .vala .c .glade .ui
.vala.c:
	$(VALAC) $(VALA_DEFAULT_PKGS) -C --vapidir=$(top_srcdir)/tests --vapidir=../ --pkg testman --pkg gtk+-2.0 --pkg globalmenu-gnome-private --pkg gmodule-2.0 $^
	touch $@

.glade.ui:
	gtk-builder-convert $< $@

test_unload_LDADD = $(GTK_LIBS) $(GLIB_LIBS) $(top_builddir)/tests/libtestman.la

AM_CPPFLAGS = -I$(srcdir)/.. $(GTK_CFLAGS) $(GLIB_CFLAGS) -include $(CONFIG_HEADER) -I$(top_srcdir)/tests
BUILT_SOURCES = $(STAMP)
LDADD = $(GTK_LIBS) $(GLIB_LIBS) ../libglobalmenu-gnome.la $(top_builddir)/tests/libtestman.la
EXTRA_DIST += $(STAMP) $(VALASOURCES) $(VALASOURCES:.vala=.h) test-unload.ui test-unload.glade
