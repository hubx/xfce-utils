/*  xfce4
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@xfce.org)
 *                2002 Xavier MAILLARD (zedek@fxgsproject.org)
 *                2003 Jasper Huijsmans (huysmans@users.sourceforge.net)
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

#include <config.h>

#if 0
#include <my_gettext.h>
#endif

#define N_(x) x
#define _(x) x

#include <stdio.h>
#include <string.h>

#include <libxfcegui4/libxfcegui4.h>

#include "xfce-logo-icon.h"

#define SLOGAN "\" ... and mice fly ... \""

#ifndef XFCE_LICENSE
#define XFCE_LICENSE "COPYING"
#endif

#ifndef XFCE_AUTHORS
#define XFCE_AUTHORS "AUTHORS"
#endif

#define BORDER 6

static char *progname = NULL;

/* useful functions */
static void fill_buffer(const char *filename, char **buf, int *nb)
{
    GError *err = NULL;

    if(!filename)
        return;

    g_file_get_contents(filename, buf, nb, &err);

    if(err)
    {
        g_error("%s: %s", progname, err->message);
	g_error_free(err);
    }
}

static GtkWidget *create_bold_label(const char *text)
{
    GtkWidget *label;
    char *markup;

    if(!text)
        return gtk_label_new("");

    markup = g_strconcat("<b> ", text, " </b>", NULL);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), markup);

    return label;
}

static void info_help_cb(GtkWidget * w, gpointer data)
{
    exec_command("xfhelp4");
}

GtkWidget *create_scrolled_text_view(const char *buffer, int length, 
				     gboolean hscrolling)
{
    GtkWidget *frame, *sw, *textview;
    GtkTextBuffer *textbuffer;

    g_return_val_if_fail(buffer != NULL, NULL);
    
    textbuffer = gtk_text_buffer_new(NULL);
    textview = gtk_text_view_new();
    
    frame = gtk_frame_new(NULL);
    gtk_container_set_border_width(GTK_CONTAINER(frame), BORDER);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_widget_show(frame);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), 
		   hscrolling ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER,
		   GTK_POLICY_AUTOMATIC);
    gtk_widget_show(sw);
    gtk_container_add(GTK_CONTAINER(frame), sw);

    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(textbuffer), buffer, length);

    gtk_text_view_set_buffer(GTK_TEXT_VIEW(textview), textbuffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textview), BORDER);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textview), BORDER);

    gtk_widget_show(textview);
    gtk_container_add(GTK_CONTAINER(sw), textview);

    return frame;
}

static void add_info_header(GtkWidget *vbox, GdkPixbuf *icon)
{
    GtkWidget *header;
    char *text;

    text = 
	g_strdup_printf("%s\n<span size=\"smaller\" style=\"italic\">%s</span>",
		_("XFce Desktop Environment"), 
		_("Copyright 2002-2003 by Olivier Fourdan" ));
	   
    header = create_header(icon, text);
    g_free(text);   		   
    gtk_widget_show(header);
    
    gtk_box_pack_start(GTK_BOX(vbox), header, FALSE, FALSE, 0);
}

static void add_info_page(GtkNotebook * notebook)
{
    GtkWidget *info_label_1;
    GtkWidget *info_notebook_page;
    GtkWidget *info_view;
    char *buffer;

    info_label_1 = gtk_label_new(_("Info"));
    gtk_widget_show(info_label_1);

    /* %s, %s == version, slogan */
    buffer = g_strdup_printf(_("\n"
"XFce 4, version %s\n"
"%s\n"
"\n"
"XFce is a collection of programs that together provide a full-featured\n"
"desktop enviroment. The following programs are part of XFce:\n"
"\n"
"o Window manager (xfwm4)\n"
"   handles the placement of windows on the screen\n"
"\n"
"o Panel (xfce4-panel)\n"
"   program lauchers, popup menus, clock, desktop switcher and more.\n"
"\n"
"o Desktop manager (xfdesktop)\n"
"   sets a background color or image and provides a menu when you\n"
"   click on the desktop background\n"
"\n"
"o File manager (xffm)\n"
"   fast file manager\n"
"\n"
"o Utilities\n"
"   xfrun4: run programs\n"
"   xftaskbar4: simple taskbar with optional pager\n"
"\n\n"
"Thank you for your interest in XFce,\n"
"\n"
"                -- The XFce Development Team --\n"), 
    VERSION, SLOGAN);


    info_view = create_scrolled_text_view(buffer, -1, FALSE);
    g_free(buffer);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), info_view,
                             info_label_1);
}

static void add_credits_page(GtkNotebook * notebook)
{
    GtkWidget *info_label;
    GtkWidget *info_credits_view;
    char *filename, *buffer;
    int nbytes_read;

    info_label = gtk_label_new(_("Credits"));
    gtk_widget_show(info_label);

    filename = g_build_filename(DATADIR, XFCE_AUTHORS, NULL);
    fill_buffer(filename, &buffer, &nbytes_read);
    g_free(filename);

    info_credits_view = create_scrolled_text_view(buffer, nbytes_read, FALSE);
    gtk_widget_show(info_credits_view);
    g_free(buffer);
    
    gtk_notebook_append_page(notebook, info_credits_view, info_label);
}

static void add_license_page(GtkNotebook * notebook)
{
    GtkWidget *info_label;
    GtkWidget *info_license_view;
    char *filename, *buffer;
    int nbytes_read;

    info_label = gtk_label_new(_("License"));
    gtk_widget_show(info_label);

    filename = g_build_filename(DATADIR, XFCE_LICENSE, NULL);
    fill_buffer(filename, &buffer, &nbytes_read);
    g_free(filename);

    info_license_view = create_scrolled_text_view(buffer, nbytes_read, TRUE);
    gtk_widget_show(info_license_view);
    g_free(buffer);
    
    gtk_notebook_append_page(notebook, info_license_view, info_label);
}

int main(int argc, char **argv)
{
    GtkWidget *info;
    GtkWidget *vbox, *vbox2;
    GtkWidget *notebook;
    GtkWidget *buttonbox;
    GtkWidget *info_ok_button;
    GtkWidget *info_help_button;
    GdkPixbuf *logo_pb;

    progname = argv[0];

    gtk_init(&argc, &argv);
    
    info = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(info), _("About XFce 4"));
    gtk_window_stick(GTK_WINDOW(info));

    logo_pb = inline_icon_at_size(xfce_logo_data, 48, 48);
    gtk_window_set_icon(GTK_WINDOW(info), logo_pb);
    
    vbox2 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox2);
    gtk_container_add(GTK_CONTAINER(info), vbox2);

    /* header with logo */
    add_info_header(vbox2, logo_pb);
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
    add_info_page(GTK_NOTEBOOK(notebook));
    add_credits_page(GTK_NOTEBOOK(notebook));
    add_license_page(GTK_NOTEBOOK(notebook));

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
}
