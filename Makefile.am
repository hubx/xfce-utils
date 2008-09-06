

EXTRA_DIST = xfce-utils.spec xfce-utils.spec TODO

SUBDIRS = doc icons xfrun scripts xfce4-about po

AUTOMAKE_OPTIONS =							\
	1.8								\
	dist-bzip2

desktopdir = $(XSESSION_PREFIX)/share/xsessions
desktop_in_files = xfce.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(desktop_in_files) \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	AUTHORS \
	COPYING \
	ChangeLog \
	INSTALL \
	NEWS \
	README


DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	xfce.desktop

DISTCHECK_CONFIGURE_FLAGS = 						\
	--with-xsession-prefix=/tmp/$(PACKAGE)-distcheck

# vi:set ts=8 sw=8 noet ai nocindent:
