/*  xftaskbar
 *  Copyright (C) 2003 Olivier Fourdan (fourdan@xfce.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GDK_MULTIHEAD_SAFE
#undef GDK_MULTIHEAD_SAFE
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <libxfcegui4/libxfcegui4.h>
#include <libxfce4mcs/mcs-client.h>
#include <libxfce4util/i18n.h>

#define CHANNEL  "taskbar"
#define HIDDEN_HEIGHT 5
#define TOP TRUE
#define BOTTOM FALSE
#define DEFAULT_HEIGHT  30

static McsClient *client = NULL;

typedef struct _Taskbar Taskbar;

struct _Taskbar
{
    guint x, y, width, height;
    gboolean position;
    gboolean autohide;
    gboolean show_pager;
    gboolean show_tray;
    gboolean all_tasks;
    gboolean hidden;
    GtkWidget *win;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *tasklist;
    GtkWidget *pager;

    XfceSystemTray *tray;
    GtkWidget *iconbox;
};

static GdkFilterReturn
client_event_filter(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    if(mcs_client_process_event(client, (XEvent *) xevent))
        return GDK_FILTER_REMOVE;
    else
        return GDK_FILTER_CONTINUE;
}

static void
watch_cb(Window window, Bool is_start, long mask, void *cb_data)
{
    GdkWindow *gdkwin;

    gdkwin = gdk_window_lookup(window);

    if(is_start)
    {
        if(!gdkwin)
        {
            gdkwin = gdk_window_foreign_new(window);
        }
        else
        {
            g_object_ref(gdkwin);
        }
        gdk_window_add_filter(gdkwin, client_event_filter, cb_data);
    }
    else
    {
        g_assert(gdkwin);
        gdk_window_remove_filter(gdkwin, client_event_filter, cb_data);
        g_object_unref(gdkwin);
    }
}

static gint taskbar_get_thickness(Taskbar *taskbar)
{
    GtkStyle *style;
    
    g_return_val_if_fail (taskbar != NULL, 2);
    style = gtk_widget_get_style(taskbar->win);
    if (style)
    {
        return style->ythickness;
    }
    g_warning(_("Cannot get initial style"));
    return 2;
}

static gint taskbar_get_height(Taskbar *taskbar)
{
    g_return_val_if_fail (taskbar != NULL, DEFAULT_HEIGHT);
    if (taskbar->hidden)
    {
        return (2 * taskbar_get_thickness(taskbar) + 1);
    }
    return taskbar->height;
}

static void taskbar_update_margins(Taskbar *taskbar)
{
    DesktopMargins margins;
    guint height;

    g_return_if_fail (taskbar != NULL);
    if (taskbar->autohide)
    {
        height = (2 * taskbar_get_thickness(taskbar) + 1);
    }
    else
    {
        height = taskbar->height;
    }
    
    margins.left = 0;
    margins.right = 0;
    margins.top = (taskbar->position) ? height : 0;
    margins.bottom = (taskbar->position) ? 0 : height;
    netk_set_desktop_margins(GDK_WINDOW_XWINDOW(taskbar->win->window), &margins);
}

static void taskbar_position(Taskbar *taskbar)
{
    g_return_if_fail (taskbar != NULL);
    gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar->width, taskbar_get_height(taskbar));
}

static void taskbar_toggle_autohide(Taskbar *taskbar)
{
    g_return_if_fail (taskbar != NULL);
    if (taskbar->autohide)
    {
        gtk_widget_hide (taskbar->frame);
        taskbar->hidden = TRUE;
        taskbar_position(taskbar);
    }
    else
    {
        gtk_widget_show (taskbar->frame);
        taskbar->hidden = FALSE;
        taskbar_position(taskbar);
    }
    taskbar_update_margins(taskbar);
}

static void taskbar_toggle_pager(Taskbar *taskbar)
{
    g_return_if_fail (taskbar != NULL);
    if (taskbar->show_pager)
    {
        gtk_widget_show (taskbar->pager);
    }
    else
    {
        gtk_widget_hide (taskbar->pager);
    }
}

static void taskbar_toggle_tray(Taskbar *taskbar)
{
    g_return_if_fail (taskbar != NULL);
    if (taskbar->show_tray)
    {
        gtk_widget_show (taskbar->iconbox);
    }
    else
    {
        gtk_widget_hide (taskbar->iconbox);
    }
}

static void taskbar_change_size(Taskbar *taskbar, int height)
{
    g_return_if_fail (taskbar != NULL);
    if (taskbar->height != height)
    {
        taskbar->height = height;
        gtk_widget_set_size_request(GTK_WIDGET(taskbar->pager), -1, taskbar->height - 2 * taskbar_get_thickness(taskbar));
        taskbar_position(taskbar);
        taskbar_update_margins(taskbar);
    }
}

static gboolean taskbar_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
    Taskbar *taskbar = (Taskbar *) data;
    
    g_assert (data != NULL);
    if ((allocation) && (widget == taskbar->win))
    {
        if (taskbar->position == TOP)
        {
            taskbar->y = 0;
        }
        else
        {
            taskbar->y = gdk_screen_height() - allocation->height;
        }
        gtk_window_move(GTK_WINDOW(taskbar->win), taskbar->x, taskbar->y);
    }
    return FALSE;
}

static gboolean taskbar_enter (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    Taskbar *taskbar = (Taskbar *) data;
  
    if (!(taskbar->autohide))
    {
        return FALSE;
    }
    if (event->detail != GDK_NOTIFY_INFERIOR)
    {
        gtk_widget_show (taskbar->frame);
        taskbar->hidden = FALSE;
        taskbar_position(taskbar);
    }
  
    return FALSE;
}

static gboolean taskbar_leave (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    Taskbar* taskbar = (Taskbar *) data;
  
    if (!(taskbar->autohide))
    {
        return FALSE;
    }
    if (event->detail != GDK_NOTIFY_INFERIOR)
    {
        gtk_widget_hide (taskbar->frame);
        taskbar->hidden = TRUE;
        taskbar_position(taskbar);
    }
    return FALSE;
}

static void notify_cb(const char *name, const char *channel_name, McsAction action, McsSetting * setting, void *data)
{
    Taskbar *taskbar = (Taskbar *) data;

    if(g_ascii_strcasecmp(CHANNEL, channel_name))
    {
        g_message(_("This should not happen"));
        return;
    }

    switch (action)
    {
        case MCS_ACTION_NEW:
        case MCS_ACTION_CHANGED:
            if(setting->type == MCS_TYPE_INT)
            {
                if (!strcmp(name, "Taskbar/Position"))
                {
                    taskbar->position = setting->data.v_int ? TOP : BOTTOM;
                    taskbar_position(taskbar);
                    taskbar_update_margins(taskbar);
                }
                else if (!strcmp(name, "Taskbar/AutoHide"))
                {
                    taskbar->autohide = setting->data.v_int ? TRUE : FALSE;
                    taskbar_toggle_autohide(taskbar);
                }
                else if (!strcmp(name, "Taskbar/ShowPager"))
                {
                    taskbar->show_pager = setting->data.v_int ? TRUE : FALSE;
                    taskbar_toggle_pager(taskbar);
                }
                else if(!strcmp(name, "Taskbar/ShowTray"))
                {
                    taskbar->show_tray = setting->data.v_int ? TRUE : FALSE;
                    taskbar_toggle_tray(taskbar);
                }
                else if(!strcmp(name, "Taskbar/ShowAllTasks"))
                {
                    taskbar->all_tasks = setting->data.v_int ? TRUE : FALSE;
                    netk_tasklist_set_include_all_workspaces(NETK_TASKLIST(taskbar->tasklist), taskbar->all_tasks);
                }
                else if (!strcmp(name, "Taskbar/Height"))
                {
                    taskbar_change_size(taskbar, setting->data.v_int);
                }
            }
            break;
        case MCS_ACTION_DELETED:
        default:
            break;
    }
}

static void taskbar_destroy(GtkWidget * widget, gpointer data)
{
    g_assert (data != NULL);
    g_free(data);
}

static void icon_docked(XfceSystemTray *tray, GtkWidget *icon, GtkBox *iconbox)
{
    gtk_box_pack_start(iconbox, icon, FALSE, FALSE, 5);
    gtk_widget_show(icon);
}

static void icon_undocked(XfceSystemTray *tray, GtkWidget *icon, GtkBox *iconbox)
{
}

int main(int argc, char **argv)
{
    SessionClient *client_session;
    DesktopMargins margins;
    NetkScreen *screen;
    Taskbar *taskbar;
    GError *error;
    Display *dpy;
    int scr;
    int left, right;
    gboolean use_xinerama;

#ifdef ENABLE_NLS
    /* This is required for UTF-8 at least - Please don't remove it */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    gtk_init(&argc, &argv);

    client_session = client_session_new(argc, argv, NULL /* data */ , SESSION_RESTART_IF_RUNNING, 60);

    if(!session_init(client_session))
    {
        g_message(_("Cannot connect to session manager"));
    }
    dpy = GDK_DISPLAY();
    scr = XDefaultScreen(dpy);
    screen = netk_screen_get_default();

    /* because the pager doesn't respond to signals at the moment */
    netk_screen_force_update(screen);

    if(!netk_get_desktop_margins(DefaultScreenOfDisplay(dpy), &margins))
    {
        g_message(_("Cannot get desktop margins"));
    }
    
    use_xinerama = xineramaInit(dpy);
    left = isLeftMostHead(dpy, scr, 0, 0) ? margins.left : 0;
    right = isRightMostHead(dpy, scr, 0, 0) ? margins.right : 0;
    
    taskbar = g_new(Taskbar, 1);
    taskbar->x = left;
    taskbar->width = MyDisplayWidth(dpy, scr, 0, 0) - left - right;
    taskbar->y = 0;
    taskbar->height = 1;
    taskbar->position = TOP;
    taskbar->autohide = FALSE;
    taskbar->show_pager = TRUE;
    taskbar->show_tray = TRUE;
    taskbar->all_tasks = FALSE;
    taskbar->hidden = FALSE;

    taskbar->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_stick(GTK_WINDOW(taskbar->win));
    netk_gtk_window_set_dock_type(GTK_WINDOW(taskbar->win));
    netk_gtk_window_avoid_input(GTK_WINDOW(taskbar->win));
    gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar->width, taskbar->height);
    gtk_window_set_title(GTK_WINDOW(taskbar->win), "Task List");
    gtk_window_set_decorated(GTK_WINDOW(taskbar->win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(taskbar->win), FALSE);
    gtk_widget_show (taskbar->win);
    gtk_widget_add_events (taskbar->win, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect (G_OBJECT(taskbar->win), "destroy", G_CALLBACK(taskbar_destroy), taskbar);
    g_signal_connect (G_OBJECT(taskbar->win), "enter_notify_event", G_CALLBACK (taskbar_enter), taskbar);
    g_signal_connect (G_OBJECT(taskbar->win), "leave_notify_event", G_CALLBACK (taskbar_leave), taskbar);
    g_signal_connect (G_OBJECT(taskbar->win), "size_allocate", G_CALLBACK (taskbar_size_allocate), taskbar);

    taskbar->frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(taskbar->frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(taskbar->win), taskbar->frame);

    taskbar->hbox = gtk_hbox_new (FALSE, 1);
    gtk_container_add (GTK_CONTAINER (taskbar->frame), taskbar->hbox);

    taskbar->tasklist = netk_tasklist_new(screen);
    netk_tasklist_set_grouping(NETK_TASKLIST(taskbar->tasklist), NETK_TASKLIST_ALWAYS_GROUP);
    netk_tasklist_set_grouping_limit(NETK_TASKLIST(taskbar->tasklist), 100);
    netk_tasklist_set_include_all_workspaces(NETK_TASKLIST(taskbar->tasklist), taskbar->all_tasks);
    gtk_box_pack_start (GTK_BOX (taskbar->hbox), taskbar->tasklist, TRUE, TRUE, 0);

    taskbar->pager = netk_pager_new(screen);
    netk_pager_set_orientation(NETK_PAGER(taskbar->pager), GTK_ORIENTATION_HORIZONTAL);
    netk_pager_set_n_rows(NETK_PAGER(taskbar->pager), 1);
    gtk_box_pack_start(GTK_BOX(taskbar->hbox), taskbar->pager, FALSE, FALSE, 0);

    taskbar->iconbox = gtk_hbox_new(TRUE, 5);
    gtk_box_pack_start(GTK_BOX(taskbar->hbox), taskbar->iconbox, FALSE, FALSE,
                    0);

    taskbar->tray = xfce_system_tray_new();

    g_signal_connect(taskbar->tray, "icon_docked", G_CALLBACK(icon_docked), taskbar->iconbox);
    g_signal_connect(taskbar->tray, "icon_undocked", G_CALLBACK(icon_undocked), taskbar->iconbox);

    if (!xfce_system_tray_register(taskbar->tray, gdk_screen_get_default(), &error)) 
    {
        xfce_err("Unable to register system tray: %s", error->message);
        g_error_free(error);
    }

    gtk_widget_show (taskbar->iconbox);
    gtk_widget_show (taskbar->tasklist);
    gtk_widget_show (taskbar->pager);
    gtk_widget_show (taskbar->hbox);
    taskbar_change_size(taskbar, DEFAULT_HEIGHT);
    taskbar_position(taskbar);

    client = mcs_client_new(GDK_DISPLAY(), XDefaultScreen(GDK_DISPLAY()), notify_cb, watch_cb, taskbar);
    if(client)
    {
        mcs_client_add_channel(client, CHANNEL);
    }
    else
    {
        g_warning(_("Cannot create MCS client channel"));
    }

    gtk_main();

    xineramaFree();
    return 0;
}
