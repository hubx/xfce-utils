
gdm_prefix = @GDM_PREFIX@
switchdesk_prefix = "/usr/share/apps/switchdesk"
wmsession_prefix = "/etc/X11/wmsession.d"

noinst_DATA = $(xfce_FILES)
xfce_SOURCE = XFce4.in Xclients.xfce4.in 10XFce4.in
xfce_FILES = XFce4 Xclients.xfce4 10XFce4
EXTRA_DIST = $(xfce_SOURCE)
DISTCLEANFILES = $(xfce_FILES)

install-data-local :
	test no = @USE_GDM@ || $(MAKE) install-gdm

install-gdm: $(xfce_FILES)
	$(mkinstalldirs) $(DESTDIR)$(gdm_prefix)
	$(mkinstalldirs) $(DESTDIR)$(switchdesk_prefix)
	$(mkinstalldirs) $(DESTDIR)$(wmsession_prefix)
	$(INSTALL_SCRIPT) XFce4 \
	 $(DESTDIR)$(gdm_prefix)/XFce4
	$(INSTALL_SCRIPT) Xclients.xfce4 \
	 $(DESTDIR)$(switchdesk_prefix)/Xclients.xfce4
	$(INSTALL_SCRIPT) 10XFce4 \
	 $(DESTDIR)$(wmsession_prefix)/10XFce4

uninstall-local: uninstall-gdm

uninstall-gdm:
	rm -f $(DESTDIR)$(gdm_prefix)/XFce4
	rm -f $(DESTDIR)$(switchdesk_prefix)/Xclients.xfce4
	rm -f $(DESTDIR)$(wmsession_prefix)/10XFce4

