aboutdir=$(datadir)/xfce4

about_DATA =								\
	AUTHORS								\
	COPYING

bin_PROGRAMS =  xfce4-about

xfce4_about_SOURCES =							\
	info.c								\
	xfce-logo-icon.h

xfce4_about_CFLAGS =							\
	@LIBXFCEGUI4_CFLAGS@						\
	-DDATADIR=\"$(datadir)/xfce4\"					\
	-DSYSCONFDIR=\"$(sysconfdir)/xfce4\"

xfce4_about_LDADD =							\
	@LIBXFCEGUI4_LIBS@

IMAGES = xfce-logo.jpg

VARIABLES = xfce_logo_data $(srcdir)/$(IMAGES)

noinst_DATA = xfce-logo.jpg 

EXTRA_DIST = $(noinst_DATA)

xfce-logo-icon.h: $(IMAGES)
	gdk-pixbuf-csource --raw --build-list $(VARIABLES) > $(srcdir)/xfce-logo-icon.h

