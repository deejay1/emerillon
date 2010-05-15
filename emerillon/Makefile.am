SUBDIRS =

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)								\
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

emerillon_CFLAGS = $(EOG_CFLAGS)
emerillon_LDFLAGS = -export-dynamic

emerillon_LDADD = libemerillon.la $(EMERILLON_LIBS)

CLEANFILES = $(BUILT_SOURCES)

headerdir = $(prefix)/include/emerillon-@EMERILLON_API_VERSION@/emerillon
header_DATA = $(API_H_FILES)

API_H_FILES =			\
	emerillon.h			\
	window.h			\
	sidebar.h

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
