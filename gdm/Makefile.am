
gdm_prefix = @GDM_PREFIX@
switchdesk_prefix = "/usr/share/apps/switchdesk"
wmsession_prefix  = "$(gdm_prefix)/wmsession.d"
dmsession_prefix  = "$(gdm_prefix)/dm/Sessions"
gdmsession_prefix = "$(gdm_prefix)/gdm/Sessions"

noinst_DATA = $(xfce_FILES)
xfce_SOURCE = XFce4.in Xclients.xfce4.in 10XFce4.in
xfce_FILES = XFce4 Xclients.xfce4 10XFce4 xfce.desktop
EXTRA_DIST = $(xfce_SOURCE)
DISTCLEANFILES = $(xfce_FILES)

XFce4 : $(srcdir)/XFce4.in
	@sed -e "s%BIN_DIR%$(bindir)%g"     $(srcdir)/XFce4.in > $@

10XFce4 : $(srcdir)/10XFce4.in
	@sed -e "s%BIN_DIR%$(bindir)%g"     $(srcdir)/10XFce4.in > $@

Xclients.xfce4 : $(srcdir)/Xclients.xfce4.in
	@sed -e "s%BIN_DIR%$(bindir)%g"     $(srcdir)/Xclients.xfce4.in > $@

install-data-local :
	test no = @USE_GDM@ || $(MAKE) install-gdm

install-data-local :
	test no = @USE_GDM@ || $(MAKE) install-gdm

install-gdm: $(xfce_FILES)
	$(mkinstalldirs) $(DESTDIR)$(switchdesk_prefix)
	$(mkinstalldirs) $(DESTDIR)$(wmsession_prefix)
	$(mkinstalldirs) $(DESTDIR)$(dmsession_prefix)
	$(mkinstalldirs) $(DESTDIR)$(gdmsession_prefix)
	$(INSTALL_SCRIPT) XFce4 \
	        $(DESTDIR)$(gdmsession_prefix)/XFce4
	$(INSTALL_SCRIPT) Xclients.xfce4 \
	        $(DESTDIR)$(switchdesk_prefix)/Xclients.xfce4
	$(INSTALL_SCRIPT) 10XFce4 \
	        $(DESTDIR)$(wmsession_prefix)/10XFce4
	$(INSTALL_SCRIPT) xfce.desktop \
	        $(DESTDIR)$(dmsession_prefix)/xfce.desktop

uninstall-local: uninstall-gdm

uninstall-gdm:
	rm -f $(DESTDIR)$(gdmsession_prefix)/XFce4
	rm -f $(DESTDIR)$(dmsession_prefix)/xfce.desktop
	rm -f $(DESTDIR)$(switchdesk_prefix)/Xclients.xfce4
	rm -f $(DESTDIR)$(wmsession_prefix)/10XFce4

