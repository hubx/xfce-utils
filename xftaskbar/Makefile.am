SUBDIRS = plugin

bin_PROGRAMS =  xftaskbar4

xftaskbar4_SOURCES = xftaskbar.c 

xftaskbar4_CFLAGS =							\
	@LIBXFCEGUI4_CFLAGS@						\
	@LIBXFCE4MCS_CLIENT_CFLAGS@					\
	-DDATADIR=\"$(datadir)/xfce4\"					\
	-DSYSCONFDIR=\"$(sysconfdir)/xfce4\"

xftaskbar4_LDADD =							\
	@LIBXFCEGUI4_LIBS@						\
	@LIBXFCE4MCS_CLIENT_LIBS@

