/*
 * xfrun - a simple quick run dialog with saved history and completion
 *
 * Copyright (c) 2006 Brian J. Tarricone <bjt23@cornell.edu>
 * Copyright (c) 2008 Stefan Stuhr <xfce4devlist@sstuhr.dk>
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

#include <stdio.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include "xfrun-dialog.h"

#define BORDER       8
#define MAX_ENTRIES 20

struct _XfrunDialogPrivate
{
    GtkWidget *comboboxentry;
    GtkWidget *entry;
    GtkWidget *terminal_chk;
    GtkTreeModel *completion_model;

    gchar *run_argument;
    gboolean destroy_on_close;
    gchar *working_directory;

    gchar *entry_val_tmp;
};

enum
{
    PROP0,
    PROP_RUN_ARGUMENT,
    PROP_WORKING_DIRECTORY,
};

enum
{
    SIG_CLOSED,
    N_SIGS,
};

enum
{
    XFRUN_COL_COMMAND = 0,
    XFRUN_COL_IN_TERMINAL,
    XFRUN_N_COLS,
};


static void xfrun_dialog_set_property(GObject *object,
                                      guint property_id,
                                      const GValue *value,
                                      GParamSpec *pspec);
static void xfrun_dialog_get_property(GObject *object,
                                      guint property_id,
                                      GValue *value,
                                      GParamSpec *pspec);
static void xfrun_dialog_finalize(GObject *object);

static gboolean xfrun_dialog_key_press_event(GtkWidget *widget,
                                             GdkEventKey *evt);
static gboolean xfrun_dialog_delete_event(GtkWidget *widget,
                                          GdkEventAny *evt);

static gboolean xfrun_comboboxentry_changed(GtkComboBoxEntry *comboboxentry,
                                            gpointer user_data);
static gboolean xfrun_entry_check_match(GtkTreeModel *model,
                                        GtkTreePath *path,
                                        GtkTreeIter *iter,
                                        gpointer data);
static gboolean xfrun_entry_focus_out(GtkWidget *widget,
                                      GdkEventFocus *evt,
                                      gpointer user_data);
static void xfrun_run_clicked(GtkWidget *widget,
                              gpointer user_data);
static gboolean xfrun_match_selected(GtkEntryCompletion *completion,
                                     GtkTreeModel *model,
                                     GtkTreeIter *iter,
                                     gpointer user_data);
static void xfrun_setup_entry_completion(XfrunDialog *dialog);
static GtkTreeModel *xfrun_create_completion_model(XfrunDialog *dialog);

guint __signals[N_SIGS] = { 0, };


G_DEFINE_TYPE(XfrunDialog, xfrun_dialog, GTK_TYPE_WINDOW)


static void
xfrun_dialog_class_init(XfrunDialogClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

    g_type_class_add_private(klass, sizeof(XfrunDialogPrivate));

    gobject_class->set_property = xfrun_dialog_set_property;
    gobject_class->get_property = xfrun_dialog_get_property;
    gobject_class->finalize = xfrun_dialog_finalize;

    widget_class->key_press_event = xfrun_dialog_key_press_event;
    widget_class->delete_event = xfrun_dialog_delete_event;

    __signals[SIG_CLOSED] = g_signal_new("closed", XFRUN_TYPE_DIALOG,
                                         G_SIGNAL_RUN_LAST,
                                         G_STRUCT_OFFSET(XfrunDialogClass,
                                                         closed),
                                         NULL, NULL,
                                         g_cclosure_marshal_VOID__VOID,
                                         G_TYPE_NONE, 0);

    g_object_class_install_property(gobject_class, PROP_RUN_ARGUMENT,
                                    g_param_spec_string("run-argument",
                                                        "Run argument",
                                                        "1st argument to pass to the program run",
                                                        NULL,
                                                        G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, PROP_WORKING_DIRECTORY,
                                    g_param_spec_string("working-directory",
                                                        "Working directory",
                                                        "Directory to chdir() to before running the command",
                                                        NULL,
                                                        G_PARAM_READWRITE));
}

static void
xfrun_dialog_init(XfrunDialog *dialog)
{
    GtkWidget *entry, *comboboxentry, *chk, *btn, *vbox, *bbox;
    GtkTreeIter itr;

    dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE(dialog, XFRUN_TYPE_DIALOG,
                                               XfrunDialogPrivate);
    GTK_WINDOW(dialog)->type = GTK_WINDOW_TOPLEVEL;

    gtk_widget_set_size_request(GTK_WIDGET(dialog), 400, -1);

    vbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), BORDER);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(dialog), vbox);

    dialog->priv->comboboxentry = comboboxentry = gtk_combo_box_entry_new();
    dialog->priv->entry = entry = gtk_bin_get_child(GTK_BIN(comboboxentry));
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    xfrun_setup_entry_completion(dialog);
    gtk_widget_show(comboboxentry);
    gtk_box_pack_start(GTK_BOX(vbox), comboboxentry, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(comboboxentry), "changed",
                     G_CALLBACK(xfrun_comboboxentry_changed), dialog);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
                     G_CALLBACK(xfrun_entry_focus_out), dialog);

    dialog->priv->terminal_chk = chk = gtk_check_button_new_with_mnemonic(_("Run in _terminal"));
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(vbox), chk, FALSE, FALSE, 0);

    if(gtk_tree_model_get_iter_first(dialog->priv->completion_model, &itr)) {
        gboolean in_terminal = FALSE;

        gtk_tree_model_get (dialog->priv->completion_model, &itr,
                            XFRUN_COL_IN_TERMINAL, &in_terminal, -1);

        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(comboboxentry), &itr);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), in_terminal);
    }

    bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(bbox), BORDER);
    gtk_widget_show(bbox);
    gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

    btn = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_widget_show(btn);
    gtk_box_pack_end(GTK_BOX(bbox), btn, FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(btn), "clicked",
                             G_CALLBACK(xfrun_dialog_delete_event), dialog);

    btn = xfce_gtk_button_new_mixed(GTK_STOCK_EXECUTE, _("_Run"));
    gtk_widget_show(btn);
    gtk_box_pack_end(GTK_BOX(bbox), btn, FALSE, FALSE, 0);
    GTK_WIDGET_SET_FLAGS(btn, GTK_CAN_DEFAULT);
    gtk_widget_grab_default(btn);
    g_signal_connect(G_OBJECT(btn), "clicked",
                     G_CALLBACK(xfrun_run_clicked), dialog);
}

static void
xfrun_dialog_set_property(GObject *object,
                          guint property_id,
                          const GValue *value,
                          GParamSpec *pspec)
{
    XfrunDialog *dialog = XFRUN_DIALOG(object);

    switch(property_id) {
        case PROP_RUN_ARGUMENT:
            xfrun_dialog_set_run_argument(dialog, g_value_get_string(value));
            break;

        case PROP_WORKING_DIRECTORY:
            xfrun_dialog_set_working_directory(dialog,
                                               g_value_get_string(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
xfrun_dialog_get_property(GObject *object,
                          guint property_id,
                          GValue *value,
                          GParamSpec *pspec)
{
    XfrunDialog *dialog = XFRUN_DIALOG(object);

    switch(property_id) {
        case PROP_RUN_ARGUMENT:
            g_value_set_string(value, xfrun_dialog_get_run_argument(dialog));
            break;

        case PROP_WORKING_DIRECTORY:
            g_value_set_string(value,
                               xfrun_dialog_get_working_directory(dialog));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
xfrun_dialog_finalize(GObject *object)
{
    XfrunDialog *dialog = XFRUN_DIALOG(object);

    g_free(dialog->priv->run_argument);
    g_free(dialog->priv->working_directory);
    g_free(dialog->priv->entry_val_tmp);

    if(dialog->priv->completion_model)
        g_object_unref(G_OBJECT(dialog->priv->completion_model));

    G_OBJECT_CLASS(xfrun_dialog_parent_class)->finalize(object);
}

static gboolean
xfrun_dialog_key_press_event(GtkWidget *widget,
                             GdkEventKey *evt)
{
    if(evt->keyval == GDK_Escape) {
        xfrun_dialog_delete_event(widget, NULL);
        return TRUE;
    }

    return GTK_WIDGET_CLASS(xfrun_dialog_parent_class)->key_press_event(widget, evt);
}

static gboolean
xfrun_dialog_delete_event(GtkWidget *widget,
                          GdkEventAny *evt)
{
    XfrunDialog *dialog = XFRUN_DIALOG(widget);

    g_signal_emit(G_OBJECT(widget), __signals[SIG_CLOSED], 0);

    if(dialog->priv->destroy_on_close)
        gtk_widget_destroy(widget);
    else {
        /* assume we're going to use this again */
        xfrun_setup_entry_completion(dialog);
        gtk_editable_select_region(GTK_EDITABLE(dialog->priv->entry), 0, -1);
        gtk_widget_grab_focus(dialog->priv->entry);
        gtk_widget_hide(widget);
    }

    return TRUE;
}

