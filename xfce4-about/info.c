/*  xfce4
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@xfce.org)
 *                2002 Xavier MAILLARD (zedek@fxgsproject.org)
 *                2003 Jasper Huijsmans (huysmans@users.sourceforge.net)
 *                2003 Benedikt Meurer (benedikt.meurer@unix-ag.uni-siegen.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libxfce4util/i18n.h>
#include <libxfcegui4/libxfcegui4.h>

#include "xfce-logo-icon.h"

#ifndef XFCE_LICENSE
#define XFCE_LICENSE	"COPYING"
#endif

#ifndef XFCE_AUTHORS
#define XFCE_AUTHORS	"AUTHORS"
#endif

#ifndef XFCE_INFO
#define XFCE_INFO	"INFO"
#endif

#define BORDER 6

static void
info_help_cb(GtkWidget *w, gpointer data)
{
	exec_command("xfhelp4");
}

static void
add_page(GtkNotebook *notebook, const gchar *name, const gchar *filename,
		gboolean hscrolling)
{
	GtkTextBuffer *textbuffer;
	GtkWidget *textview;
	GtkWidget *label;
	GtkWidget *view;
	GtkWidget *sw;
	GError *err;
	char *path;
	char *buf;
	int n;

	err = NULL;

	label = gtk_label_new(name);
	gtk_widget_show(label);

	path = g_build_filename(DATADIR, filename, NULL);

	g_file_get_contents(path, &buf, &n, &err);

	if (err != NULL) {
		xfce_err("%s", err->message);
		g_error_free(err);
	}
	else {
		textbuffer = gtk_text_buffer_new(NULL);
		gtk_text_buffer_set_text(textbuffer, buf, n);

		view = gtk_frame_new(NULL);
		gtk_container_set_border_width(GTK_CONTAINER(view), BORDER);
		gtk_frame_set_shadow_type(GTK_FRAME(view), GTK_SHADOW_IN);
		gtk_widget_show(view);

		sw = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), 
			hscrolling ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER,
			GTK_POLICY_AUTOMATIC);
		gtk_widget_show(sw);
		gtk_container_add(GTK_CONTAINER(view), sw);

		textview = gtk_text_view_new_with_buffer(textbuffer);
		gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
		gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textview), BORDER);
		gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textview), BORDER);
		gtk_widget_show(textview);
		gtk_container_add(GTK_CONTAINER(sw), textview);

		gtk_notebook_append_page(notebook, view, label);

		g_free(buf);
	}

	g_free(path);
}

int
main(int argc, char **argv)
{
    GtkWidget *info;
    GtkWidget *header;
    GtkWidget *vbox, *vbox2;
    GtkWidget *notebook;
    GtkWidget *buttonbox;
    GtkWidget *info_ok_button;
    GtkWidget *info_help_button;
    GdkPixbuf *logo_pb;
    char *text;

    gtk_init(&argc, &argv);

    /* XXX - We could use a GtkDialog instead here, since this is
     * really a simple dialog.
     */
    info = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(info), _("About XFce 4"));
    gtk_window_stick(GTK_WINDOW(info));

    logo_pb = inline_icon_at_size(xfce_logo_data, 48, 48);
    gtk_window_set_icon(GTK_WINDOW(info), logo_pb);
    
    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox2);
    gtk_container_add(GTK_CONTAINER(info), vbox2);

    /* header with logo */
    text = g_strdup_printf(
		    "%s\n<span size=\"smaller\" style=\"italic\">%s</span>",
		    _("XFce Desktop Environment"), 
		    _("Copyright 2002-2003 by Olivier Fourdan"));
    header = create_header(logo_pb, text);
    gtk_widget_show(header);
    gtk_box_pack_start(GTK_BOX(vbox2), header, FALSE, FALSE, 0);
    g_free(text);
    g_object_unref(logo_pb);

    vbox = gtk_vbox_new(FALSE, BORDER);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), BORDER);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(vbox2), vbox, TRUE, TRUE, 0);

    /* the notebook */
    notebook = gtk_notebook_new();
    gtk_widget_show(notebook);
    gtk_widget_set_size_request(notebook, -1, 300);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    /* add pages */
    add_page(GTK_NOTEBOOK(notebook), _("Info"), XFCE_INFO, FALSE);
    add_page(GTK_NOTEBOOK(notebook), _("Credits"), XFCE_AUTHORS, FALSE);
    add_page(GTK_NOTEBOOK(notebook), _("License"), XFCE_LICENSE, TRUE);

    /* buttons */
    buttonbox = gtk_hbutton_box_new();
    gtk_widget_show(buttonbox);
    gtk_box_pack_start(GTK_BOX(vbox), buttonbox, FALSE, FALSE, BORDER);

    info_ok_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_widget_show(info_ok_button);
    gtk_box_pack_start(GTK_BOX(buttonbox), info_ok_button, FALSE, FALSE, 0);

    info_help_button = gtk_button_new_from_stock(GTK_STOCK_HELP);
    gtk_widget_show(info_help_button);
    gtk_box_pack_start(GTK_BOX(buttonbox), info_help_button, FALSE, FALSE, 0);

    gtk_button_box_set_child_secondary(GTK_BUTTON_BOX(buttonbox),
                                       info_ok_button, TRUE);

    g_signal_connect(info, "delete-event", 
	    	     G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(info, "destroy-event", 
	    	     G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(info_ok_button, "clicked", 
	    	     G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(info_help_button, "clicked", 
	    	     G_CALLBACK(info_help_cb), NULL);

    gtk_window_set_position(GTK_WINDOW(info), GTK_WIN_POS_CENTER);
    gtk_widget_show(info);

    gtk_main();

    return(EXIT_SUCCESS);
}
