dnl configure.ac
dnl

AC_INIT([xfrun/xfrun.c])

AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([xfce-utils], [0.2])

AM_MAINTAINER_MODE

dnl Check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl Check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check C Compiler Characteristics
AC_C_CONST
AC_C_INLINE

dnl Check for standard header files
AC_HEADER_STDC

dnl Check for X11 installed
BM_LIBX11_REQUIRE

dnl enable GDM-specific startup files?
AC_MSG_CHECKING([whether to install GDM specific files])
AC_ARG_ENABLE([gdm],
AC_HELP_STRING([--enable-gdm], [enable the installation of GDM specific files])
AC_HELP_STRING([--disable-gdm], [do not install GDM specific files (default)]),
	[], [enable_gdm=no])
USE_GDM=$enable_gdm
AC_SUBST([USE_GDM])
AC_MSG_RESULT([$enable_gdm])

GDM_PREFIX=/etc/X11/gdm/Sessions
AC_MSG_CHECKING(where GDM keeps session files)
AC_ARG_WITH(gdm-prefix,
	[  --with-gdm-prefix=STR        where GDM keeps session files (default=/etc/X11/gdm/Sessions)],
	[ GDM_PREFIX="$withval", GDM_PREFIX=/etc/X11/gdm/Sessions]
)
AC_MSG_RESULT($GDM_PREFIX)
AC_SUBST(GDM_PREFIX)

dnl Check for required packages
BM_DEPEND([LIBXFCE4MCS_CLIENT], [libxfce4mcs-client-1.0], [0.0.3])
BM_DEPEND([LIBXFCEGUI4], [libxfcegui4-1.0], [0.0.19])
BM_DEPEND([XFCE_MCS_MANAGER], [xfce-mcs-manager], [0.1])

dnl Check for debugging support
BM_DEBUG_SUPPORT

AC_OUTPUT([
xfce-utils.spec
Makefile
doc/Makefile
doc/images/Makefile
doc/html/Makefile
doc/xml/Makefile
xfrun/Makefile
xftaskbar/Makefile
xftaskbar/plugin/Makefile
scripts/Makefile
gdm/Makefile
xfce4-about/Makefile
])