static void
xfrun_setup_entry_completion(XfrunDialog *dialog)
{
    GtkComboBoxEntry *comboboxentry = GTK_COMBO_BOX_ENTRY(dialog->priv->comboboxentry);
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    GtkTreeModel *completion_model;

    /* clear out the old completion and resources, if any */
    gtk_entry_set_completion(GTK_ENTRY(dialog->priv->entry), NULL);
    gtk_combo_box_set_model(GTK_COMBO_BOX(comboboxentry), NULL);
    if(dialog->priv->completion_model)
        g_object_unref(dialog->priv->completion_model);

    dialog->priv->completion_model = completion_model = xfrun_create_completion_model(dialog);
    gtk_combo_box_set_model(GTK_COMBO_BOX(comboboxentry), completion_model);
    gtk_combo_box_entry_set_text_column(comboboxentry, XFRUN_COL_COMMAND);

    gtk_entry_completion_set_model(completion, completion_model);
    gtk_entry_completion_set_text_column(completion, XFRUN_COL_COMMAND);
    gtk_entry_completion_set_popup_completion(completion, TRUE);
    gtk_entry_completion_set_inline_completion(completion, TRUE);
    g_signal_connect(G_OBJECT(completion), "match-selected",
                     G_CALLBACK(xfrun_match_selected), dialog);

    gtk_entry_set_completion(GTK_ENTRY(dialog->priv->entry), completion);
    g_object_unref(G_OBJECT(completion));
}

