/*
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; You may only use version 2 of the License,
	you have no option to use any other version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

        xfce4 mcs plugin   - (c) 2002 Olivier Fourdan
                             (c) 2004 Benedikt Meurer

 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#include <libxfce4mcs/mcs-common.h>
#include <libxfce4mcs/mcs-manager.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>
#include <xfce-mcs-manager/manager-plugin.h>

#define BORDER 5

#define RCDIR    "mcs_settings"
#define OLDRCDIR "settings"
#define CHANNEL  "taskbar"
#define RCFILE   "taskbar.xml"
#define PLUGIN_NAME "taskbar"

#define TOP TRUE
#define BOTTOM FALSE
#define DEFAULT_HEIGHT	30
#define DEFAULT_WIDTH_PERCENT 100
#define DEFAULT_HORIZ_ALIGN 0

#define DEFAULT_ICON_SIZE 48

static void create_channel(McsPlugin * mcs_plugin);
static gboolean write_options(McsPlugin * mcs_plugin);
static void run_dialog(McsPlugin * mcs_plugin);

static gboolean is_running = FALSE;
static gboolean position = TOP;
static gboolean autohide = FALSE;
static gboolean show_pager = TRUE;
static gboolean show_tray = TRUE;
static gboolean all_tasks = FALSE;
static gboolean group_tasks = FALSE;
static gboolean show_text = TRUE;
static int height = DEFAULT_HEIGHT;
static int width_percent = DEFAULT_WIDTH_PERCENT;
static int horiz_align = DEFAULT_HORIZ_ALIGN;

typedef struct _Itf Itf;
struct _Itf
{
    McsPlugin *mcs_plugin;

    GSList *pos_radiobutton_group;

    GtkWidget *xftaskbar_dialog;
    GtkWidget *tasks_vbox;
    GtkWidget *alltasks_checkbutton;
    GtkWidget *grouptasks_checkbutton;
    GtkWidget *showtext_checkbutton;
    GtkWidget *autohide_checkbutton;
    GtkWidget *dialog_action_area1;
    GtkWidget *dialog_header;
    GtkWidget *dialog_vbox1;
    GtkWidget *frame1;
    GtkWidget *frame2;
    GtkWidget *frame3;
    GtkWidget *frame4;
    GtkWidget *frame5;
    GtkWidget *hbox1;
    GtkWidget *hbox2;
    GtkWidget *height_scale;
    GtkWidget *label14;
    GtkWidget *label15;
    GtkWidget *label16;
    GtkWidget *width_scale;
    GtkWidget *width_label_top;
    GtkWidget *width_label_left;
    GtkWidget *width_label_right;
    GtkWidget *pager_checkbutton;
    GtkWidget *tray_checkbutton;
    GtkWidget *pos_bottom_radiobutton;
    GtkWidget *pos_top_radiobutton;

    GSList *align_button_group;
    GtkWidget *align_frame;
    GtkWidget *align_hbox;
    GtkWidget *align_left_button;
    GtkWidget *align_center_button;
    GtkWidget *align_right_button;

    GtkWidget *table3;
    GtkWidget *vbox1;
    GtkWidget *vbox2;
    GtkWidget *closebutton;
};

static void cb_dialog_response(GtkWidget * dialog, gint response_id)
{
    if(response_id == GTK_RESPONSE_HELP)
    {
        g_message("HELP: TBD");
    }
    else
    {
        is_running = FALSE;
        gtk_widget_destroy(dialog);
    }
}

static void cb_position_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    position = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->pos_top_radiobutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/Position", CHANNEL, position ? 1 : 0);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_align_changed(GtkWidget *dialog, gpointer user_data)
{
  Itf *itf = (Itf *)user_data;
  McsPlugin *mcs_plugin = itf->mcs_plugin;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->align_left_button)))
    horiz_align = -1;
  else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->align_center_button)))
    horiz_align = 0;
  else
    horiz_align = 1;

  mcs_manager_set_int(mcs_plugin->manager, "Taskbar/HorizAlign", CHANNEL, horiz_align);
  mcs_manager_notify(mcs_plugin->manager, CHANNEL);
  write_options(mcs_plugin);
}

static void cb_showpager_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    show_pager = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->pager_checkbutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowPager", CHANNEL, show_pager ? 1 : 0);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_showtray_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    show_tray = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->tray_checkbutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowTray", CHANNEL, show_tray ? 1 : 0);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_alltasks_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    all_tasks = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->alltasks_checkbutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowAllTasks", CHANNEL, all_tasks ? 1 : 0);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_grouptasks_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    group_tasks = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->grouptasks_checkbutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/GroupTasks", CHANNEL, group_tasks ? 1 : 0);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_showtext_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    show_text = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->showtext_checkbutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowText", CHANNEL, show_text ? 1 : 0);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_autohide_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    autohide = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->autohide_checkbutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/AutoHide", CHANNEL, autohide ? 1 : 0);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_height_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    height = (int)gtk_range_get_value(GTK_RANGE(itf->height_scale));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/Height", CHANNEL, height);

    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

static void cb_width_percent_changed(GtkWidget *dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    width_percent = (int)gtk_range_get_value(GTK_RANGE(itf->width_scale));

    gtk_widget_set_sensitive (itf->align_frame, (width_percent < 100));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/WidthPercent",
            CHANNEL, width_percent);

    mcs_manager_notify(mcs_plugin->manager, CHANNEL);
    write_options(mcs_plugin);
}

Itf *create_xftaskbar_dialog(McsPlugin * mcs_plugin)
{
    Itf *dialog;

    dialog = g_new(Itf, 1);

    dialog->mcs_plugin = mcs_plugin;
    dialog->pos_radiobutton_group = NULL;
    dialog->align_button_group = NULL;

    dialog->xftaskbar_dialog = gtk_dialog_new();

    gtk_window_set_icon(GTK_WINDOW(dialog->xftaskbar_dialog), mcs_plugin->icon);

    gtk_window_set_title (GTK_WINDOW (dialog->xftaskbar_dialog), _("Taskbar"));
    gtk_dialog_set_has_separator (GTK_DIALOG (dialog->xftaskbar_dialog), FALSE);

    dialog->dialog_vbox1 = GTK_DIALOG (dialog->xftaskbar_dialog)->vbox;
    gtk_widget_show (dialog->dialog_vbox1);

    dialog->dialog_header = xfce_create_header(mcs_plugin->icon,
                                               _("Taskbar"));
    gtk_widget_show(dialog->dialog_header);
    gtk_box_pack_start(GTK_BOX(dialog->dialog_vbox1), dialog->dialog_header, FALSE, TRUE, 0);

    dialog->hbox1 = gtk_hbox_new (TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->hbox1), BORDER + 1);
    gtk_widget_show (dialog->hbox1);
    gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->hbox1, TRUE, TRUE, 0);

    dialog->vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox1), BORDER);
    gtk_widget_show (dialog->vbox1);
    gtk_box_pack_start (GTK_BOX (dialog->hbox1), dialog->vbox1, TRUE, TRUE, 0);

    dialog->frame1 = xfce_framebox_new (_("Position"), TRUE);
    gtk_widget_show (dialog->frame1);
    gtk_box_pack_start (GTK_BOX (dialog->vbox1), dialog->frame1, TRUE, TRUE, 0);

    dialog->hbox2 = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (dialog->hbox2);
    xfce_framebox_add (XFCE_FRAMEBOX (dialog->frame1), dialog->hbox2);

    dialog->pos_top_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("_Top"));
    gtk_widget_show (dialog->pos_top_radiobutton);
    gtk_box_pack_start (GTK_BOX (dialog->hbox2), dialog->pos_top_radiobutton, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (dialog->pos_top_radiobutton), dialog->pos_radiobutton_group);
    dialog->pos_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->pos_top_radiobutton));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->pos_top_radiobutton), position);

    dialog->pos_bottom_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("_Bottom"));
    gtk_widget_show (dialog->pos_bottom_radiobutton);
    gtk_box_pack_start (GTK_BOX (dialog->hbox2), dialog->pos_bottom_radiobutton, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (dialog->pos_bottom_radiobutton), dialog->pos_radiobutton_group);
    dialog->pos_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->pos_bottom_radiobutton));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->pos_bottom_radiobutton), !position);

    dialog->align_frame = xfce_framebox_new (_("Alignment"), TRUE);
    gtk_widget_set_sensitive (dialog->align_frame, (width_percent < 100));
    gtk_widget_show (dialog->align_frame);
    gtk_box_pack_start (GTK_BOX (dialog->vbox1), dialog->align_frame, TRUE, TRUE, 0);

    dialog->align_hbox = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (dialog->align_hbox);
    xfce_framebox_add (XFCE_FRAMEBOX (dialog->align_frame), dialog->align_hbox);

    dialog->align_left_button = gtk_radio_button_new_with_mnemonic (NULL, _("_Left"));
    gtk_widget_show (dialog->align_left_button);
    gtk_box_pack_start (GTK_BOX (dialog->align_hbox), dialog->align_left_button, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (dialog->align_left_button), dialog->align_button_group);
    dialog->align_button_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->align_left_button));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->align_left_button), horiz_align < 0);

    dialog->align_center_button = gtk_radio_button_new_with_mnemonic (NULL, _("_Center"));
    gtk_widget_show (dialog->align_center_button);
    gtk_box_pack_start (GTK_BOX (dialog->align_hbox), dialog->align_center_button, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (dialog->align_center_button), dialog->align_button_group);
    dialog->align_button_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->align_center_button));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->align_center_button), horiz_align == 0);

    dialog->align_right_button = gtk_radio_button_new_with_mnemonic (NULL, _("_Right"));
    gtk_widget_show (dialog->align_right_button);
    gtk_box_pack_start (GTK_BOX (dialog->align_hbox), dialog->align_right_button, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (dialog->align_right_button), dialog->align_button_group);
    dialog->align_button_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->align_right_button));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->align_right_button), horiz_align > 0);

    dialog->frame3 = xfce_framebox_new (_("Autohide"), TRUE);
    gtk_widget_show (dialog->frame3);
    gtk_box_pack_start (GTK_BOX (dialog->vbox1), dialog->frame3, TRUE, TRUE, 0);

    dialog->autohide_checkbutton = gtk_check_button_new_with_mnemonic (_("Auto _hide taskbar"));
    gtk_widget_show (dialog->autohide_checkbutton);
    xfce_framebox_add (XFCE_FRAMEBOX (dialog->frame3), dialog->autohide_checkbutton);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->autohide_checkbutton), autohide);

    dialog->frame4 = xfce_framebox_new (_("Size"), TRUE);
    gtk_widget_show (dialog->frame4);
    gtk_box_pack_start (GTK_BOX (dialog->vbox1), dialog->frame4, TRUE, TRUE, 0);

    dialog->table3 = gtk_table_new (2, 3, FALSE);
    gtk_widget_show (dialog->table3);
    xfce_framebox_add (XFCE_FRAMEBOX (dialog->frame4), dialog->table3);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->table3), 5);

    dialog->label14 = gtk_label_new (_("Height :"));
    gtk_widget_show (dialog->label14);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->label14, 0, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (dialog->label14), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->label14), 0, 0.5);

    dialog->label15 = xfce_create_small_label(_("Small"));
    gtk_widget_show (dialog->label15);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->label15, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (dialog->label15), TRUE);
    gtk_label_set_justify (GTK_LABEL (dialog->label15), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->label15), 1, 0.5);

    dialog->label16 = xfce_create_small_label(_("Large"));
    gtk_widget_show (dialog->label16);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->label16, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (dialog->label16), TRUE);
    gtk_label_set_justify (GTK_LABEL (dialog->label16), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->label16), 0, 0.5);

    dialog->height_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (height, 28, 60, 10, 10, 10)));
    gtk_widget_show (dialog->height_scale);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->height_scale, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (dialog->height_scale), FALSE);
    gtk_scale_set_digits (GTK_SCALE (dialog->height_scale), 0);
    gtk_range_set_update_policy (GTK_RANGE (dialog->height_scale), GTK_UPDATE_DISCONTINUOUS);

    dialog->width_label_top = gtk_label_new (_("Width:"));
    gtk_widget_show (dialog->width_label_top);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->width_label_top,
                      0, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (dialog->width_label_top),
                           GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->width_label_top), 0, 0.5);

    dialog->width_label_left = xfce_create_small_label(_("Small"));
    gtk_widget_show (dialog->width_label_left);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->width_label_left,
                      0, 1, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (dialog->width_label_left), TRUE);
    gtk_label_set_justify (GTK_LABEL (dialog->width_label_left),
                           GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->width_label_left), 1, 0.5);

    dialog->width_label_right = xfce_create_small_label(_("Large"));
    gtk_widget_show (dialog->width_label_right);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->width_label_right,
                      2, 3, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (dialog->width_label_right), TRUE);
    gtk_label_set_justify (GTK_LABEL (dialog->width_label_right),
                           GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->width_label_right), 0, 0.5);

    dialog->width_scale = gtk_hscale_new (GTK_ADJUSTMENT (
                gtk_adjustment_new (width_percent, 20, 110, 10, 10, 10)));
    gtk_widget_show (dialog->width_scale);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->width_scale,
                      1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (dialog->width_scale), FALSE);
    gtk_scale_set_digits (GTK_SCALE (dialog->width_scale), 0);
    gtk_range_set_update_policy (GTK_RANGE (dialog->width_scale),
                                 GTK_UPDATE_DISCONTINUOUS);

    dialog->vbox2 = gtk_vbox_new (TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox2), BORDER);
    gtk_widget_show (dialog->vbox2);
    gtk_box_pack_start (GTK_BOX (dialog->hbox1), dialog->vbox2, TRUE, TRUE, 0);

    dialog->frame5 = xfce_framebox_new (_("Tasks"), TRUE);
    gtk_widget_show (dialog->frame5);
    gtk_box_pack_start (GTK_BOX (dialog->vbox2), dialog->frame5, TRUE, TRUE, 0);

    dialog->tasks_vbox = gtk_vbox_new (TRUE, 0);
    gtk_widget_show (dialog->tasks_vbox);
    xfce_framebox_add (XFCE_FRAMEBOX (dialog->frame5), dialog->tasks_vbox);

    dialog->alltasks_checkbutton = gtk_check_button_new_with_mnemonic (_("Show tasks from _all workspaces"));
    gtk_widget_show (dialog->alltasks_checkbutton);
    gtk_box_pack_start (GTK_BOX (dialog->tasks_vbox), dialog->alltasks_checkbutton, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->alltasks_checkbutton), all_tasks);

    dialog->grouptasks_checkbutton = gtk_check_button_new_with_mnemonic (_("Always _group tasks"));
    gtk_widget_show (dialog->grouptasks_checkbutton);
    gtk_box_pack_start (GTK_BOX (dialog->tasks_vbox), dialog->grouptasks_checkbutton, FALSE, FALSE, 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->grouptasks_checkbutton), group_tasks);

    dialog->showtext_checkbutton = gtk_check_button_new_with_mnemonic (_("Show application _names"));
    gtk_widget_show (dialog->showtext_checkbutton);
    gtk_box_pack_start (GTK_BOX (dialog->tasks_vbox), dialog->showtext_checkbutton, FALSE, FALSE, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->showtext_checkbutton), show_text);

    dialog->frame2 = xfce_framebox_new (_("Pager"), TRUE);
    gtk_widget_show (dialog->frame2);
    gtk_box_pack_start (GTK_BOX (dialog->vbox2), dialog->frame2, TRUE, TRUE, 0);

    dialog->pager_checkbutton = gtk_check_button_new_with_mnemonic (_("Show _pager in taskbar"));
    gtk_widget_show (dialog->pager_checkbutton);
    xfce_framebox_add (XFCE_FRAMEBOX (dialog->frame2), dialog->pager_checkbutton);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->pager_checkbutton), show_pager);

    dialog->frame2 = xfce_framebox_new (_("Notification area"), TRUE);
    gtk_widget_show (dialog->frame2);
    gtk_box_pack_start (GTK_BOX (dialog->vbox2), dialog->frame2, TRUE, TRUE, 0);

    dialog->tray_checkbutton = gtk_check_button_new_with_mnemonic (_("Show _system tray in taskbar"));
    gtk_widget_show (dialog->tray_checkbutton);
    xfce_framebox_add (XFCE_FRAMEBOX (dialog->frame2), dialog->tray_checkbutton);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->tray_checkbutton), show_tray);

    dialog->dialog_action_area1 = GTK_DIALOG (dialog->xftaskbar_dialog)->action_area;
    gtk_widget_show (dialog->dialog_action_area1);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->dialog_action_area1), GTK_BUTTONBOX_END);

    dialog->closebutton = gtk_button_new_from_stock ("gtk-close");
    gtk_widget_show (dialog->closebutton);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog->xftaskbar_dialog), dialog->closebutton, GTK_RESPONSE_CLOSE);
    GTK_WIDGET_SET_FLAGS (dialog->closebutton, GTK_CAN_DEFAULT);

    return dialog;
}

static void setup_dialog(Itf * itf)
{
    g_signal_connect(G_OBJECT(itf->xftaskbar_dialog), "response", G_CALLBACK(cb_dialog_response), itf->mcs_plugin);

    g_signal_connect(G_OBJECT(itf->pos_top_radiobutton), "toggled", G_CALLBACK(cb_position_changed), itf);
    g_signal_connect(G_OBJECT(itf->align_left_button), "toggled", G_CALLBACK(cb_align_changed), itf);
    g_signal_connect(G_OBJECT(itf->align_center_button), "toggled", G_CALLBACK(cb_align_changed), itf);
    g_signal_connect(G_OBJECT(itf->align_right_button), "toggled", G_CALLBACK(cb_align_changed), itf);
    g_signal_connect(G_OBJECT(itf->pager_checkbutton), "toggled", G_CALLBACK(cb_showpager_changed), itf);
    g_signal_connect(G_OBJECT(itf->tray_checkbutton), "toggled", G_CALLBACK(cb_showtray_changed), itf);
    g_signal_connect(G_OBJECT(itf->alltasks_checkbutton), "toggled", G_CALLBACK(cb_alltasks_changed), itf);
    g_signal_connect(G_OBJECT(itf->grouptasks_checkbutton), "toggled", G_CALLBACK(cb_grouptasks_changed), itf);
    g_signal_connect(G_OBJECT(itf->showtext_checkbutton), "toggled", G_CALLBACK(cb_showtext_changed), itf);
    g_signal_connect(G_OBJECT(itf->autohide_checkbutton), "toggled", G_CALLBACK(cb_autohide_changed), itf);
    g_signal_connect(G_OBJECT(itf->height_scale), "value_changed", G_CALLBACK(cb_height_changed), itf);
    g_signal_connect(G_OBJECT(itf->width_scale), "value_changed", G_CALLBACK(cb_width_percent_changed), itf);

    gtk_window_set_position (GTK_WINDOW (itf->xftaskbar_dialog), GTK_WIN_POS_CENTER);
    gtk_widget_show(itf->xftaskbar_dialog);
}

McsPluginInitResult mcs_plugin_init(McsPlugin * mcs_plugin)
{
    /* This is required for UTF-8 at least - Please don't remove it */
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

    create_channel(mcs_plugin);
    mcs_plugin->plugin_name = g_strdup(PLUGIN_NAME);
    mcs_plugin->caption = g_strdup(_("Taskbar"));
    mcs_plugin->run_dialog = run_dialog;
    mcs_plugin->icon = xfce_themed_icon_load ("xfce4-taskbar", 48);

    mcs_manager_notify (mcs_plugin->manager, CHANNEL);

    return (MCS_PLUGIN_INIT_OK);
}

