SUBDIRS = cut-paste emerillon data plugins po bindings help docs

EXTRA_DIST = 			\
	autogen.sh		\
	ChangeLog		\
	README			\
	AUTHORS

DISTCLEANFILES = 		\
	ChangeLog

DISTCHECK_CONFIGURE_FLAGS =		\
	--disable-scrollkeeper		\
	--disable-schemas-install	\
	--enable-gtk-doc \
	--enable-introspection

.PHONY: ChangeLog
ChangeLog: Makefile.am
	git log --pretty=short > ChangeLog

dist: ChangeLog