static gchar **
xfrun_get_histfile_content(void)
{
    gchar **lines = NULL, *histfile, *contents = NULL;
    gsize length = 0;

    histfile = xfce_resource_lookup(XFCE_RESOURCE_CACHE, "xfce4/xfrun4/history");
    if(histfile && g_file_get_contents(histfile, &contents, &length, NULL)) {
        lines = g_strsplit(contents, "\n", -1);
        g_free(contents);
    }

    g_free(histfile);

    return lines;
}

static gboolean
xfrun_comboboxentry_changed(GtkComboBoxEntry *comboboxentry,
                            gpointer user_data)
{
    XfrunDialog *dialog = XFRUN_DIALOG(user_data);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean in_terminal = FALSE;

    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(comboboxentry), &iter)) {
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboboxentry));
        gtk_tree_model_get(model, &iter,
                           XFRUN_COL_IN_TERMINAL, &in_terminal,
                           -1);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->priv->terminal_chk),
                                     in_terminal);
    }

    return FALSE;
}

static gboolean
xfrun_entry_check_match(GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data)
{
    XfrunDialog *dialog = XFRUN_DIALOG(data);
    gchar *command = NULL;
    gboolean in_terminal = FALSE, ret = FALSE;

    gtk_tree_model_get(model, iter,
                       XFRUN_COL_COMMAND, &command,
                       XFRUN_COL_IN_TERMINAL, &in_terminal,
                       -1);

    if(!g_utf8_collate(command, dialog->priv->entry_val_tmp)) {
        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(dialog->priv->comboboxentry),
                                      iter);
        ret = TRUE;
    }

    g_free(command);

    return ret;
}

static gboolean
xfrun_entry_focus_out(GtkWidget *widget,
                      GdkEventFocus *evt,
                      gpointer user_data)
{
    XfrunDialog *dialog = XFRUN_DIALOG(user_data);

    if(gtk_combo_box_get_active(GTK_COMBO_BOX(dialog->priv->comboboxentry)) < 0) {
        dialog->priv->entry_val_tmp = gtk_editable_get_chars(GTK_EDITABLE(widget),
                                                             0, -1);
        gtk_tree_model_foreach(dialog->priv->completion_model,
                               xfrun_entry_check_match, dialog);
        g_free(dialog->priv->entry_val_tmp);
        dialog->priv->entry_val_tmp = NULL;
    }

    return FALSE;
}

