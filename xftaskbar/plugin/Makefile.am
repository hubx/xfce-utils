xftaskbar_plugindir = $(libdir)/xfce-mcs-manager/plugins

xftaskbar_plugin_LTLIBRARIES = xftaskbar_plugin.la

xftaskbar_plugin_la_LDFLAGS =						\
	-avoid-version							\
	-export-dynamic							\
	-module

xftaskbar_plugin_la_SOURCES =						\
	xftaskbar_plugin.c						\
	inline-icon.h

xftaskbar_plugin_la_CFLAGS = 						\
	@XFCE_MCS_MANAGER_CFLAGS@ 					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DSYSCONFDIR=\"$(sysconfdir)\"					\
	-DDATADIR=\"$(pkgdatadir)\" 

xftaskbar_plugin_la_LIBADD = 						\
	@XFCE_MCS_MANAGER_LIBS@

IMAGES=icon.png
VARIABLES=default_icon_data $(srcdir)/$(IMAGES)

EXTRA_DIST = inline-icon.h icon.png

noinst_DATA = inline-icon.h icon.png

inline-icon.h: $(IMAGES)
	gdk-pixbuf-csource --raw --build-list $(VARIABLES) > $(srcdir)/inline-icon.h

