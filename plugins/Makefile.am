AUTOMAKE_OPTIONS = subdir-objects

pluginsdir = $(libdir)/emerillon/plugins

INCLUDES = \
	-I$(top_srcdir) 				\
	$(EMERILLON_CFLAGS) 				\
	$(SEARCH_DEPS_CFLAGS) 				\
	$(WARN_CFLAGS)					\
	-DEMERILLON_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DEMERILLON_UIDIR=\""$(datadir)/emerillon/ui/"\" \
	-DEMERILLON_DATADIR=\""$(datadir)/emerillon"\"

AM_LDFLAGS = -avoid-version -module

plugins_LTLIBRARIES = \
	placemarks/libplacemarks.la

plugins_in_files = \
	placemarks/placemarks.emerillon-plugin.in

# Placemarks plugin
placemarks_libplacemarks_la_SOURCES = \
	placemarks/placemarks.h  \
	placemarks/placemarks.c

placemarks_libplacemarks_la_LIBADD = $(EMERILLON_LIBS)

# Search plugin
if ENABLE_SEARCH
plugins_LTLIBRARIES += \
	search/libsearch.la

plugins_in_files += \
	search/search.emerillon-plugin.in

search_libsearch_la_SOURCES = \
	search/search.h  \
	search/search.c

search_libsearch_la_LIBADD = \
	$(EMERILLON_LIBS) \
	$(SEARCH_DEPS_LIBS)
endif

# Generate plugin info files

%.emerillon-plugin: %.emerillon-plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(AM_V_GEN)$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugins_DATA = $(plugins_in_files:.emerillon-plugin.in=.emerillon-plugin)

DISTCLEANFILES = $(plugins_DATA)
CLEANFILES = $(plugins_DATA)

EXTRA_DIST = $(plugins_in_files)

-include $(top_srcdir)/git.mk