static void
xfrun_add_to_history(const gchar *command,
                     gboolean in_terminal)
{
    gchar *histfile, *histfile1, **lines = xfrun_get_histfile_content();
    gint i;
    FILE *fp = NULL;
    GList *new_lines = NULL, *l;

    histfile = xfce_resource_save_location(XFCE_RESOURCE_CACHE,
                                    "xfce4/xfrun4/history.new", TRUE);
    if(!histfile) {
        g_critical("Unable to write to history file.");
        return;
    }

    if(!lines) {
        fp = fopen(histfile, "w");
        if(fp) {
            fprintf(fp, "%d:%s\n", in_terminal ? 1 : 0, command);
            fclose(fp);
        }
    } else {
        gchar *new_line;

        for(i = 0; lines[i]; ++i) {
            if(strlen(lines[i]) < 3 || lines[i][1] != ':')
                continue;

            if(g_utf8_collate(lines[i] + 2, command))
                new_lines = g_list_append(new_lines, lines[i]);
        }

        new_line = g_strdup_printf("%d:%s", in_terminal ? 1 : 0, command);
        new_lines = g_list_prepend(new_lines, new_line);

        fp = fopen(histfile, "w");
        if(fp) {
            for(l = new_lines, i = 0; l && i < MAX_ENTRIES; l = l->next, ++i)
                fprintf(fp, "%s\n", (char *)l->data);
            fclose(fp);
        }

        g_free(new_line);
    }

    histfile1 = g_strdup(histfile);
    histfile1[strlen(histfile1)-4] = 0;

    if(rename(histfile, histfile1))
        g_critical("Unable to rename '%s' to '%s'", histfile, histfile1);
    unlink(histfile);

    g_free(histfile1);
    g_free(histfile);
    g_strfreev(lines);
}

static void
xfrun_spawn_child_setup(gpointer data)
{
#if !defined(G_OS_WIN32) && defined(HAVE_SETSID)
    setsid();
#endif
}

static void
xfrun_run_clicked(GtkWidget *widget,
                  gpointer user_data)
{
    XfrunDialog  *dialog = XFRUN_DIALOG(user_data);
    GdkScreen    *gscreen;
    gboolean      in_terminal;
    GError       *error = NULL;
    gchar       **argv = NULL;
    gchar        *cmdline;
    gchar        *original_cmdline;
    gchar        *new_cmdline;
    gint          argc;

    cmdline = gtk_editable_get_chars(GTK_EDITABLE(dialog->priv->entry), 0, -1);
    original_cmdline = g_strdup (cmdline);
    in_terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->priv->terminal_chk));

    new_cmdline = xfce_expand_variables (cmdline, NULL);
    g_free (cmdline);
    cmdline = new_cmdline;

    gscreen = gtk_widget_get_screen(widget);

    if(dialog->priv->run_argument) {
        gchar *run_arg_quoted;

        run_arg_quoted = g_shell_quote(dialog->priv->run_argument);
        new_cmdline = g_strconcat(cmdline, " ", run_arg_quoted, NULL);

        g_free(run_arg_quoted);
        g_free(cmdline);
        cmdline = new_cmdline;
    }

    if (g_str_has_prefix (cmdline, "#"))
      {
        /* Shortcut to open manpages in terminal */
        new_cmdline = g_strconcat ("exo-open --launch TerminalEmulator 'man ",
                                   cmdline + 1, "'", NULL);
        g_free (cmdline);
        cmdline = new_cmdline;
        /* We already do that */
        in_terminal = FALSE;
      }

    if(in_terminal) {
        gint i = 0;

        argv = g_new0(gchar *, 4);
        argv[i++] = "xfterm4";
        argv[i++] = "-e";
        argv[i++] = cmdline;
        argv[i++] = NULL;
    } else {
        /* error is handled below */
        g_shell_parse_argv(cmdline, &argc, &argv, &error);
    }

    if(argv && gdk_spawn_on_screen(gscreen,
                                   dialog->priv->working_directory,
                                   argv, NULL, G_SPAWN_SEARCH_PATH,
                                   xfrun_spawn_child_setup, NULL, NULL,
                                   &error))
    {
        xfrun_add_to_history(original_cmdline, in_terminal);
        xfrun_dialog_delete_event(GTK_WIDGET(dialog), NULL);
    } else {
        gchar *primary = g_strdup_printf(_("The command \"%s\" failed to run:"),
                                         cmdline);
        xfce_message_dialog(GTK_WINDOW(dialog), _("Run Error"),
                            GTK_STOCK_DIALOG_ERROR, primary,
                            error ? error->message : _("Unknown Error"),
                            GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
        g_free(primary);
        if(error)
            g_error_free(error);
    }

    g_free(cmdline);
    g_free(original_cmdline);
    if(in_terminal)
        g_free(argv);
    else
        g_strfreev(argv);
}

