SUBDIRS = \
  	 images

noinst_DATA = \
	BSD.xml \
	copyright.xml \
	GPL.xml \
	LGPL.xml \
	xfce4-user-guide.xml

docdir=$(datadir)/xfce4/doc/C

doc_DATA= \
	BSD.html \
	GPL.html \
	LGPL.html \
	xfce4-user-guide.html \
	xfce4-intro.html \
	xfce4-copyright.html \
	xfce4-use.html \
	xfce4-install.html

EXTRA_DIST = $(noinst_DATA) $(doc_DATA)

html: $(srcdir)/xfce4-user-guide.xml
	xsltproc $(top_srcdir)/doc/xfce.xsl $(srcdir)/xfce4-user-guide.xml

