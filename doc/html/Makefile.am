htmldir=$(datadir)/xfce4/doc/html

html_DATA=\
GPL.html \
xfce4-copyright.html \
xfce4-using.html \
index.html \
xfce-utils-taskbar.html \
xfce-utils-run.html \
xfce-utils-about.html \
xfce-utils-scripts.html \
xfce-utils-copyright.html \
xfce-utils.html

EXTRA_DIST = $(html_DATA)

html:
	xsltproc $(srcdir)/../xml/xfce.xsl $(srcdir)/../xml/xfce4.xml
	xsltproc $(srcdir)/../xml/xfce.xsl $(srcdir)/../xml/xfce-utils.xml

