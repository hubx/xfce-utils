/*
 * xfrun - a simple quick run dialog with saved history and completion
 *
 * Copyright (c) 2006 Brian J. Tarricone <bjt23@cornell.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include "xfrun-dialog.h"

int
main(int argc,
     char **argv)
{
    GtkWidget *dialog;
    const gchar *run_argument = NULL;

    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

    gtk_init(&argc, &argv);

    if(argc > 1) {
        if(!strcmp(argv[1], "--daemon") || !strcmp(argv[1], "--quit")) {
            /* we don't support daemon mode */
            xfce_message_dialog(NULL, _("Daemon Mode"),
                                GTK_STOCK_DIALOG_ERROR,
                                _("Daemon mode is not supported."),
                                _("Xfrun must be compiled with D-BUS support to enable daemon mode."),
                                GTK_STOCK_QUIT, GTK_RESPONSE_ACCEPT, NULL);
            return 1;
        } else
            run_argument = argv[1];
    }

    dialog = xfrun_dialog_new(run_argument);
    xfce_gtk_window_center_on_active_screen(GTK_WINDOW(dialog));
    gtk_widget_show(dialog);
    g_signal_connect(G_OBJECT(dialog), "closed",
                     G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_set_size_request(dialog, -1, -1);

    gtk_main();

    return 0;
}
