AC_PREREQ(2.59)

m4_define(emerillion_major_version, 0)
m4_define(emerillion_minor_version, 1)
m4_define(emerillion_micro_version, 0)
m4_define(emerillion_version, emerillion_major_version.emerillion_minor_version.emerillion_micro_version)

AC_INIT(emerillion, emerillion_version)
AC_CONFIG_SRCDIR(emerillion/main.c)

AC_DEFINE(EMERILLION_MAJOR_VERSION, emerillion_major_version, [Emerillion major version])
AC_SUBST(EMERILLION_MAJOR_VERSION, emerillion_major_version)
AC_DEFINE(EMERILLION_MINOR_VERSION, emerillion_minor_version, [Emerillion minor version])
AC_SUBST(EMERILLION_MINOR_VERSION, emerillion_minor_version)
AC_DEFINE(EMERILLION_MICRO_VERSION, emerillion_micro_version, [Emerillion micro version])
AC_SUBST(EMERILLION_MICRO_VERSION, emerillion_micro_version)

AM_INIT_AUTOMAKE([1.8 dist-bzip2 no-dist-gzip -Wno-portability])

AC_CONFIG_HEADERS(config.h)

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

################
# Dependencies #
################
GLIB_REQUIRED=2.12.0
GTK_REQUIRED=2.12.0
GCONF_REQUIRED=2.5.90
CHAMPLAIN_REQUIRED=0.2.6
CHAMPLAIN_GTK_REQUIRED=0.2.6
GEOCLUE_REQUIRED=0.11.1

PKG_CHECK_MODULES(EMERILLION, [
    glib-2.0 >= $GLIB_REQUIRED
    gtk+-2.0 >= $GTK_REQUIRED
    gconf-2.0 >= $GCONF_REQUIRED \
    champlain-0.2 >= $CHAMPLAIN_REQUIRED
    champlain-gtk-0.2 >= $CHAMPLAIN_GTK_REQUIRED
    geoclue >= $GEOCLUE_REQUIRED
])
AC_SUBST(EMERILLION_CFLAGS)
AC_SUBST(EMERILLION_LIBS)

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

################
# Output files #
################
AC_CONFIG_FILES([
Makefile
emerillion/Makefile
data/Makefile
po/Makefile.in
])

AC_OUTPUT

echo "

Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC}
"
