scripts= \
	xfterm4 \
	xflock4 \
	xfmountdev4

bin_SCRIPTS= \
	$(scripts) \
	xfterm4 \
	xfbrowser4 \
	startxfce4 \
	xfhelp4

# cannot be set from configure, because ${prefix} is not expanded
edit = sed \
	-e 's,@datadir\@,$(datadir),g' \
	-e 's,@sysconfdir\@,$(sysconfdir),g' \
	-e 's,@BROWSER_DEFAULT\@,@BROWSER_DEFAULT@,g' \
	-e 's,@TERMINAL_DEFAULT\@,@TERMINAL_DEFAULT@,g'

xfbrowser4: Makefile $(srcdir)/xfbrowser4.in
	rm -f xfbrowser4 xfbrowser4.tmp
	$(edit) $(srcdir)/xfbrowser4.in >xfbrowser4.tmp
	mv xfbrowser4.tmp xfbrowser4

startxfce4: Makefile $(srcdir)/startxfce4.in
	rm -f startxfce4 startxfce4.tmp
	$(edit) $(srcdir)/startxfce4.in >startxfce4.tmp
	mv startxfce4.tmp startxfce4

xfhelp4: Makefile $(srcdir)/xfhelp4.in
	rm -f xfhelp4 xfhelp4.tmp
	$(edit) $(srcdir)/xfhelp4.in >xfhelp4.tmp
	mv xfhelp4.tmp xfhelp4

xinitrc: Makefile $(srcdir)/xinitrc.in
	rm -f xinitrc xinitrc.tmp
	$(edit) $(srcdir)/xinitrc.in >xinitrc.tmp
	mv xinitrc.tmp xinitrc

conffilesdir=@sysconfdir@/xdg/xfce4
conffiles_DATA=\
	xinitrc \
	Xft.xrdb

EXTRA_DIST = \
	$(scripts) \
	$(conffiles_DATA) \
	xfbrowser4.in \
	startxfce4.in \
	xfhelp4.in \
	xinitrc.in

distclean-local:
	rm -f xfhelp4 startxfce4 xfbrowser4 xinitrc

