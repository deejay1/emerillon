@INTLTOOL_SCHEMAS_RULE@

schemas_in_files = emerillon.schemas.in
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

uidir = $(datadir)/emerillon
UI_FILES = emerillon-ui.xml
ui_DATA = $(UI_FILES)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emerillon.pc

install-data-local:
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $(schemas_DATA) ; do \
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p >&1 > /dev/null; \
	done \
	fi
endif

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES= emerillon.desktop.in.in
DESKTOP_FILES= $(DESKTOP_IN_FILES:.desktop.in.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

EXTRA_DIST =			\
        $(DESKTOP_IN_FILES)	\
        $(schemas_in_files)	\
        $(UI_FILES)

DISTCLEANFILES = 		\
        $(schemas_DATA)		\
        $(DESKTOP_FILES)
