AC_PREREQ(2.59)

m4_define(emerillon_major_version, 0)
m4_define(emerillon_minor_version, 1)
m4_define(emerillon_micro_version, 2)
m4_define(emerillon_version, emerillon_major_version.emerillon_minor_version.emerillon_micro_version)

m4_define([emerillon_api_version],
          [emerillon_major_version.emerillon_minor_version])
AC_INIT(emerillon, emerillon_version)
AC_CONFIG_SRCDIR(emerillon/main.c)

AC_DEFINE(EMERILLON_MAJOR_VERSION, emerillon_major_version, [Emerillon major version])
AC_SUBST(EMERILLON_MAJOR_VERSION, emerillon_major_version)
AC_DEFINE(EMERILLON_MINOR_VERSION, emerillon_minor_version, [Emerillon minor version])
AC_SUBST(EMERILLON_MINOR_VERSION, emerillon_minor_version)
AC_DEFINE(EMERILLON_MICRO_VERSION, emerillon_micro_version, [Emerillon micro version])
AC_SUBST(EMERILLON_MICRO_VERSION, emerillon_micro_version)

EMERILLON_API_VERSION=emerillon_api_version
AC_SUBST(EMERILLON_API_VERSION)

AM_INIT_AUTOMAKE([1.8 dist-bzip2 no-dist-gzip -Wno-portability])

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR(m4)

AM_MAINTAINER_MODE

GNOME_MAINTAINER_MODE_DEFINES
GNOME_COMPILE_WARNINGS
GNOME_DEBUG_CHECK

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])

###############
# GLib tools. #
###############
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)
AC_PATH_PROG(GLIB_MKENUMS, glib-mkenums)

##########
# GConf. #
##########
AM_GCONF_SOURCE_2

AC_PATH_PROG([GCONFTOOL], [gconftool-2], [no])

if test "$GCONFTOOL" = "no"; then
    AC_MSG_ERROR([gconftool-2 not found])
fi

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
GLIB_REQUIRED=2.12.0
GTHREAD_REQUIRED=2.12.0
GTK_REQUIRED=2.20.0
GCONF_REQUIRED=2.5.90
CHAMPLAIN_REQUIRED=0.4
CHAMPLAIN_GTK_REQUIRED=0.4
GEOCLUE_REQUIRED=0.11.1
ETHOS_REQUIRED=0.2
ETHOS_UI_REQUIRED=0.2

PKG_CHECK_MODULES(EMERILLON, [
    glib-2.0 >= $GLIB_REQUIRED
    gthread-2.0 >= $GTHREAD_REQUIRED
    gtk+-2.0 >= $GTK_REQUIRED
    gconf-2.0 >= $GCONF_REQUIRED
    champlain-0.4 >= $CHAMPLAIN_REQUIRED
    champlain-gtk-0.4 >= $CHAMPLAIN_GTK_REQUIRED
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
      rest-0.6 >= 0.6
    ]
  )

  AC_DEFINE(ENABLE_SEARCH, [], [Enable search plugin code])
fi

AM_CONDITIONAL(ENABLE_SEARCH, test "x$enable_search" = "xyes")
# -----------------------------------------------------------

# Workaround a bug in geoclue.
EMERILLON_CFLAGS=`echo $EMERILLON_CFLAGS | sed s,/geoclue,,`

AC_SUBST(EMERILLON_CFLAGS)
AC_SUBST(EMERILLON_LIBS)

###############
# Depecations #
###############
AC_ARG_ENABLE(deprecations,
              [AC_HELP_STRING([--enable-deprecations],
                              [warn about deprecated usages [default=yes]])],,
              [enable_deprecations=yes])

if test "x$enable_deprecations" = "xyes"; then
   DISABLE_DEPRECATED_CFLAGS="\
-DG_DISABLE_DEPRECATED \
-DGDK_DISABLE_DEPRECATED \
-DGTK_DISABLE_DEPRECATED \
-DGDK_PIXBUF_DISABLE_DEPRECATED \
-DGNOME_DISABLE_DEPRECATED"
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
fi

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

Bindings:
       Vala             : ${enable_vala}
"
