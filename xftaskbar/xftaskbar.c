/*  xftaskbar
 *  Copyright (C) 2003,2004 Olivier Fourdan (fourdan@xfce.org)
 *  Copyright (c) 2003,2004 Benedikt Meurer <benny@xfce.org>
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

#ifdef HAVE_MEMORY_H
#include <memory.h>
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
#include <libxfce4util/libxfce4util.h>

#define CHANNEL  "taskbar"
#define HIDDEN_HEIGHT  5
#define HIDE_TIMEOUT   500
#define UNHIDE_TIMEOUT 100
#define TOP            TRUE
#define BOTTOM         FALSE
#define DEFAULT_HEIGHT 30
#define DEFAULT_WIDTH_PERCENT 100
#define DEFAULT_HORIZ_ALIGN 0

static McsClient *client = NULL;

typedef struct _Taskbar Taskbar;

struct _Taskbar
{
    Display *dpy;
    int scr;

    GdkScreen *gscr;
    guint x, y, width, height;
    guint width_percent;
    gint horiz_align;  /* <0 left, =0 center, >0 right */
    gint hide_timeout;
    gint unhide_timeout;
    gboolean position;
    gboolean autohide;
    gboolean show_pager;
    gboolean show_tray;
    gboolean all_tasks;
    gboolean group_tasks;
    gboolean hidden;
    GtkWidget *win;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *tasklist;
    GtkWidget *pager;

    XfceSystemTray *tray;
    GtkWidget *iconbox;
    gboolean tray_registered;
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

static gint taskbar_get_width(Taskbar *taskbar)
{
  return (taskbar->width * taskbar->width_percent) / 100.0;
}

static gint taskbar_get_x(Taskbar *taskbar)
{
  if (taskbar->horiz_align < 0) {
    /* left */
    return taskbar->x;
  }
  else if (taskbar->horiz_align > 0) {
    /* right */
    return taskbar->x + (taskbar->width - taskbar_get_width(taskbar));
  }
  else {
    /* center */
    return taskbar->x + (taskbar->width - taskbar_get_width(taskbar)) / 2;
  }
}

static void taskbar_update_margins(Taskbar *taskbar)
{
    guint height;
    gulong data [12] = { 0, };

    g_return_if_fail (taskbar != NULL);
    
    if (taskbar->autohide)
    {
        height = (2 * taskbar_get_thickness(taskbar) + 1);
    }
    else
    {
        height = taskbar->height;
    }
    
    data[0]  = 0;                                 /* left           */	        		
    data[1]  = 0;                                 /* right          */	        		
    data[2]  = (taskbar->position) ? height : 0;  /* top            */	        		
    data[3]  = (taskbar->position) ? 0 : height;  /* bottom         */	        		
    data[4]  = 0;                                 /* left_start_y   */     		
    data[5]  = 0;                                 /* left_end_y     */       		
    data[6]  = 0;                                 /* right_start_y  */    		
    data[7]  = 0;                                 /* right_end_y    */      		
    data[8]  = taskbar_get_x (taskbar);  	  /* top_start_x    */			
    data[9]  = taskbar_get_x (taskbar) 
               + taskbar_get_width (taskbar);     /* top_end_x      */        		
    data[10] = taskbar_get_x (taskbar);           /* bottom_start_x */   		
    data[11] = taskbar_get_x (taskbar) 
               + taskbar_get_width (taskbar);     /* bottom_end_x   */     		

    gdk_error_trap_push ();
    gdk_property_change (taskbar->win->window,
			 gdk_atom_intern ("_NET_WM_STRUT", FALSE),
			 gdk_atom_intern ("CARDINAL", FALSE), 32,
			 GDK_PROP_MODE_REPLACE, (guchar *) &data, 4);
    gdk_property_change (taskbar->win->window,
			 gdk_atom_intern ("_NET_WM_STRUT_PARTIAL", FALSE),
			 gdk_atom_intern ("CARDINAL", FALSE), 32,
			 GDK_PROP_MODE_REPLACE, (guchar *) &data, 12);
    gdk_error_trap_pop ();
}

static void taskbar_position(Taskbar *taskbar)
{
    g_return_if_fail (taskbar != NULL);
    gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar_get_width(taskbar), taskbar_get_height(taskbar));
    if (GTK_WIDGET_REALIZED (taskbar->win))
      taskbar_update_margins(taskbar);
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

