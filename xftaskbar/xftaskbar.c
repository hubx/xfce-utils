
#include <gtk/gtk.h>
#include <libxfcegui4/libxfcegui4.h>

#define FIXED_HEIGHT 26
#define HIDDEN_HEIGHT 4

typedef struct _Taskbar Taskbar;
struct _Taskbar
{
    guint x, y, width, height;
    GtkWidget *win;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *tasklist;
    GtkWidget *pager;
    GtkWidget *eventbox;
};

static gboolean panel_enter (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    Taskbar *taskbar = (Taskbar *) data;
  
    if (event->detail != GDK_NOTIFY_INFERIOR)
    {
	gtk_widget_show (taskbar->frame);
	gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar->width, taskbar->height);
	gtk_window_move(GTK_WINDOW(taskbar->win), taskbar->x, taskbar->y);
    }
  
    return FALSE;
}

static gboolean panel_leave (GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    Taskbar* taskbar = (Taskbar *) data;
  
    if (event->detail != GDK_NOTIFY_INFERIOR)
    {
	gtk_widget_hide (taskbar->frame);
	gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar->width, HIDDEN_HEIGHT);
	gtk_window_move(GTK_WINDOW(taskbar->win), taskbar->x, taskbar->y);
    }
    return FALSE;
}

static void panel_destroy(GtkWidget * widget, gpointer data)
{
    g_assert (data != NULL);
    g_free(data);
}

int main(int argc, char **argv)
{
    SessionClient *client_session;
    DesktopMargins margins;
    NetkScreen *screen;
    Taskbar *taskbar;
    
    gtk_init(&argc, &argv);

    client_session = client_session_new(argc, argv, NULL /* data */ , SESSION_RESTART_IF_RUNNING, 60);

    if(!session_init(client_session))
    {
        g_message("Cannot connect to session manager");
    }

    screen = netk_screen_get_default();

    /* because the pager doesn't respond to signals at the moment */
    netk_screen_force_update(screen);

    if(!netk_get_desktop_margins(DefaultScreenOfDisplay(GDK_DISPLAY()), &margins))
    {
        g_message("Cannot set desktop margins");
    }
    
    taskbar = g_new(Taskbar, 1);
    taskbar->x = margins.left;
    taskbar->y = margins.top;
    taskbar->width = gdk_screen_width() - margins.left - margins.right;
    taskbar->height = FIXED_HEIGHT;

    taskbar->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_stick(GTK_WINDOW(taskbar->win));
    netk_gtk_window_set_dock_type(GTK_WINDOW(taskbar->win));
    gtk_window_set_title(GTK_WINDOW(taskbar->win), "Task List");
    gtk_window_set_decorated(GTK_WINDOW(taskbar->win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(taskbar->win), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(taskbar->win), taskbar->width, taskbar->height);
    gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar->width, taskbar->height);
    gtk_window_move(GTK_WINDOW(taskbar->win), taskbar->x, taskbar->y);
    g_signal_connect (G_OBJECT(taskbar->win), "destroy", G_CALLBACK(panel_destroy), taskbar);
    margins.top += HIDDEN_HEIGHT;
    
    taskbar->eventbox = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(taskbar->win), taskbar->eventbox);
    gtk_widget_add_events (taskbar->eventbox, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect (G_OBJECT(taskbar->eventbox), "enter_notify_event", G_CALLBACK (panel_enter), taskbar);
    g_signal_connect (G_OBJECT(taskbar->eventbox), "leave_notify_event", G_CALLBACK (panel_leave), taskbar);
    
    taskbar->frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(taskbar->frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(taskbar->eventbox), taskbar->frame);

    taskbar->hbox = gtk_hbox_new (FALSE, 1);
    gtk_container_add (GTK_CONTAINER (taskbar->frame), taskbar->hbox);

    taskbar->tasklist = netk_tasklist_new(screen);
    netk_tasklist_set_grouping(NETK_TASKLIST(taskbar->tasklist), NETK_TASKLIST_ALWAYS_GROUP);
    netk_tasklist_set_grouping_limit(NETK_TASKLIST(taskbar->tasklist), 0);
    gtk_widget_set_size_request (GTK_WIDGET (taskbar->tasklist), -1, FIXED_HEIGHT);
    gtk_box_pack_start (GTK_BOX (taskbar->hbox), taskbar->tasklist, TRUE, TRUE, 0);

    taskbar->pager = netk_pager_new(screen);
    netk_pager_set_orientation(NETK_PAGER(taskbar->pager), GTK_ORIENTATION_HORIZONTAL);
    netk_pager_set_n_rows(NETK_PAGER(taskbar->pager), 1);
    gtk_widget_set_size_request (GTK_WIDGET (taskbar->pager), -1, FIXED_HEIGHT);
    gtk_box_pack_end (GTK_BOX (taskbar->hbox), taskbar->pager, FALSE, FALSE, 0);

    gtk_widget_show (taskbar->tasklist);
    gtk_widget_show (taskbar->pager);
    gtk_widget_show (taskbar->hbox);
    gtk_widget_show (taskbar->frame);
    gtk_widget_show (taskbar->eventbox);
    gtk_widget_show (taskbar->win);

    gtk_widget_hide (taskbar->frame);
    gtk_widget_set_size_request(GTK_WIDGET(taskbar->win), taskbar->width, HIDDEN_HEIGHT);
    gtk_window_move(GTK_WINDOW(taskbar->win), taskbar->x, taskbar->y);

    netk_set_desktop_margins(GDK_WINDOW_XWINDOW(taskbar->win->window), &margins);

    gtk_main();

    return 0;
}
