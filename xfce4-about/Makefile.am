aboutdir = $(datadir)/xfce4

about_DATA =								\
	AUTHORS								\
	BSD								\
	COPYING								\
	GPL								\
	INFO								\
	INFO.ca								\
	INFO.de								\
	INFO.es								\
	INFO.fi								\
	INFO.fr								\
	INFO.it								\
	INFO.ja								\
	INFO.pt								\
	INFO.ru								\
	INFO.uk								\
	INFO.vi								\
	LGPL

bin_PROGRAMS =  xfce4-about

xfce4_about_SOURCES =							\
	info.c

xfce4_about_CFLAGS =							\
	@LIBXFCE4UI_CFLAGS@						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DDATADIR=\"$(datadir)/xfce4\"

xfce4_about_LDADD =							\
	@LIBXFCE4UI_LIBS@

EXTRA_DIST = 								\
	$(about_DATA)

