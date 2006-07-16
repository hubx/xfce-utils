bin_PROGRAMS =  xfrun4

xfrun4_SOURCES = \
	xfrun-dialog.c \
	xfrun-dialog.h

#if HAVE_DBUS

#xfrun4_SOURCES += \
#	xfrun-dbus.c

#else

xfrun4_SOURCES += \
	xfrun.c

#endif

xfrun4_CFLAGS = \
	-DLOCALEDIR=\"$(localedir)\" \
	@LIBXFCE4UTIL_CFLAGS@ \
	@LIBXFCEGUI4_CFLAGS@ \
	@GTK_CFLAGS@ \
	@DBUS_CFLAGS@

xfrun4_LDADD = \
	@LIBXFCE4UTIL_LIBS@ \
	@LIBXFCEGUI4_LIBS@ \
	@GTK_LIBS@ \
	@DBUS_LIBS@

EXTRA_DIST = \
	xfrun.c \

#	xfrun-dbus.c
