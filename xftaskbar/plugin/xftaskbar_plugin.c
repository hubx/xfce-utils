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

 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#include <libxfce4mcs/mcs-common.h>
#include <libxfce4mcs/mcs-manager.h>
#include <xfce-mcs-manager/manager-plugin.h>
#include <libxfcegui4/libxfcegui4.h>
#include "inline-icon.h"

#define _(String) String

#define RCDIR    "settings"
#define CHANNEL  "taskbar"
#define RCFILE   "taskbar.xml"
#define PLUGIN_NAME "taskbar"

#define TOP TRUE
#define BOTTOM FALSE
#define DEFAULT_HEIGHT	30

#define DEFAULT_ICON_SIZE 48
#ifndef DATADIR
#define DATADIR "/usr/local/share/xfce4"
#endif

static void create_channel(McsPlugin * mcs_plugin);
static gboolean write_options(McsPlugin * mcs_plugin);
static void run_dialog(McsPlugin * mcs_plugin);

static gboolean is_running = FALSE;
static gboolean position = TOP;
static gboolean autohide = FALSE;
static gboolean show_pager = TRUE;
static gboolean all_tasks = FALSE;
static int height = DEFAULT_HEIGHT;

typedef struct _Itf Itf;
struct _Itf
{
    McsPlugin *mcs_plugin;

    GSList *pos_radiobutton_group;

    GtkWidget *xftaskbar_dialog;
    GtkWidget *alltasks_checkbutton;
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
    GtkWidget *label1;
    GtkWidget *label2;
    GtkWidget *label3;
    GtkWidget *label9;
    GtkWidget *label13;
    GtkWidget *label14;
    GtkWidget *label15;
    GtkWidget *label16;
    GtkWidget *pager_checkbutton;
    GtkWidget *pos_bottom_radiobutton;
    GtkWidget *pos_top_radiobutton;
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

static void cb_showpager_changed(GtkWidget * dialog, gpointer user_data)
{
    Itf *itf = (Itf *) user_data;
    McsPlugin *mcs_plugin = itf->mcs_plugin;

    show_pager = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(itf->pager_checkbutton));

    mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowPager", CHANNEL, show_pager ? 1 : 0);
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

Itf *create_xftaskbar_dialog(McsPlugin * mcs_plugin)
{
    Itf *dialog;
    GdkPixbuf *icon;

    dialog = g_new(Itf, 1);

    dialog->mcs_plugin = mcs_plugin;
    dialog->pos_radiobutton_group = NULL;

    dialog->xftaskbar_dialog = gtk_dialog_new();

    icon = inline_icon_at_size(default_icon_data, 32, 32);
    gtk_window_set_icon(GTK_WINDOW(dialog->xftaskbar_dialog), icon);
    g_object_unref(icon);

    gtk_window_set_title (GTK_WINDOW (dialog->xftaskbar_dialog), _("Taskbar"));
    gtk_window_set_position (GTK_WINDOW (dialog->xftaskbar_dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_set_has_separator (GTK_DIALOG (dialog->xftaskbar_dialog), FALSE);

    dialog->dialog_vbox1 = GTK_DIALOG (dialog->xftaskbar_dialog)->vbox;
    gtk_widget_show (dialog->dialog_vbox1);

    dialog->dialog_header = create_header(icon, _("Taskbar"));
    gtk_widget_show(dialog->dialog_header);
    gtk_box_pack_start(GTK_BOX(dialog->dialog_vbox1), dialog->dialog_header, FALSE, TRUE, 0);

    dialog->hbox1 = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (dialog->hbox1);
    gtk_box_pack_start (GTK_BOX (dialog->dialog_vbox1), dialog->hbox1, TRUE, TRUE, 0);

    dialog->vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (dialog->vbox1);
    gtk_box_pack_start (GTK_BOX (dialog->hbox1), dialog->vbox1, TRUE, TRUE, 0);

    dialog->frame1 = gtk_frame_new (NULL);
    gtk_widget_show (dialog->frame1);
    gtk_box_pack_start (GTK_BOX (dialog->vbox1), dialog->frame1, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->frame1), 3);

    dialog->hbox2 = gtk_hbox_new (TRUE, 0);
    gtk_widget_show (dialog->hbox2);
    gtk_container_add (GTK_CONTAINER (dialog->frame1), dialog->hbox2);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->hbox2), 5);