static void create_channel(McsPlugin * mcs_plugin)
{
    McsSetting *setting;
    gchar *rcfile, *path;
    
    path = g_build_filename ("xfce4", RCDIR, RCFILE, NULL);
    rcfile = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, path);
    g_free (path);

    if (!rcfile)
        rcfile = xfce_get_userfile(OLDRCDIR, RCFILE, NULL);

    if (g_file_test (rcfile, G_FILE_TEST_EXISTS))
        mcs_manager_add_channel_from_file(mcs_plugin->manager, CHANNEL, rcfile);
    else
        mcs_manager_add_channel (mcs_plugin->manager, CHANNEL);
    
    g_free(rcfile);

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/Position", CHANNEL);
    if(setting)
    {
        position = setting->data.v_int ? TOP : BOTTOM;
    }
    else
    {
        position = TOP;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/Position", CHANNEL, position ? 1 : 0);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/HorizAlign", CHANNEL);
    if(setting)
    {
        horiz_align = setting->data.v_int;
    }
    else
    {
        horiz_align = DEFAULT_HORIZ_ALIGN;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/HorizAlign", CHANNEL, horiz_align);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/AutoHide", CHANNEL);
    if(setting)
    {
        autohide = (setting->data.v_int ? TRUE : FALSE);
    }
    else
    {
        autohide = FALSE;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/AutoHide", CHANNEL, autohide ? 1 : 0);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/ShowPager", CHANNEL);
    if(setting)
    {
        show_pager = (setting->data.v_int ? TRUE : FALSE);
    }
    else
    {
        show_pager = FALSE;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowPager", CHANNEL, show_pager ? 1 : 0);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/ShowTray", CHANNEL);
    if(setting)
    {
        show_tray = (setting->data.v_int ? TRUE : FALSE);
    }
    else
    {
        show_tray = TRUE;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowTray", CHANNEL, show_tray ? 1 : 0);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/ShowAllTasks", CHANNEL);
    if(setting)
    {
        all_tasks = (setting->data.v_int ? TRUE : FALSE);
    }
    else
    {
        all_tasks = FALSE;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowAllTasks", CHANNEL, all_tasks ? 1 : 0);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/GroupTasks", CHANNEL);
    if(setting)
    {
        group_tasks = (setting->data.v_int ? TRUE : FALSE);
    }
    else
    {
        group_tasks = FALSE;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/GroupTasks", CHANNEL, group_tasks ? 1 : 0);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/Height", CHANNEL);
    if(setting)
    {
        height = setting->data.v_int;
    }
    else
    {
        height = DEFAULT_HEIGHT;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/Height", CHANNEL, height);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/WidthPercent", CHANNEL);
    if(setting)
    {
        width_percent = setting->data.v_int;
    }
    else
    {
        width_percent = DEFAULT_WIDTH_PERCENT;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/WidthPercent", CHANNEL, width_percent);
    }

    setting = mcs_manager_setting_lookup(mcs_plugin->manager, "Taskbar/ShowText", CHANNEL);
    if(setting)
    {
        show_text = setting->data.v_int == 0 ? FALSE : TRUE;
    }
    else
    {
        show_text = TRUE;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowText", CHANNEL, show_text ? 1 : 0);
    }
}

static gboolean write_options(McsPlugin * mcs_plugin)
{
    gchar *rcfile, *path;
    gboolean result;

    path = g_build_filename ("xfce4", RCDIR, RCFILE, NULL);
    rcfile = xfce_resource_save_location (XFCE_RESOURCE_CONFIG, path, TRUE);
    result = mcs_manager_save_channel_to_file(mcs_plugin->manager, CHANNEL, rcfile);
    g_free(path);
    g_free(rcfile);

    return result;
}

static void run_dialog(McsPlugin * mcs_plugin)
{
    Itf *dialog;

    if(is_running)
        return;

    is_running = TRUE;

    dialog = create_xftaskbar_dialog(mcs_plugin);
    setup_dialog(dialog);
}

/* macro defined in manager-plugin.h */
MCS_PLUGIN_CHECK_INIT
