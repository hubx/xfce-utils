/*
 * xfrun - a simple quick run dialog with saved history and completion
 *
 * Copyright (c) 2006 Brian J. Tarricone <bjt23@cornell.edu>
 *                    Jannis Pohlmann <jannis@xfce.org>
 *                    Jani Monoses <jani.monoses@gmail.com>
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "xfrun-dialog.h"

/**
 * method: org.xfce.RunDialog.OpenDialog
 * param:  display_name (DBUS_TYPE_STRING)
 * param:  working_directory (DBUS_TYPE_STRING)
 * param:  run_argument (DBUS_TYPE_STRING)
 * return: success, or org.xfce.RunDialog.ErrorGeneral
 * description: shows the run dialog on the specified display.  will execute
 *              the command in the specified working directory.  an optional
 *              argument can be passed that will be passed to the program that
 *              is executed.
 **
 * method: org.xfce.RunDialog.Quit
 * description: instructs the xfrun daemon to quit.
 */
#define RUNDIALOG_DBUS_SERVICE        "org.xfce.RunDialog"
#define RUNDIALOG_DBUS_INTERFACE      "org.xfce.RunDialog"
#define RUNDIALOG_DBUS_PATH           "/org/xfce/RunDialog"
#define RUNDIALOG_DBUS_METHOD_OPEN    "OpenDialog"
#define RUNDIALOG_DBUS_METHOD_QUIT    "Quit"
#define RUNDIALOG_DBUS_ERROR_GENERAL  "org.xfce.RunDialog.ErrorGeneral"


static GtkWidget *static_dialog = NULL;
static gboolean static_dialog_in_use = FALSE;

static void
xfrun_static_dialog_closed(XfrunDialog *dialog,
                           gpointer user_data)
{
    static_dialog_in_use = FALSE;
}

/* gdk_display_close() is essentially broken on X11.  so every time we call
 * gdk_display_open(), we can't close it, and thus leak memory.  instead, let's
 * keep a cache of all open displays, and, in normal situations we'll only have
 * one display open per screen.
 */
static GdkDisplay *
xfrun_find_or_open_display(const gchar *display_name)
{
    static GHashTable *open_displays = NULL;
    GdkDisplay *gdpy = NULL;
    
    if(G_UNLIKELY(!open_displays)) {
        open_displays = g_hash_table_new_full(g_str_hash, g_str_equal,
                                              (GDestroyNotify)g_free, NULL);
    }
    
    gdpy = g_hash_table_lookup(open_displays, display_name);
    if(!gdpy) {
        DBG("couldn't find display '%s'; opening a new one", display_name);
        gdpy = gdk_display_open(display_name);
        if(gdpy)
            g_hash_table_insert(open_displays, g_strdup(display_name), gdpy);
    }
    
    return gdpy;
}


