SUBDIRS =

INCLUDES =								\
	-I$(top_srcdir)							\
	-I$(srcdir)							\
	$(EMERILLION_CFLAGS)						\
	$(WARN_CFLAGS)							\
	$(DISABLE_DEPRECATED_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\"					\
	-DEMERILLION_DATADIR=\""$(datadir)/emerillion"\"		\
	-DEMERILLION_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	-DEMERILLION_UIDIR=\""$(datadir)/emerillion/ui/"\"

bin_PROGRAMS = emerillion

emerillion_LDADD = $(EMERILLION_LIBS)

BUILT_SOURCES =

emerillion_SOURCES =	\
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
