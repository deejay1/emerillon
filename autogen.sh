#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

PKG_NAME="emerillon"

(test -f $srcdir/emerillon/main.c) || {
    echo -n "**Error**: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

which gnome-autogen.sh || {
    echo "You need to install gnome-common from the GNOME GIT"
    exit 1
}

which gtkdocize || {
    echo "You are missing gtk-doc. Please install it"
    exit 1
}

REQUIRED_AUTOCONF_VERSION=2.63
REQUIRED_AUTOMAKE_VERSION=1.11
REQUIRED_INTLTOOL_VERSION=0.40.0
REQUIRED_PKG_CONFIG_VERSION=0.16.0
REQUIRED_GTK_DOC_VERSION=1.14
USE_GNOME2_MACROS=1 USE_COMMON_DOC_BUILD=yes . gnome-autogen.sh
