
INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-about\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = \
	xfce4-about

xfce4_about_built_sources = \
	about-dialog-ui.h

xfce4_about_SOURCES = \
	$(xfce4_about_built_sources) \
	contributors.h \
	translators.h \
	main.c

xfce4_about_CFLAGS = \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(PLATFORM_CFLAGS)

xfce4_about_LDADD = \
	$(LIBXFCE4UTIL_LIBS) \
	$(GTK_LIBS) \
	$(LIBXFCE4UI_LIBS)

desktopdir = $(datadir)/applications
desktop_in_files = xfce4-about.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

if MAINTAINER_MODE

about-dialog-ui.h: $(srcdir)/xfce4-about-dialog.glade Makefile
	$(AM_V_GEN) exo-csource --static --strip-comments --strip-content --name=xfce_about_dialog_ui $< >$@

DISTCLEANFILES = \
	$(xfce4_about_built_sources)

BUILT_SOURCES = \
	$(xfce4_about_built_sources)

endif

EXTRA_DIST = 								\
	$(desktop_in_files)						\
	xfce4-about-dialog.glade

DISTCLEANFILES = 							\
	$(desktop_DATA)

