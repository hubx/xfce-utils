
#include <gtk/gtk.h>
#include <libxfcegui4/libxfcegui4.h>

#define FIXED_HEIGHT 26

int main(int argc, char **argv)
{
    DesktopMargins margins;
    SessionClient *client_session;
    NetkScreen *screen;
    GtkWidget *win;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *tasklist;
    GtkWidget *pager;

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

    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_stick(GTK_WINDOW(win));
    netk_gtk_window_set_dock_type(GTK_WINDOW(win));
    gtk_window_stick(GTK_WINDOW(win));
    gtk_window_set_title(GTK_WINDOW(win), "Task List");
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(win), gdk_screen_width() - margins.left - margins.right, FIXED_HEIGHT);
    gtk_widget_set_size_request(GTK_WIDGET(win), gdk_screen_width() - margins.left - margins.right, FIXED_HEIGHT);
    gtk_window_move(GTK_WINDOW(win), margins.left, margins.top);
    margins.top += FIXED_HEIGHT;
    
    /* quit on window close */
    g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(win), frame);

    hbox = gtk_hbox_new (FALSE, 1);
    gtk_container_add (GTK_CONTAINER (frame), hbox);
    gtk_widget_show (hbox);

    tasklist = netk_tasklist_new(screen);
    netk_tasklist_set_grouping(NETK_TASKLIST(tasklist), NETK_TASKLIST_ALWAYS_GROUP);
    netk_tasklist_set_grouping_limit(NETK_TASKLIST(tasklist), 0);
    gtk_widget_set_size_request (GTK_WIDGET (tasklist), -1, FIXED_HEIGHT);
    gtk_box_pack_start (GTK_BOX (hbox), tasklist, TRUE, TRUE, 0);

    pager = netk_pager_new(screen);
    netk_pager_set_orientation(NETK_PAGER(pager), GTK_ORIENTATION_HORIZONTAL);
    netk_pager_set_n_rows(NETK_PAGER(pager), 1);
    gtk_widget_set_size_request (GTK_WIDGET (pager), -1, FIXED_HEIGHT);
    gtk_box_pack_end (GTK_BOX (hbox), pager, FALSE, FALSE, 0);

    gtk_widget_show(tasklist);
    gtk_widget_show(pager);
    gtk_widget_show(frame);
    gtk_widget_show(win);
    netk_set_desktop_margins(GDK_WINDOW_XWINDOW(win->window), &margins);

    gtk_main();

    return 0;
}