/* server message handler */
static DBusHandlerResult
xfrun_handle_dbus_message(DBusConnection *connection,
                          DBusMessage *message,
                          void *user_data)
{
    if(dbus_message_is_method_call(message,
                                   RUNDIALOG_DBUS_INTERFACE,
                                   RUNDIALOG_DBUS_METHOD_OPEN))
    {
        DBusMessage *reply = NULL;
        DBusError derror;
        gchar *display_name = NULL, *cwd = NULL, *run_argument = NULL;
        
        dbus_error_init(&derror);
        
        if(!dbus_message_get_args(message, &derror, 
                                  DBUS_TYPE_STRING, &display_name,
                                  DBUS_TYPE_STRING, &cwd,
                                  DBUS_TYPE_STRING, &run_argument,
                                  DBUS_TYPE_INVALID))
        {
            reply = dbus_message_new_error(message, RUNDIALOG_DBUS_ERROR_GENERAL,
                                           derror.message);
            dbus_error_free(&derror);
        } else {
            GdkDisplay *gdpy;
            GdkScreen *gscreen = NULL;
            
            gdpy = xfrun_find_or_open_display(display_name);
            if(!gdpy) {
                gchar *msgstr = g_strdup_printf(_("Unable to open display \"%s\"."),
                                                display_name);
                reply = dbus_message_new_error(message,
                                               RUNDIALOG_DBUS_ERROR_GENERAL,
                                               msgstr);
                g_free(msgstr);
            } else {
                GtkWidget *dialog;
                
                gscreen = gdk_display_get_default_screen(gdpy);
                
                if(!strlen(run_argument))
                    run_argument = NULL;
                
                if(static_dialog_in_use) {
                    dialog = xfrun_dialog_new(run_argument);
                    xfrun_dialog_set_destroy_on_close(XFRUN_DIALOG(dialog),
                                                      TRUE);
                    xfrun_dialog_set_working_directory(XFRUN_DIALOG(dialog),
                                                       cwd);
                } else {
                    dialog = static_dialog;
                    xfrun_dialog_set_run_argument(XFRUN_DIALOG(dialog),
                                                  run_argument);
                    xfrun_dialog_set_working_directory(XFRUN_DIALOG(dialog),
                                                       cwd);
                    if(GTK_WIDGET_REALIZED(dialog)) {
                        gdk_x11_window_set_user_time(dialog->window,
                                                     gdk_x11_get_server_time(dialog->window));
                    }
                    static_dialog_in_use = TRUE;
                }
                
                /* this handles setting the dialog to the right screen */
                xfce_gtk_window_center_on_monitor(GTK_WINDOW(dialog),
                                                  gscreen, 0);
                xfrun_dialog_select_text(XFRUN_DIALOG(dialog));
                gtk_widget_show(dialog);
            
                reply = dbus_message_new_method_return(message);
                
            }
        }

        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        
        return DBUS_HANDLER_RESULT_HANDLED;
    } else if(dbus_message_is_method_call(message,
                                          RUNDIALOG_DBUS_INTERFACE,
                                          RUNDIALOG_DBUS_METHOD_QUIT))
    {
        DBusMessage *reply = dbus_message_new_method_return(message);
        dbus_connection_send(connection, reply, NULL);
        dbus_message_unref(reply);
        
        gtk_main_quit();
    } else if(dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL,
                                     "Disconnected"))
    {
        g_printerr(_("D-BUS message bus disconnected. Exiting ...\n"));
        gtk_main_quit();
        
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/* server registration */
static gboolean
xfrun_register_dbus_service()
{
    static const struct DBusObjectPathVTable vtable = {
        NULL, 
        xfrun_handle_dbus_message,
        NULL,
    };
    DBusConnection *connection;
    DBusError derror;
    
    dbus_error_init(&derror);
    connection = dbus_bus_get(DBUS_BUS_SESSION, &derror);
    if(G_UNLIKELY(!connection)) {
        dbus_error_free(&derror);
        return FALSE;
    }
    
    dbus_connection_setup_with_g_main(connection, NULL);
    
    if(dbus_bus_request_name(connection, RUNDIALOG_DBUS_SERVICE, 0, &derror) < 0) {
        dbus_error_free(&derror);
        return FALSE;
    }
    
    if(!dbus_connection_register_object_path(connection, RUNDIALOG_DBUS_PATH,
                                             &vtable, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

/* client handler */
static gboolean
xfrun_show_dialog(const gchar *run_argument)
{
    DBusConnection *connection;
    DBusMessage *method;
    DBusMessage *result;
    DBusError derror;
    gchar *cwd, *display_name, *dummy_run_argument = NULL;
    
    dbus_error_init(&derror);
    connection = dbus_bus_get(DBUS_BUS_SESSION, &derror);
    if(!connection) {
        dbus_error_free(&derror);
        return FALSE;
    }
    
    method = dbus_message_new_method_call(RUNDIALOG_DBUS_SERVICE,
                                          RUNDIALOG_DBUS_PATH,
                                          RUNDIALOG_DBUS_INTERFACE,
                                          RUNDIALOG_DBUS_METHOD_OPEN);
    dbus_message_set_auto_start(method, TRUE);
    
    display_name = gdk_screen_make_display_name(gdk_screen_get_default());
    cwd = g_get_current_dir();
    
    if(!run_argument)
        run_argument = dummy_run_argument = g_strdup("");
    
    dbus_message_append_args(method,
                             DBUS_TYPE_STRING, &display_name,
                             DBUS_TYPE_STRING, &cwd,
                             DBUS_TYPE_STRING, &run_argument,
                             DBUS_TYPE_INVALID);
    
    g_free(display_name);
    g_free(cwd);
    g_free(dummy_run_argument);
    
    result = dbus_connection_send_with_reply_and_block(connection, method,
                                                       5000, &derror);
    dbus_message_unref(method);
    if(!result) {
        dbus_error_free(&derror);
        return FALSE;
    }
    
    dbus_message_unref(result);
    dbus_connection_unref(connection);
    
    return TRUE;
}

/* send quit message */
static void
xfrun_send_quit()
{
    DBusConnection *connection;
    DBusMessage *method, *result;
    
    connection = dbus_bus_get(DBUS_BUS_SESSION, NULL);
    if(!connection)
        return;
    
    method = dbus_message_new_method_call(RUNDIALOG_DBUS_SERVICE,
                                          RUNDIALOG_DBUS_PATH,
                                          RUNDIALOG_DBUS_INTERFACE,
                                          RUNDIALOG_DBUS_METHOD_QUIT);
    dbus_message_set_auto_start(method, FALSE);
    
    result = dbus_connection_send_with_reply_and_block(connection, method,
                                                       5000, NULL);
    dbus_message_unref(method);
    if(result)
        dbus_message_unref(result);
    
    dbus_connection_unref(connection);
}

int
main(int argc,
     char **argv)
{
    gboolean have_gtk = gtk_init_check(&argc, &argv);

    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    
    if(argc > 1 && !strcmp(argv[1], "--quit"))
        xfrun_send_quit();
    else if(argc > 1 && !strcmp(argv[1], "--daemon")) {
        if(!have_gtk) {
            g_critical("GTK is not available, failing.");
            return 1;
        }
        
        if(argc == 2 || strcmp(argv[2], "--no-detach")) {  /* for debugging purposes... */
#ifdef HAVE_DAEMON
            if(daemon(1, 1)) {
                xfce_message_dialog(NULL, _("System Error"),
                                    GTK_STOCK_DIALOG_ERROR,
                                    _("Unable to fork to background:"),
                                    strerror(errno), GTK_STOCK_QUIT,
                                    GTK_RESPONSE_ACCEPT, NULL);
                return 1;
            }
#else
            switch(fork()) {
                case -1:
                    /* failed */
                    xfce_message_dialog(NULL, _("System Error"),
                                        GTK_STOCK_DIALOG_ERROR,
                                        _("Unable to fork to background:"),
                                        strerror(errno), GTK_STOCK_QUIT,
                                        GTK_RESPONSE_ACCEPT, NULL);
                    return 1;
                case 0:
                    /* child (daemon) */
# ifdef HAVE_SETSID
                    setsid();
# endif
                    break;
                default:
                    /* parent */
                    _exit(0);
            }
#endif
        }
        
        static_dialog = xfrun_dialog_new(NULL);
        xfrun_dialog_set_destroy_on_close(XFRUN_DIALOG(static_dialog), FALSE);
        g_signal_connect(G_OBJECT(static_dialog), "closed",
                         G_CALLBACK(xfrun_static_dialog_closed), NULL);
        static_dialog_in_use = FALSE;
        
        xfrun_register_dbus_service();
        
        gtk_main();
    } else {
        if(!have_gtk) {
            g_critical("GTK is not available, failing.");
            return 1;
        }
        
        if(!xfrun_show_dialog(argc > 1 ? argv[1] : NULL)) {
            GtkWidget *fallback_dialog = xfrun_dialog_new(argc > 1
                                                          ? argv[1]
                                                          : NULL);
            xfrun_dialog_set_destroy_on_close(XFRUN_DIALOG(fallback_dialog),
                                              TRUE);
            g_signal_connect(G_OBJECT(fallback_dialog), "destroy",
                             G_CALLBACK(gtk_main_quit), NULL);
            xfce_gtk_window_center_on_monitor_with_pointer(GTK_WINDOW(fallback_dialog));
            gtk_widget_show(fallback_dialog);
            
            gtk_main();
        }
    }
    
    return 0;
}