    dialog->pos_top_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Top"));
    gtk_widget_show (dialog->pos_top_radiobutton);
    gtk_box_pack_start (GTK_BOX (dialog->hbox2), dialog->pos_top_radiobutton, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (dialog->pos_top_radiobutton), dialog->pos_radiobutton_group);
    dialog->pos_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->pos_top_radiobutton));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->pos_top_radiobutton), position);

    dialog->pos_bottom_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Bottom"));
    gtk_widget_show (dialog->pos_bottom_radiobutton);
    gtk_box_pack_start (GTK_BOX (dialog->hbox2), dialog->pos_bottom_radiobutton, FALSE, FALSE, 0);
    gtk_radio_button_set_group (GTK_RADIO_BUTTON (dialog->pos_bottom_radiobutton), dialog->pos_radiobutton_group);
    dialog->pos_radiobutton_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (dialog->pos_bottom_radiobutton));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->pos_bottom_radiobutton), !position);

    dialog->label1 = gtk_label_new (_("Position"));
    gtk_widget_show (dialog->label1);
    gtk_frame_set_label_widget (GTK_FRAME (dialog->frame1), dialog->label1);
    gtk_label_set_justify (GTK_LABEL (dialog->label1), GTK_JUSTIFY_LEFT);

    dialog->frame5 = gtk_frame_new (NULL);
    gtk_widget_show (dialog->frame5);
    gtk_box_pack_start (GTK_BOX (dialog->vbox1), dialog->frame5, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->frame5), 3);

    dialog->alltasks_checkbutton = gtk_check_button_new_with_mnemonic (_("Show tasks from all workspaces"));
    gtk_widget_show (dialog->alltasks_checkbutton);
    gtk_container_add (GTK_CONTAINER (dialog->frame5), dialog->alltasks_checkbutton);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->alltasks_checkbutton), all_tasks);

    dialog->label3 = gtk_label_new (_("Tasks"));
    gtk_widget_show (dialog->label3);
    gtk_frame_set_label_widget (GTK_FRAME (dialog->frame5), dialog->label3);
    gtk_label_set_justify (GTK_LABEL (dialog->label3), GTK_JUSTIFY_LEFT);

    dialog->frame2 = gtk_frame_new (NULL);
    gtk_widget_show (dialog->frame2);
    gtk_box_pack_start (GTK_BOX (dialog->vbox1), dialog->frame2, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->frame2), 3);

    dialog->pager_checkbutton = gtk_check_button_new_with_mnemonic (_("Show pager in taskbar"));
    gtk_widget_show (dialog->pager_checkbutton);
    gtk_container_add (GTK_CONTAINER (dialog->frame2), dialog->pager_checkbutton);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->pager_checkbutton), show_pager);

    dialog->label2 = gtk_label_new (_("Pager"));
    gtk_widget_show (dialog->label2);
    gtk_frame_set_label_widget (GTK_FRAME (dialog->frame2), dialog->label2);
    gtk_label_set_justify (GTK_LABEL (dialog->label2), GTK_JUSTIFY_LEFT);

    dialog->vbox2 = gtk_vbox_new (TRUE, 0);
    gtk_widget_show (dialog->vbox2);
    gtk_box_pack_start (GTK_BOX (dialog->hbox1), dialog->vbox2, TRUE, TRUE, 0);

    dialog->frame3 = gtk_frame_new (NULL);
    gtk_widget_show (dialog->frame3);
    gtk_box_pack_start (GTK_BOX (dialog->vbox2), dialog->frame3, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->frame3), 3);

    dialog->autohide_checkbutton = gtk_check_button_new_with_mnemonic (_("Auto hide taskbar"));
    gtk_widget_show (dialog->autohide_checkbutton);
    gtk_container_add (GTK_CONTAINER (dialog->frame3), dialog->autohide_checkbutton);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->autohide_checkbutton), autohide);

    dialog->label9 = gtk_label_new (_("Autohide"));
    gtk_widget_show (dialog->label9);
    gtk_frame_set_label_widget (GTK_FRAME (dialog->frame3), dialog->label9);
    gtk_label_set_justify (GTK_LABEL (dialog->label9), GTK_JUSTIFY_LEFT);

    dialog->frame4 = gtk_frame_new (NULL);
    gtk_widget_show (dialog->frame4);
    gtk_box_pack_start (GTK_BOX (dialog->vbox2), dialog->frame4, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->frame4), 3);

    dialog->table3 = gtk_table_new (2, 3, FALSE);
    gtk_widget_show (dialog->table3);
    gtk_container_add (GTK_CONTAINER (dialog->frame4), dialog->table3);
    gtk_container_set_border_width (GTK_CONTAINER (dialog->table3), 5);

    dialog->label14 = gtk_label_new (_("Height :"));
    gtk_widget_show (dialog->label14);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->label14, 0, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (dialog->label14), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->label14), 0, 0.5);

    dialog->label15 = gtk_label_new (_("<small><i>Small</i></small>"));
    gtk_widget_show (dialog->label15);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->label15, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (dialog->label15), TRUE);
    gtk_label_set_justify (GTK_LABEL (dialog->label15), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->label15), 1, 0.5);

    dialog->label16 = gtk_label_new (_("<small><i>Large</i></small>"));
    gtk_widget_show (dialog->label16);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->label16, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_use_markup (GTK_LABEL (dialog->label16), TRUE);
    gtk_label_set_justify (GTK_LABEL (dialog->label16), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (dialog->label16), 0, 0.5);

    dialog->height_scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (height, 20, 60, 10, 10, 10)));
    gtk_widget_show (dialog->height_scale);
    gtk_table_attach (GTK_TABLE (dialog->table3), dialog->height_scale, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_scale_set_draw_value (GTK_SCALE (dialog->height_scale), FALSE);
    gtk_scale_set_digits (GTK_SCALE (dialog->height_scale), 0);
    gtk_range_set_update_policy (GTK_RANGE (dialog->height_scale), GTK_UPDATE_DISCONTINUOUS);

    dialog->label13 = gtk_label_new (_("Size"));
    gtk_widget_show (dialog->label13);
    gtk_frame_set_label_widget (GTK_FRAME (dialog->frame4), dialog->label13);
    gtk_label_set_justify (GTK_LABEL (dialog->label13), GTK_JUSTIFY_LEFT);

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
    g_signal_connect(G_OBJECT(itf->pager_checkbutton), "toggled", G_CALLBACK(cb_showpager_changed), itf);
    g_signal_connect(G_OBJECT(itf->alltasks_checkbutton), "toggled", G_CALLBACK(cb_alltasks_changed), itf);
    g_signal_connect(G_OBJECT(itf->autohide_checkbutton), "toggled", G_CALLBACK(cb_autohide_changed), itf);
    g_signal_connect(G_OBJECT(itf->height_scale), "value_changed", G_CALLBACK(cb_height_changed), itf);

    gtk_widget_show(itf->xftaskbar_dialog);
}

