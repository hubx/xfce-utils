@SET_MAKE@

EXTRA_DIST = xfce-utils.spec xfce-utils.spec.in TODO

SUBDIRS = backdrops icons xfbd xfmouse xfrun

distclean-local:
	rm -f xfce-utils.spec

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz
