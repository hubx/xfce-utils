dnl $Id$
dnl
dnl Copyright (c) 2002-2004
dnl         The Xfce development team. All rights reserved.
dnl
dnl Written for Xfce by Olivier Fourdan <fourdan@xfce.org>, Jasper
dnl Huijsmans <jasper@xfce.org>, Benedikt Meurer <benny@xfce.org>
dnl and Xavier MAILLARD <zedek@fxgsproject.org>.
dnl

AC_INIT([xfrun/xfrun.c])

AM_INIT_AUTOMAKE([xfce-utils], [4.1.99.1])

AC_DEFINE_UNQUOTED([RELEASE_LABEL], ["Xfce 4.2 RC1"], [Optional release plain name])
AC_SUBST(RELEASE_LABEL)

AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

dnl Check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX
AM_CONDITIONAL([HAVE_CYGWIN], [test "`uname | grep \"CYGWIN\"`" != ""])

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

dnl Check for default browser to use
AC_MSG_CHECKING([what browser to use by default])
AC_ARG_WITH([browser],
AC_HELP_STRING([--with-browser=BROWSER], [What should be the default browser?]),
  [], [with_browser=""])
BROWSER_DEFAULT=$with_browser
AC_SUBST([BROWSER_DEFAULT])
AC_MSG_RESULT([$with_browser])

dnl enable GDM-specific startup files?
AC_MSG_CHECKING([whether to install GDM specific files])
AC_ARG_ENABLE([gdm],
AC_HELP_STRING([--enable-gdm], [enable the installation of GDM specific files])
AC_HELP_STRING([--disable-gdm], [do not install GDM specific files (default)]),
	[], [enable_gdm=no])
USE_GDM=$enable_gdm
AC_SUBST([USE_GDM])
AC_MSG_RESULT([$enable_gdm])

dnl Check where to install GDM session files
if test x"$enable_gdm" = x"yes"; then
  GDM_PREFIX=/etc/X11
  AC_MSG_CHECKING(where GDM keeps session files)
  AC_ARG_WITH(gdm-prefix,
  AC_HELP_STRING([--with-gdm-prefix=STR], [where gdm data are installed (default=/etc/X11)]),
  	[], [with_gdm_prefix=/etc/X11])
  GDM_PREFIX=$with_gdm_prefix
  AC_SUBST(GDM_PREFIX)
  AC_MSG_RESULT($with_gdm_prefix)
fi

dnl Check for i18n support
BM_I18N([xfce-utils], [ar az be bg ca cs de es es_MX et eu fa fi fr gu he
		       hi hu it ja ko lt ms nl pa pl pt_BR pt_PT ru sk
		       sv ta tr uk vi zh_CN zh_TW])

dnl Check for required packages
BM_DEPEND([LIBXFCE4MCS_CLIENT], [libxfce4mcs-client-1.0], [4.1.3])
BM_DEPEND([LIBXFCEGUI4], [libxfcegui4-1.0], [4.1.27])

dnl configure the mcs plugin
XFCE_MCS_PLUGIN([XFCE_MCS_MANAGER], [4.1.3])

dnl Check for gtkhtml installed
BM_DEPEND_CHECK([LIBGTKHTML], [libgtkhtml-2.0], [2.0.0],
  [gtkhtml], [GTK+ HTML library], [no])

dnl Check for dbh installed
BM_DEPEND_CHECK([LIBDBH], [dbh-1.0], [1.0],
  [dbh], [DBH library], [yes])



dnl Check for debugging support
BM_DEBUG_SUPPORT

dnl Check for vendor specific information
AC_MSG_CHECKING([for additional vendor info])
AC_ARG_WITH([vendor-info],
AC_HELP_STRING([--with-vendor-info=FILE], [Specify an additional file that should be displayed in the about dialog]),
  [], [with_vendor_info=])
if test x"$with_vendor_info" != x""; then
  AC_DEFINE_UNQUOTED([VENDOR_INFO], ["$with_vendor_info"], [Additional vendor info])
  AC_MSG_RESULT([$with_vendor_info])
else
  AC_MSG_RESULT([none])
fi

AC_OUTPUT([
xfce-utils.spec
Makefile
po/Makefile.in
doc/Makefile
doc/index/Makefile
doc/index/C/Makefile
doc/index/fr/Makefile
doc/index/it/Makefile
doc/xfce-utils/Makefile
doc/xfce-utils/C/Makefile
doc/xfce-utils/C/images/Makefile
doc/xfce-utils/fr/Makefile
doc/xfce-utils/fr/images/Makefile
doc/xfce-utils/it/Makefile
doc/xfce-utils/it/images/Makefile
doc/xfce4-user-guide/Makefile
doc/xfce4-user-guide/C/Makefile
doc/xfce4-user-guide/C/images/Makefile
doc/xfce4-user-guide/fr/Makefile
doc/xfce4-user-guide/fr/images/Makefile
doc/xfce4-user-guide/it/Makefile
doc/xfce4-user-guide/it/images/Makefile
icons/Makefile
icons/48x48/Makefile
icons/scalable/Makefile
xfrun/Makefile
xftaskbar/Makefile
xftaskbar/plugin/Makefile
scripts/Makefile
gdm/Makefile
xfce4-about/Makefile
])

