aboutdir=$(datadir)/xfce4

about_DATA = COPYING AUTHORS

bin_PROGRAMS =  xfce4-about

xfce4_about_SOURCES = info.c xfce-logo-icon.h

xfce4_about_CFLAGS = -I$(top_srcdir) @CFLAGS@ @X_CFLAGS@ @BASE_DEPENDENCIES_CFLAGS@  -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DDATADIR=\"$(datadir)/xfce4\" -DSYSCONFDIR=\"$(sysconfdir)/xfce4\" -DVERSION=\"@VERSION@\"
xfce4_about_LDADD = @LIBS@ @X_LIBS@  @BASE_DEPENDENCIES_LIBS@ 

IMAGES=xfce-logo.jpg
VARIABLES=xfce_logo_data $(srcdir)/$(IMAGES)

noinst_DATA = xfce-logo.jpg 

EXTRA_DIST = $(noinst_DATA)

xfce-logo-icon.h: $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $(srcdir)/xfce-logo-icon.h

