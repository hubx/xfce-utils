bin_PROGRAMS =  xfrun4

xfrun4_SOURCES = xfrun.c xfcombo.i

xfrun4_CFLAGS =								\
	@LIBXFCEGUI4_CFLAGS@						\
	@LIBDBH_CFLAGS@							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DLIBDIR=\"$(libdir)\"						\
	-DDATADIR=\"$(datadir)/xfce4\"					\
	-DSYSCONFDIR=\"$(sysconfdir)/xfce4\"

xfrun4_LDADD =								\
	@LIBXFCEGUI4_LIBS@						\
	@LIBDBH_LIBS@

