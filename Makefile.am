
SUBDIRS = doc \
	icons \
	xfrun \
	scripts \
	xfce4-about \
	po

.PHONY: ChangeLog

ChangeLog: Makefile
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp \
	&& mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) \
	|| (touch ChangeLog; echo 'Git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

AUTOMAKE_OPTIONS =							\
	1.8								\
	dist-bzip2

desktopdir = $(XSESSION_PREFIX)/share/xsessions
desktop_in_files = xfce.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	xfce-utils.spec \
	TODO \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	$(desktop_in_files)

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	$(desktop_DATA)

DISTCHECK_CONFIGURE_FLAGS = \
	--with-xsession-prefix=/tmp/$(PACKAGE)-distcheck \
	--enable-gen-doc

# vi:set ts=8 sw=8 noet ai nocindent:
