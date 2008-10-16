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

emerillion_SOURCES =		\
	emerillion.c		\
	$(BUILT_SOURCES)

uidir = $(datadir)/emerillion-2/ui/
ui_DATA =

CLEANFILES = $(BUILT_SOURCES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
