xftaskbar_plugindir = @XFCE_MCS_MANAGER_PLUGINSDIR@

xftaskbar_plugin_LTLIBRARIES = xftaskbar_plugin.la

xftaskbar_plugin_la_LDFLAGS =						\
	-avoid-version							\
	-module

xftaskbar_plugin_la_SOURCES =						\
	xftaskbar_plugin.c

xftaskbar_plugin_la_CFLAGS = 						\
	@XFCE_MCS_MANAGER_CFLAGS@ 					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DSYSCONFDIR=\"$(sysconfdir)\"					\
	-DDATADIR=\"$(pkgdatadir)\" 

xftaskbar_plugin_la_LIBADD = 						\
	@XFCE_MCS_MANAGER_LIBS@

desktopdir = $(datadir)/applications
desktop_DATA = xfce-taskbar-settings.desktop

iconsdir = $(datadir)/icons/hicolor/48x48/apps
icons_DATA = xfce4-taskbar.png

EXTRA_DIST = 								\
	$(desktop_DATA)							\
	$(icons_DATA)