McsPluginInitResult mcs_plugin_init(McsPlugin * mcs_plugin)
{
    create_channel(mcs_plugin);
    mcs_plugin->plugin_name = g_strdup(PLUGIN_NAME);
    mcs_plugin->caption = g_strdup(_("Taskbar"));
    mcs_plugin->run_dialog = run_dialog;
    mcs_plugin->icon = inline_icon_at_size(default_icon_data, DEFAULT_ICON_SIZE, DEFAULT_ICON_SIZE);
    mcs_manager_notify(mcs_plugin->manager, CHANNEL);

    return (MCS_PLUGIN_INIT_OK);
}

static void create_channel(McsPlugin * mcs_plugin)
{
    McsSetting *setting;

    const gchar *home = g_getenv("HOME");
    gchar *rcfile;
    
    rcfile = g_strconcat(home, G_DIR_SEPARATOR_S, ".xfce4", G_DIR_SEPARATOR_S, RCDIR, G_DIR_SEPARATOR_S, RCFILE, NULL);
    mcs_manager_add_channel_from_file(mcs_plugin->manager, CHANNEL, rcfile);
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
        show_pager = TRUE;
        mcs_manager_set_int(mcs_plugin->manager, "Taskbar/ShowPager", CHANNEL, show_pager ? 1 : 0);
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
}

static gboolean write_options(McsPlugin * mcs_plugin)
{
    const gchar *home = g_getenv("HOME");
    gchar *rcfile;
    gboolean result;

    rcfile = g_strconcat(home, G_DIR_SEPARATOR_S, ".xfce4", G_DIR_SEPARATOR_S, RCDIR, G_DIR_SEPARATOR_S, RCFILE, NULL);
    result = mcs_manager_save_channel_to_file(mcs_plugin->manager, CHANNEL, rcfile);
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