static gboolean
xfrun_match_selected(GtkEntryCompletion *completion,
                     GtkTreeModel *model,
                     GtkTreeIter *iter,
                     gpointer user_data)
{
    XfrunDialog *dialog = XFRUN_DIALOG(user_data);
    gboolean in_terminal = FALSE;

    gtk_tree_model_get(model, iter,
                       XFRUN_COL_IN_TERMINAL, &in_terminal,
                       -1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dialog->priv->terminal_chk),
                                 in_terminal);

    return FALSE;
}

static GtkTreeModel *
xfrun_create_completion_model(XfrunDialog *dialog)
{
    GtkListStore *ls;
    gchar **lines = NULL;
    gchar *histfile = NULL;

    ls = gtk_list_store_new(XFRUN_N_COLS, G_TYPE_STRING, G_TYPE_BOOLEAN);

    lines = xfrun_get_histfile_content();
    if(lines) {
        GtkTreeIter itr;
        gint i;

        for(i = 0; lines[i]; ++i) {
            if(strlen(lines[i]) < 3 || lines[i][1] != ':')
                continue;

            gtk_list_store_append(ls, &itr);
            gtk_list_store_set(ls, &itr,
                               XFRUN_COL_COMMAND, lines[i] + 2,
                               XFRUN_COL_IN_TERMINAL,
                               lines[i][0] == '1' ? TRUE : FALSE,
                               -1);
        }

        g_strfreev(lines);
    }

    g_free(histfile);

    return GTK_TREE_MODEL(ls);
}




GtkWidget *
xfrun_dialog_new(const gchar *run_argument)
{
    return g_object_new(XFRUN_TYPE_DIALOG,
                        "run-argument", run_argument,
                        NULL);
}

void
xfrun_dialog_set_run_argument(XfrunDialog *dialog,
                              const gchar *run_argument)
{
    g_return_if_fail(XFRUN_IS_DIALOG(dialog));

    g_free(dialog->priv->run_argument);
    dialog->priv->run_argument = g_strdup(run_argument);

    if(run_argument) {
        gchar *title = g_strdup_printf(_("Open %s with what program?"),
                                       run_argument);
        gtk_window_set_title(GTK_WINDOW(dialog), title);
        g_free(title);
    } else
        gtk_window_set_title(GTK_WINDOW(dialog), _("Run program"));
}

G_CONST_RETURN gchar *
xfrun_dialog_get_run_argument(XfrunDialog *dialog)
{
    g_return_val_if_fail(XFRUN_IS_DIALOG(dialog), NULL);
    return dialog->priv->run_argument;
}

void
xfrun_dialog_set_destroy_on_close(XfrunDialog *dialog,
                                  gboolean destroy_on_close)
{
    g_return_if_fail(XFRUN_IS_DIALOG(dialog));
    dialog->priv->destroy_on_close = destroy_on_close;
}

gboolean
xfrun_dialog_get_destroy_on_close(XfrunDialog *dialog)
{
    g_return_val_if_fail(XFRUN_IS_DIALOG(dialog), FALSE);
    return dialog->priv->destroy_on_close;
}

void
xfrun_dialog_set_working_directory(XfrunDialog *dialog,
                                   const gchar *working_directory)
{
    g_return_if_fail(XFRUN_IS_DIALOG(dialog));

    g_free(dialog->priv->working_directory);
    dialog->priv->working_directory = g_strdup(working_directory);
}

G_CONST_RETURN gchar *
xfrun_dialog_get_working_directory(XfrunDialog *dialog)
{
    g_return_val_if_fail(XFRUN_IS_DIALOG(dialog), NULL);
    return dialog->priv->working_directory;
}

void
xfrun_dialog_select_text(XfrunDialog *dialog)
{
    gtk_editable_select_region(GTK_EDITABLE(XFRUN_DIALOG(dialog)->priv->entry),
                               0, -1);
}
