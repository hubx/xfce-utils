@SET_MAKE@

EXTRA_DIST = xfce-utils.spec xfce-utils.spec.in TODO

SUBDIRS = xfrun xftaskbar scripts gdm

distclean-local:
	rm -f xfce-utils.spec

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
