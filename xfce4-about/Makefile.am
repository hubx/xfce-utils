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
	AUTHORS.html.ru                                                 \
	AUTHORS.it							\
	AUTHORS.lt							\
	AUTHORS.ru							\
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
	info.c

xfce4_about_CFLAGS =							\
	@LIBGTKHTML_CFLAGS@						\
	@LIBXFCEGUI4_CFLAGS@						\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DDATADIR=\"$(datadir)/xfce4\"

xfce4_about_LDADD =							\
	@LIBGTKHTML_LIBS@						\
	@LIBXFCEGUI4_LIBS@

EXTRA_DIST = 								\
	$(about_DATA)

