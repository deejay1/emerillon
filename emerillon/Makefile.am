SUBDIRS =

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(EMERILLON_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DEMERILLON_DATADIR=\""$(datadir)/emerillon"\"		\
	-DEMERILLON_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DEMERILLON_UIDIR=\""$(datadir)/emerillon/ui/"\"

bin_PROGRAMS = emerillon

emerillon_LDADD = \
	$(EMERILLON_LIBS) \
	$(top_builddir)/cut-paste/libephyspinner.la

BUILT_SOURCES =

emerillon_SOURCES =	\
	main.c		\
	address.c	\
	address.h	\
	window.c	\
	window.h	\
	sidebar.c	\
	sidebar.h	\
	$(BUILT_SOURCES)

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
