@INTLTOOL_SCHEMAS_RULE@

schemas_in_files = emerillion.schemas.in
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

uidir = $(datadir)/emerillion
UI_FILES = emerillion-ui.xml
ui_DATA = $(UI_FILES)

install-data-local:
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
	for p in $(schemas_DATA) ; do \
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/data/$$p >&1 > /dev/null; \
	done \
	fi
endif

EXTRA_DIST =			\
        $(schemas_in_files)	\
	$(UI_FILES)

DISTCLEANFILES = 		\
        $(schemas_DATA)	
