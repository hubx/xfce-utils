aboutdir = $(datadir)/xfce4

about_DATA =								\
	AUTHORS								\
	AUTHORS.ca							\
	AUTHORS.de							\
	AUTHORS.fr							\
	AUTHORS.html.ca							\
	AUTHORS.html.de							\
	AUTHORS.html.fr							\
	AUTHORS.html							\
	COPYING								\
	BSD								\
	LGPL								\
	GPL								\
	INFO								\
	INFO.fr								\
	INFO.html.ca							\
	INFO.html.de							\
	INFO.html.fr							\
	INFO.html

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
	rm -f INFO INFO.html INFO.html.de INFO.html.ca

xfce-logo-icon.h: $(IMAGES)
	gdk-pixbuf-csource --raw --build-list $(VARIABLES) > $(srcdir)/xfce-logo-icon.h

