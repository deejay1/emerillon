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

CLEANFILES = $(BUILT_SOURCES)

API_H_FILES =           \
	emerillon.h     \
	window.h        \
	sidebar.h

headerdir = $(prefix)/include/emerillon-@EMERILLON_API_VERSION@/emerillon
header_DATA = $(API_H_FILES)

# introspection support
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(filter %.c,$(libemerillon_la_SOURCES)) $(API_H_FILES)

Emerillon-0.1.gir: emerillon
Emerillon_0_1_gir_NAMESPACE = Emerillon
Emerillon_0_1_gir_VERSION = 0.1
Emerillon_0_1_gir_PROGRAM = emerillon
Emerillon_0_1_gir_FILES = $(introspection_sources)
Emerillon_0_1_gir_INCLUDES = GObject-2.0 Gtk-3.0 Champlain-0.10
Emerillon_0_1_gir_CFLAGS = -DEMERILLON_COMPILATION -I$(top_srcdir)
INTROSPECTION_GIRS += Emerillon-0.1.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)
endif

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
