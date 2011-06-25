AC_PREREQ(2.63)

m4_define(emerillon_major_version, 0)
m4_define(emerillon_minor_version, 1)
m4_define(emerillon_micro_version, 90)
m4_define(emerillon_version, emerillon_major_version.emerillon_minor_version.emerillon_micro_version)

m4_define([emerillon_api_version],
          [emerillon_major_version.emerillon_minor_version])
AC_INIT([emerillon], [emerillon_version], [http://bugzilla.gnome.org/enter_bug.cgi?product=emerillon], [emerillon], [http://projects.gnome.org/emerillon])
AC_CONFIG_SRCDIR(emerillon/main.c)

AC_DEFINE(EMERILLON_MAJOR_VERSION, emerillon_major_version, [Emerillon major version])
AC_SUBST(EMERILLON_MAJOR_VERSION, emerillon_major_version)
AC_DEFINE(EMERILLON_MINOR_VERSION, emerillon_minor_version, [Emerillon minor version])
AC_SUBST(EMERILLON_MINOR_VERSION, emerillon_minor_version)
AC_DEFINE(EMERILLON_MICRO_VERSION, emerillon_micro_version, [Emerillon micro version])
AC_SUBST(EMERILLON_MICRO_VERSION, emerillon_micro_version)

EMERILLON_API_VERSION=emerillon_api_version
AC_SUBST(EMERILLON_API_VERSION)

AM_INIT_AUTOMAKE([1.11 dist-bzip2 no-dist-gzip -Wno-portability])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

GOBJECT_INTROSPECTION_CHECK([0.6.7])

GNOME_MAINTAINER_MODE_DEFINES
GNOME_COMPILE_WARNINGS([maximum])
GNOME_DEBUG_CHECK

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

###############
# GLib tools. #
###############
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)

#############
# GSettings #
#############
GLIB_GSETTINGS

################
# Translations #
################
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.40.0])

GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

###################
# gnome-doc-utils #
###################

PKG_PROG_PKG_CONFIG
GNOME_DOC_INIT

################
# Dependencies #
################
GLIB_REQUIRED=2.25.10
GTHREAD_REQUIRED=2.12.0
GTK_REQUIRED=3.0.0
GCONF_REQUIRED=2.31.1
CHAMPLAIN_REQUIRED=0.7.1
CHAMPLAIN_GTK_REQUIRED=0.7.1
GEOCLUE_REQUIRED=0.11.1
ETHOS_REQUIRED=0.2
ETHOS_UI_REQUIRED=0.2

PKG_CHECK_MODULES(EMERILLON, [
    glib-2.0 >= $GLIB_REQUIRED
    gthread-2.0 >= $GTHREAD_REQUIRED
    gtk+-3.0 >= $GTK_REQUIRED
    gconf-2.0 >= $GCONF_REQUIRED
    champlain-0.10 >= $CHAMPLAIN_REQUIRED
    champlain-gtk-0.10 >= $CHAMPLAIN_GTK_REQUIRED
    geoclue >= $GEOCLUE_REQUIRED
    ethos-1.0 >= $ETHOS_REQUIRED
    ethos-ui-1.0 >= $ETHOS_UI_REQUIRED
])

# -----------------------------------------------------------
# Search plugins dependencies
# -----------------------------------------------------------

AC_ARG_ENABLE(search,
  AC_HELP_STRING([--disable-search],[Do not compile the search plugin]),
    enable_search=$enableval, enable_search=yes )

if test x$enable_search = xyes; then
  AC_SUBST(SEARCH_DEPS_CFLAGS)
  AC_SUBST(SEARCH_DEPS_LIBS)
  PKG_CHECK_MODULES(SEARCH_DEPS,
    [
      rest-0.7 >= 0.6
    ]
  )

  AC_DEFINE(ENABLE_SEARCH, [], [Enable search plugin code])
fi

AM_CONDITIONAL(ENABLE_SEARCH, test "x$enable_search" = "xyes")
# -----------------------------------------------------------

#################
# Introspection #
#################
if test "x$found_introspection" = "xyes"
then
  AC_DEFINE(HAVE_INTROSPECTION, [1], [Enable introspection runtime options])
fi


# Workaround a bug in geoclue.
EMERILLON_CFLAGS=`echo $EMERILLON_CFLAGS | sed s,/geoclue,,`

AC_SUBST(EMERILLON_CFLAGS)
AC_SUBST(EMERILLON_LIBS)

#################
# Vala bindings #
#################
AC_ARG_ENABLE(vala,
  AC_HELP_STRING([--enable-vala],[install Vala bindings [default=yes]]),,
    enable_vala=yes )

if test "$enable_vala" = "yes"
then
  AM_PROG_VALAC([0.8.0])
  if test x${VALAC} = x""; then
    exit 0;
  fi
fi

AM_CONDITIONAL(ENABLE_VALA, test x${VALAC} != x"")

################
# Output files #
################
AC_CONFIG_FILES([
Makefile
emerillon/Makefile
cut-paste/Makefile
data/Makefile
data/emerillon.pc
data/emerillon.desktop.in
docs/Makefile
docs/reference/Makefile
docs/reference/version.xml
help/Makefile
plugins/Makefile
bindings/Makefile
bindings/vala/Makefile
po/Makefile.in
])

AC_OUTPUT

echo "
Emerillon ${VERSION}

Configuration:
       Prefix           : ${prefix}
       Compiler         : ${CC}
       Documentation    : ${enable_gtk_doc}
       Introspection    : ${enable_introspection}

Bindings:
       Vala             : ${enable_vala}
"