static gboolean
register_tray(Taskbar *taskbar)
{
    GError *error = NULL;

    if (xfce_system_tray_check_running(DefaultScreenOfDisplay(taskbar->dpy))) 
    {
	    xfce_info(_("There is already a system tray running on this "
			"screen"));
	    return FALSE;
    }
    else if (!xfce_system_tray_register(taskbar->tray, 
		DefaultScreenOfDisplay(taskbar->dpy),
		&error)) 
    {
        xfce_err(_("Unable to register system tray: %s"), error->message);
        g_error_free(error);

	return FALSE;
    }

    return TRUE;
}

/* called from an idle loop, so return FALSE to end loop */
static gboolean taskbar_toggle_tray(Taskbar *taskbar)
{
    g_return_val_if_fail (taskbar != NULL, FALSE);
    
    if (taskbar->show_tray)
    {
	if (!taskbar->tray_registered)
	{
	    taskbar->tray_registered = register_tray(taskbar);
	}

	taskbar->iconbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(taskbar->hbox), taskbar->iconbox, 
			   FALSE, FALSE,0);
	gtk_widget_show (taskbar->iconbox);
    }
    else
    {
	if (taskbar->tray_registered)
	{
	    xfce_system_tray_unregister(taskbar->tray);
	    taskbar->tray_registered = FALSE;
	}
	
	if (taskbar->iconbox) 
	{
            gtk_widget_destroy (taskbar->iconbox);
	    taskbar->iconbox = NULL;
	}
    }

    return FALSE;
}

static void taskbar_change_size(Taskbar *taskbar, int height)
{
    if (taskbar->height != height)
    {
        taskbar->height = height;
        gtk_widget_set_size_request(GTK_WIDGET(taskbar->pager), -1, taskbar->height - 2 * taskbar_get_thickness(taskbar));
        taskbar_position(taskbar);
    }
}

static void taskbar_set_width_percent(Taskbar *taskbar, int width_percent)
{
    if (taskbar->width_percent != width_percent)
    {
        taskbar->width_percent = width_percent;
        gtk_widget_set_size_request(GTK_WIDGET(taskbar->win),
                taskbar_get_width(taskbar),
                taskbar->height);
    }
}

static void taskbar_set_horiz_align(Taskbar *taskbar, int horiz_align)
{
    if (taskbar->horiz_align != horiz_align)
    {
        taskbar->horiz_align = horiz_align;
        gtk_widget_set_size_request(GTK_WIDGET(taskbar->win),
                taskbar_get_width(taskbar),
                taskbar->height);
    }
}

static gboolean taskbar_size_allocate (GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
    Taskbar *taskbar = (Taskbar *) data;
    GdkRectangle rect;
    gint monitor_nbr;
    
    g_assert (data != NULL);

    monitor_nbr = gdk_screen_get_monitor_at_point (taskbar->gscr, 0, 0);
    gdk_screen_get_monitor_geometry (taskbar->gscr, monitor_nbr, &rect);

    if ((allocation) && (widget == taskbar->win))
    {
        if (taskbar->position == TOP)
        {
            taskbar->y = rect.y;
        }
        else
        {
            taskbar->y = rect.y + rect.height - allocation->height;
        }
        gtk_window_move(GTK_WINDOW(taskbar->win), taskbar_get_x(taskbar), taskbar->y);
        if (GTK_WIDGET_REALIZED (taskbar->win))
          taskbar_update_margins(taskbar);
    }
    return FALSE;
}

static gboolean taskbar_unhide_timeout (Taskbar *taskbar)
{
    g_return_val_if_fail (taskbar != NULL, FALSE);

    gtk_widget_show (taskbar->frame);
    taskbar->hidden = FALSE;
    taskbar_position(taskbar);

    if (taskbar->unhide_timeout)
    {
	g_source_remove (taskbar->unhide_timeout);
	taskbar->unhide_timeout = 0;
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
	if (taskbar->hide_timeout)
	{
	    g_source_remove (taskbar->hide_timeout);
	    taskbar->hide_timeout = 0;
	}

	if (!taskbar->unhide_timeout)
	{
	    taskbar->unhide_timeout =
		g_timeout_add (UNHIDE_TIMEOUT,
			       (GSourceFunc) taskbar_unhide_timeout, taskbar);
	}

    }
  
    return FALSE;
}

