bin_PROGRAMS =  xfrun4

xfrun4_SOURCES = xfrun.c 

xfrun4_CFLAGS =								\
	@LIBXFCEGUI4_CFLAGS@						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DDATADIR=\"$(datadir)/xfce4\"					\
	-DSYSCONFDIR=\"$(sysconfdir)/xfce4\"

xfrun4_LDADD =								\
	@LIBXFCEGUI4_LIBS@

