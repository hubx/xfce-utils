aboutdir = $(datadir)/xfce4

about_DATA =								\
	AUTHORS								\
	AUTHORS.de							\
	AUTHORS.de.html							\
	AUTHORS.html							\
	COPYING								\
	INFO								\
	INFO.de.html							\
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

EXTRA_DIST = $(noinst_DATA)

distclean-local:
	rm -f INFO INFO.html

xfce-logo-icon.h: $(IMAGES)
	gdk-pixbuf-csource --raw --build-list $(VARIABLES) > $(srcdir)/xfce-logo-icon.h

