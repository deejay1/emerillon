uidir = $(datadir)/emerillon
UI_FILES = emerillon-ui.xml
ui_DATA = $(UI_FILES)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = emerillon.pc

gsettings_files = \
	org.gnome.emerillon.gschema.xml.in \
	$(NULL)
gsettings_SCHEMAS = $(gsettings_files:.xml.in=.xml)
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@


convertdir=$(datadir)/GConf/gsettings
convert_DATA=emerillon.convert

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES= emerillon.desktop.in.in
DESKTOP_FILES= $(DESKTOP_IN_FILES:.desktop.in.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

EXTRA_DIST =			\
        $(DESKTOP_IN_FILES)	\
        $(gsettings_files) 	\
        $(UI_FILES)         	\
        $(convert_DATA)

DISTCLEANFILES = 		\
        $(gsettings_SCHEMAS)		\
        $(DESKTOP_FILES)
