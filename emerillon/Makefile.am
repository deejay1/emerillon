SUBDIRS =

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(EMERILLON_CFLAGS)						\
	$(WARN_CFLAGS)						\
	$(DISABLE_DEPRECATED)					\
	-DEMERILLON_COMPILATION					\
	-DEMERILLON_DATADIR=\""$(datadir)/emerillon"\"			\
	-DEMERILLON_PLUGINDIR=\""$(libdir)/emerillon/plugins"\"		\
	-DEMERILLON_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DEMERILLON_UIDIR=\""$(datadir)/emerillon/ui/"\"

bin_PROGRAMS = emerillon

noinst_LTLIBRARIES = libemerillon.la



BUILT_SOURCES =

libemerillon_la_SOURCES =	\
	manager.h	\
	manager.c	\
	preferences.h	\
	preferences.c	\
	window.c	\
	window.h	\
	sidebar.c	\
	sidebar.h	\
	config-keys.h	\
	emerillon.h	\
	$(BUILT_SOURCES)

emerillon_SOURCES = main.c

emerillon_LDFLAGS = -export-dynamic

emerillon_LDADD = libemerillon.la $(EMERILLON_LIBS)

if HAVE_INTROSPECTION
EMERILLON_CFLAGS += $(INTROSPECTION_CFLAGS)
emerillon_LDADD += $(INTROSPECTION_LIBS)
endif

CLEANFILES = $(BUILT_SOURCES)

headerdir = $(prefix)/include/emerillon-@EMERILLON_API_VERSION@/emerillon
header_DATA = $(API_H_FILES)

API_H_FILES =			\
	emerillon.h			\
	window.h			\
	sidebar.h

if HAVE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Emerillon-0.1.gir

  introspection_sources = $(libemerillon_la_SOURCES)

  Emerillon-0.1.gir: $(libemerillon_la_SOURCES) $(bin_PROGRAMS)
  Emerillon_0_1_gir_INCLUDES = GObject-2.0 Gtk-2.0 Clutter-1.0 Champlain-0.4 GtkChamplain-0.4
  Emerillon_0_1_gir_CFLAGS =  -I$(top_srcdir) $(EMERILLON_CFLAGS) -DEMERILLON_COMPILATION
  Emerillon_0_1_gir_PROGRAM = ./$(bin_PROGRAMS)
  Emerillon_0_1_gir_FILES = $(top_srcdir)/emerillon/emerillon.h $(top_srcdir)/emerillon/window.h $(top_srcdir)/emerillon/sidebar.h

  girdir = $(datadir)/gir-1.0
  dist_gir_DATA = $(INTROSPECTION_GIRS)

  typelibdir = $(libdir)/girepository-1.0
  typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += \
	$(dist_gir_DATA)		\
	$(typelib_DATA)
endif

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
