SUBDIRS = \
	index \
	xfce4-user-guide \
	xfce-utils

noinst_DATA = \
	xfce.xsl \
	xfce-nochunk.xsl

cssdir = $(datadir)/xfce4/doc

css_DATA = \
	xfce.css

EXTRA_DIST = $(noinst_DATA) $(css_DATA)

html: Makefile
	make -C xfce4-user-guide html; \
	make -C xfce-utils html
	
