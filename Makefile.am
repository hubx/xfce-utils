@SET_MAKE@

EXTRA_DIST = xfce-utils.spec xfce-utils.spec.in TODO

SUBDIRS = doc xfrun xftaskbar scripts gdm xfce4-about

distclean-local:
	rm -rf xfce-utils.spec *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz
