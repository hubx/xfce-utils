aboutdir = $(datadir)/xfce4

about_DATA =								\
	AUTHORS								\
	AUTHORS.az							\
	AUTHORS.ca							\
	AUTHORS.de							\
	AUTHORS.es							\
	AUTHORS.fr							\
	AUTHORS.html							\
	AUTHORS.html.az							\
	AUTHORS.html.ca							\
	AUTHORS.html.de							\
	AUTHORS.html.es							\
	AUTHORS.html.fr							\
	AUTHORS.html.it							\
	AUTHORS.it							\
	AUTHORS.lt							\
	AUTHORS.vi							\
	BSD								\
	BSD.html							\
	COPYING								\
	COPYING.html							\
	COPYING.vi							\
	GPL								\
	GPL.html							\
	INFO								\
	INFO.ca								\
	INFO.es								\
	INFO.fr								\
	INFO.html							\
	INFO.html.ca							\
	INFO.html.de							\
	INFO.html.es							\
	INFO.html.fr							\
	INFO.html.it							\
	INFO.it								\
	INFO.vi								\
	LGPL								\
	LGPL.html

bin_PROGRAMS =  xfce4-about

xfce4_about_SOURCES =							\
	info.c								\
	xfce-logo-icon.h

xfce4_about_CFLAGS =							\
	@LIBGTKHTML_CFLAGS@						\
	@LIBXFCEGUI4_CFLAGS@						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DDATADIR=\"$(datadir)/xfce4\"

xfce4_about_LDADD =							\
	@LIBGTKHTML_LIBS@						\
	@LIBXFCEGUI4_LIBS@

IMAGES = xfce-logo.jpg

VARIABLES = xfce_logo_data $(srcdir)/$(IMAGES)

noinst_DATA = xfce-logo.jpg 

EXTRA_DIST = 								\
	$(about_DATA)							\
	$(noinst_DATA)

distclean-local:
	rm -f INFO INFO.fr INFO.ca INFO.html INFO.html.de INFO.html.ca INFO.html.fr

xfce-logo-icon.h: $(IMAGES)
	gdk-pixbuf-csource --raw --build-list $(VARIABLES) > $(srcdir)/xfce-logo-icon.h

