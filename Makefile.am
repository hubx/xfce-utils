@SET_MAKE@

EXTRA_DIST = xfce-utils.spec xfce-utils.spec TODO

SUBDIRS = doc xfrun xftaskbar scripts gdm xfce4-about po

distclean-local:
	rm -rf xfce-utils.spec *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

html: Makefile
	make -C doc html

ACLOCAL_AMFLAGS = -I m4
