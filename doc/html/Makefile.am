htmldir=$(datadir)/xfce4/doc/html

html_DATA=\
GPL.html \
xfce4-copyright.html \
xfce4-using.html \
index.html 

EXTRA_DIST = $(html_DATA)

html:
	xsltproc $(srcdir)/../xml/xfce.xsl $(srcdir)/../xml/xfce4.xml