static gboolean taskbar_hide_timeout (Taskbar *taskbar)
{
    g_return_val_if_fail (taskbar != NULL, FALSE);

    gtk_widget_hide (taskbar->frame);
    taskbar->hidden = TRUE;
    taskbar_position(taskbar);

    if (taskbar->hide_timeout)
    {
	g_source_remove (taskbar->hide_timeout);
	taskbar->hide_timeout = 0;
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
	if (taskbar->unhide_timeout)
	{
	    g_source_remove (taskbar->unhide_timeout);
	    taskbar->unhide_timeout = 0;
	}

	if (!taskbar->hide_timeout)
	{
	    taskbar->hide_timeout =
		g_timeout_add (HIDE_TIMEOUT, 
                               (GSourceFunc) taskbar_hide_timeout, taskbar);
	}
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
                    g_idle_add((GSourceFunc)taskbar_toggle_tray,(taskbar));
                }
                else if(!strcmp(name, "Taskbar/ShowAllTasks"))
                {
                    taskbar->all_tasks = setting->data.v_int ? TRUE : FALSE;
                    netk_tasklist_set_include_all_workspaces(NETK_TASKLIST(taskbar->tasklist), taskbar->all_tasks);
                }
                else if(!strcmp(name, "Taskbar/GroupTasks"))
                {
                    taskbar->group_tasks = setting->data.v_int ? TRUE : FALSE;
                    netk_tasklist_set_grouping(NETK_TASKLIST(taskbar->tasklist), taskbar->group_tasks ? NETK_TASKLIST_ALWAYS_GROUP : NETK_TASKLIST_AUTO_GROUP);
                }
                else if (!strcmp(name, "Taskbar/Height"))
                {
                    taskbar_change_size(taskbar, setting->data.v_int);
                }
                else if (!strcmp(name, "Taskbar/WidthPercent"))
                {
                    taskbar_set_width_percent(taskbar, setting->data.v_int);
                }
                else if (!strcmp(name, "Taskbar/HorizAlign"))
                {
                    taskbar_set_horiz_align(taskbar, setting->data.v_int);
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

static void icon_docked(XfceSystemTray *tray, GtkWidget *icon, Taskbar *taskbar)
{
    if (taskbar->tray_registered)
    {
	gtk_box_pack_start(GTK_BOX(taskbar->iconbox), icon, FALSE, FALSE, 0);
	gtk_widget_show(icon);
    }
}

static void icon_undocked(XfceSystemTray *tray, GtkWidget *icon, GtkBox *iconbox)
{
}

static void message_new(XfceSystemTray *tray, GtkWidget *icon, glong id,
		glong timeout, const gchar *text)
{
}

static void
taskbar_size_changed(GdkScreen *screen, Taskbar *taskbar)
{
    /*
     * NOTE: This breaks Xinerama, but anyway, Xinerama is not compatible
     * with Xrandr
     */
    taskbar->width = gdk_screen_get_width(screen);
    gtk_widget_set_size_request(GTK_WIDGET(taskbar->win),
            taskbar_get_width(taskbar),
            taskbar->height);
    taskbar_change_size(taskbar, taskbar->height);
}

int main(int argc, char **argv)
{
    SessionClient *client_session;
    NetkScreen *screen;
    Taskbar *taskbar;
    GdkRectangle rect;
    gint monitor_nbr;

    /* This is required for UTF-8 at least - Please don't remove it */
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    gtk_init(&argc, &argv);

    screen = netk_screen_get_default();
    /* because the pager doesn't respond to signals at the moment */
    netk_screen_force_update(screen);

    taskbar = g_new(Taskbar, 1);

    taskbar->gscr = gdk_screen_get_default ();
    if (!taskbar->gscr)
    {
	g_error (_("Cannot get default screen\n"));
    }
    gtk_widget_push_colormap(gdk_screen_get_rgb_colormap (taskbar->gscr));
    taskbar->scr = gdk_screen_get_number (taskbar->gscr);
    taskbar->dpy = gdk_x11_display_get_xdisplay (gdk_screen_get_display (taskbar->gscr));

    monitor_nbr = gdk_screen_get_monitor_at_point (taskbar->gscr, 0, 0);
    gdk_screen_get_monitor_geometry (taskbar->gscr, monitor_nbr, &rect);

    taskbar->hide_timeout = 0;
    taskbar->unhide_timeout = 0;
    taskbar->x = rect.x;
    taskbar->width = rect.width;
    taskbar->width_percent = DEFAULT_WIDTH_PERCENT;
    taskbar->horiz_align = DEFAULT_HORIZ_ALIGN;
    taskbar->y = rect.y;
    taskbar->height = 1;
    taskbar->position = TOP;
    taskbar->autohide = FALSE;
    taskbar->show_pager = TRUE;
    taskbar->show_tray = TRUE;
    taskbar->all_tasks = FALSE;
    taskbar->group_tasks = FALSE;
    taskbar->hidden = FALSE;
    taskbar->tray_registered = FALSE;

    taskbar->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(taskbar->win), "Task List");
    gtk_window_set_decorated(GTK_WINDOW(taskbar->win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(taskbar->win), FALSE);
    netk_gtk_window_set_type(GTK_WINDOW(taskbar->win), NETK_WINDOW_DOCK);
    if ((gtk_major_version >=2) && (gtk_minor_version >= 1))
    {
	g_object_set (G_OBJECT(taskbar->win), "skip_taskbar_hint", TRUE, NULL);
	g_object_set (G_OBJECT(taskbar->win), "skip_pager_hint", TRUE, NULL);
    }
    gtk_window_stick(GTK_WINDOW(taskbar->win));
    gtk_widget_add_events (taskbar->win, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    netk_gtk_window_avoid_input(GTK_WINDOW(taskbar->win));
    gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar->width, taskbar->height);
    
    g_signal_connect (G_OBJECT(taskbar->win), "destroy", G_CALLBACK(taskbar_destroy), taskbar);
    g_signal_connect (G_OBJECT(taskbar->win), "enter_notify_event", G_CALLBACK (taskbar_enter), taskbar);
    g_signal_connect (G_OBJECT(taskbar->win), "leave_notify_event", G_CALLBACK (taskbar_leave), taskbar);
    g_signal_connect (G_OBJECT(taskbar->win), "size_allocate", G_CALLBACK (taskbar_size_allocate), taskbar);
    g_signal_connect_swapped (G_OBJECT (taskbar->win), "realize", G_CALLBACK (taskbar_update_margins), taskbar);

    if (gdk_screen_get_n_monitors (taskbar->gscr)< 2) {
        /*
         * Xrandr compatibility, since Xrandr does not work well with
         * Xinerama, we only use it when theres no more than one Xinerama
         * screen reported.
         */
        g_signal_connect(G_OBJECT(taskbar->gscr), "size-changed", G_CALLBACK(taskbar_size_changed), taskbar);
    }

    taskbar->frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(taskbar->frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(taskbar->win), taskbar->frame);

    taskbar->hbox = gtk_hbox_new (FALSE, 1);
    gtk_container_add (GTK_CONTAINER (taskbar->frame), taskbar->hbox);

    taskbar->iconbox = NULL;

    taskbar->tasklist = netk_tasklist_new(screen);
    netk_tasklist_set_grouping(NETK_TASKLIST(taskbar->tasklist), taskbar->group_tasks);
    netk_tasklist_set_include_all_workspaces(NETK_TASKLIST(taskbar->tasklist), taskbar->all_tasks);
    gtk_box_pack_start (GTK_BOX (taskbar->hbox), taskbar->tasklist, TRUE, TRUE, 0);

    taskbar->pager = netk_pager_new(screen);
    netk_pager_set_orientation(NETK_PAGER(taskbar->pager), GTK_ORIENTATION_HORIZONTAL);
    netk_pager_set_n_rows(NETK_PAGER(taskbar->pager), 1);
    gtk_box_pack_start(GTK_BOX(taskbar->hbox), taskbar->pager, FALSE, FALSE, 0);

    taskbar->tray = xfce_system_tray_new();
    
/*    will get called when connecting to the mcs client 
 *    taskbar_toggle_tray(taskbar);*/

    g_signal_connect(taskbar->tray, "icon_docked", G_CALLBACK(icon_docked), taskbar);
    g_signal_connect(taskbar->tray, "icon_undocked", G_CALLBACK(icon_undocked), taskbar);
    g_signal_connect(taskbar->tray, "message_new", G_CALLBACK(message_new), taskbar);

    client_session = client_session_new(argc, argv, NULL /* data */ , SESSION_RESTART_IF_RUNNING, 30);

    if(!session_init(client_session))
    {
        g_message(_("Cannot connect to session manager"));
    }

    taskbar_change_size(taskbar, DEFAULT_HEIGHT);
    taskbar_position(taskbar);

    client = mcs_client_new(taskbar->dpy, taskbar->scr, notify_cb, watch_cb, taskbar);
    if(client)
    {
        mcs_client_add_channel(client, CHANNEL);
    }
    else
    {
        g_warning(_("Cannot create MCS client channel"));
	taskbar_toggle_tray(taskbar);
    }

    gtk_widget_show (taskbar->tasklist);
    gtk_widget_show (taskbar->hbox);
    gtk_widget_show (taskbar->win);

    gtk_main();

    return 0;
}
