SUBDIRS = \
  	 images

noinst_DATA = \
	xfce-utils.xml

docdir=$(datadir)/xfce4/doc/fr

doc_DATA= \
	xfce-utils.html

EXTRA_DIST = $(noinst_DATA) $(doc_DATA)

html: $(srcdir)/xfce-utils.xml
	xsltproc $(top_srcdir)/doc/xfce-nochunk.xsl $(srcdir)/xfce-utils.xml
