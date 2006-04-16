# $Id$

EXTRA_DIST = xfce-utils.spec xfce-utils.spec TODO

SUBDIRS = doc icons xfrun scripts gdm xfce4-about po

AUTOMAKE_OPTIONS =							\
	1.8								\
	dist-bzip2

distclean-local:
	rm -rf xfce-utils.spec *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

html: Makefile
	make -C doc html

# vi:set ts=8 sw=8 noet ai nocindent:
