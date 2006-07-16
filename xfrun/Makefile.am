bin_PROGRAMS =  xfrun4

xfrun4_SOURCES = \
	xfrun-dialog.c \
	xfrun-dialog.h

if HAVE_DBUS

xfrun4_SOURCES += \
	xfrun-dbus.c

else

xfrun4_SOURCES += \
	xfrun.c

endif

xfrun4_CFLAGS = \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDBUS_API_SUBJECT_TO_CHANGE \
	@LIBXFCE4UTIL_CFLAGS@ \
	@LIBXFCEGUI4_CFLAGS@ \
	@GTK_CFLAGS@ \
	@DBUS_CFLAGS@

xfrun4_LDADD = \
	@LIBXFCE4UTIL_LIBS@ \
	@LIBXFCEGUI4_LIBS@ \
	@GTK_LIBS@ \
	@DBUS_LIBS@


if HAVE_DBUS

service_in_files = org.xfce.RunDialog.service.in

servicedir = $(datadir)/dbus-1/services
service_DATA = $(service_in_files:.service.in=.service)

%.service: %.service.in
	sed -e "s,\@bindir\@,$(bindir),g" < $< > $@

DISTCLEANFILES = \
	$(service_DATA)

endif

EXTRA_DIST = \
	xfrun.c \
	xfrun-dbus.c \
	$(service_in_files)
