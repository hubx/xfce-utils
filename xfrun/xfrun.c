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

#include <stdio.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#define BORDER       8
#define MAX_ENTRIES 20

typedef struct
{
    GtkWidget *window;
    GtkWidget *entry;
    GtkWidget *arrow_btn;
    GtkWidget *terminal_chk;
    GtkTreeModel *completion_model;
    const gchar *run_argument;
    
    gchar *entry_val_tmp;
} XfrunDialog;

enum
{
    XFRUN_COL_COMMAND = 0,
    XFRUN_COL_IN_TERMINAL,
    XFRUN_N_COLS,
};

static gchar **
xfrun_get_histfile_content()
{
    gchar **lines = NULL, *histfile, *contents = NULL;
    gsize length = 0;
    
    histfile = xfce_resource_lookup(XFCE_RESOURCE_CACHE, "xfce4/xfrun4/history");
    if(histfile && g_file_get_contents(histfile, &contents, &length, NULL)) {
        lines = g_strsplit(contents, "\n", -1);
        g_free(contents);
    }
    
    return lines;
}

static gboolean
xfrun_key_press(GtkWidget *widget,
                GdkEventKey *evt,
                gpointer user_data)
{
    if(evt->keyval == GDK_Escape) {
        gtk_main_quit();
        return TRUE;
    }
    
    return FALSE;
}

static gboolean
xfrun_entry_check_match(GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer data)
{
    XfrunDialog *xfrun_dialog = (XfrunDialog *)data;
    gchar *command = NULL;
    gboolean in_terminal = FALSE, ret = FALSE;
    
    gtk_tree_model_get(model, iter,
                       XFRUN_COL_COMMAND, &command,
                       XFRUN_COL_IN_TERMINAL, &in_terminal,
                       -1);
    
    if(!g_utf8_collate(command, xfrun_dialog->entry_val_tmp)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(xfrun_dialog->terminal_chk),
                                     in_terminal);
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
    XfrunDialog *xfrun_dialog = (XfrunDialog *)user_data;
    
    xfrun_dialog->entry_val_tmp = gtk_editable_get_chars(GTK_EDITABLE(widget),
                                                         0, -1);
    gtk_tree_model_foreach(xfrun_dialog->completion_model,
                           xfrun_entry_check_match, xfrun_dialog);
    g_free(xfrun_dialog->entry_val_tmp);
    xfrun_dialog->entry_val_tmp = NULL;
    
    return FALSE;
}

static void
xfrun_menu_item_activated(GtkWidget *widget,
                          gpointer user_data)
{
    XfrunDialog *xfrun_dialog = (XfrunDialog *)user_data;
    GtkWidget *lbl;
    const gchar *command;
    gboolean in_terminal;
    
    lbl = gtk_bin_get_child(GTK_BIN(widget));
    g_return_if_fail(GTK_IS_LABEL(lbl));
    
    command = gtk_label_get_text(GTK_LABEL(lbl));
    gtk_entry_set_text(GTK_ENTRY(xfrun_dialog->entry), command);
    
    in_terminal = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget),
                                                     "--xfrun-in-terminal"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(xfrun_dialog->terminal_chk),
                                 in_terminal);
}

static gboolean
xfrun_populate_menu(GtkTreeModel *model,
                    GtkTreePath *path,
                    GtkTreeIter *iter,
                    gpointer data)
{
    GtkWidget *menu = GTK_WIDGET(data), *mi;
    XfrunDialog *xfrun_dialog = g_object_get_data(G_OBJECT(menu), 
                                                  "--xfrun-dialog");
    gchar *command = NULL;
    gboolean in_terminal = FALSE;
    
    gtk_tree_model_get(model, iter,
                       XFRUN_COL_COMMAND, &command,
                       XFRUN_COL_IN_TERMINAL, &in_terminal,
                       -1);
    
    mi = gtk_menu_item_new_with_label(command);
    g_object_set_data(G_OBJECT(mi), "--xfrun-in-terminal",
                      GINT_TO_POINTER(in_terminal));
    gtk_widget_show(mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
    g_signal_connect(G_OBJECT(mi), "activate",
                     G_CALLBACK(xfrun_menu_item_activated), xfrun_dialog);
    
    g_free(command);
    
    return FALSE;
}

static void
xfrun_menu_position(GtkMenu *menu,
                    gint *x,
                    gint *y,
                    gboolean *push_in,
                    gpointer user_data)
{
    XfrunDialog *xfrun_dialog = (XfrunDialog *)user_data;
    GtkAllocation *entry_al = &xfrun_dialog->entry->allocation;
    GtkAllocation *btn_al = &xfrun_dialog->arrow_btn->allocation;
    gint entry_x, entry_y;
    
    gdk_window_get_origin(xfrun_dialog->entry->window, &entry_x, &entry_y);
    
    *x = entry_x;
    *y = entry_y + entry_al->height;
    *push_in = FALSE;
    
    gtk_widget_set_size_request(GTK_WIDGET(menu),
                                btn_al->x - entry_al->x + btn_al->width, -1);
}

static gboolean
xfrun_menu_destroy_idled(gpointer user_data)
{
    gtk_widget_destroy(GTK_WIDGET(user_data));
    return FALSE;
}

static void
xfrun_menu_destroy(GtkWidget *widget,
                   gpointer user_data)
{
    g_idle_add(xfrun_menu_destroy_idled, widget);
}

static void
xfrun_menu_button_clicked(GtkWidget *widget,
                          gpointer user_data)
{
    XfrunDialog *xfrun_dialog = (XfrunDialog *)user_data;
    GtkWidget *menu;
    
    menu = gtk_menu_new();
    g_object_set_data(G_OBJECT(menu), "--xfrun-dialog", xfrun_dialog);
    gtk_widget_show(menu);
    g_signal_connect(G_OBJECT(menu), "deactivate",
                     G_CALLBACK(xfrun_menu_destroy), menu);
    
    gtk_tree_model_foreach(xfrun_dialog->completion_model,
                           xfrun_populate_menu, menu);
    
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL,
                   xfrun_menu_position, xfrun_dialog,
                   1, gtk_get_current_event_time());
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
        fprintf(fp, "%d:%s\n", in_terminal ? 1 : 0, command);
        fclose(fp);
    } else {
        for(i = 0; lines[i]; ++i) {
            if(strlen(lines[i]) < 3 || lines[i][1] != ':')
                continue;
            
            if(g_utf8_collate(lines[i] + 2, command))
                new_lines = g_list_append(new_lines, lines[i]);
        }
        
        new_lines = g_list_prepend(new_lines, g_strdup_printf("%d:%s",
                                                              in_terminal ? 1 : 0,
                                                              command));
        
        fp = fopen(histfile, "w");
        for(l = new_lines, i = 0; l && i < MAX_ENTRIES; l = l->next, ++i)
            fprintf(fp, "%s\n", (char *)l->data);
        fclose(fp);
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
xfrun_run_clicked(GtkWidget *widget,
                  gpointer user_data)
{
    XfrunDialog *xfrun_dialog = (XfrunDialog *)user_data;
    gchar *entry_str, *command;
    gboolean in_terminal;
    GError *error = NULL;
    
    entry_str = gtk_editable_get_chars(GTK_EDITABLE(xfrun_dialog->entry), 0, -1);
    in_terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(xfrun_dialog->terminal_chk));
    
    if(xfrun_dialog->run_argument) {
        command = g_strdup_printf("%s \"%s\"", entry_str,
                                  xfrun_dialog->run_argument);
        g_free(entry_str);
    } else
        command = entry_str;
    
    if(xfce_exec(command, in_terminal, FALSE, &error)) {
        xfrun_add_to_history(command, in_terminal);
        gtk_main_quit();
    } else {
        gchar *primary = g_strdup_printf(_("The command \"%s\" failed to run:"),
                                         command);
        xfce_message_dialog(GTK_WINDOW(xfrun_dialog->window), _("Run Error"),
                            GTK_STOCK_DIALOG_ERROR,
                            primary, error->message,
                            GTK_STOCK_CLOSE, GTK_RESPONSE_ACCEPT, NULL);
        g_free(primary);
        g_error_free(error);
    }
    
    g_free(command);
}

static gboolean
xfrun_match_selected(GtkEntryCompletion *completion,
                     GtkTreeModel *model,
                     GtkTreeIter *iter,
                     gpointer user_data)
{
    XfrunDialog *xfrun_dialog = (XfrunDialog *)user_data;
    gboolean in_terminal = FALSE;
    
    gtk_tree_model_get(model, iter,
                       XFRUN_COL_IN_TERMINAL, &in_terminal,
                       -1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(xfrun_dialog->terminal_chk),
                                 in_terminal);
    
    return FALSE;
}

static GtkTreeModel *
xfrun_create_completion_model(XfrunDialog *xfrun_dialog)
{
    GtkListStore *ls;
    gchar **lines = NULL;
    gchar *histfile = NULL, *contents = NULL;
    gint length = 0;
    
    ls = gtk_list_store_new(XFRUN_N_COLS, G_TYPE_STRING, G_TYPE_BOOLEAN);
    
    lines = xfrun_get_histfile_content();
    if(lines) {
        GtkTreeIter itr;
        gint i;
        gchar *p;
        
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

int
main(int argc,
     char **argv)
{
    XfrunDialog xfrun_dialog;
    gchar title[8192];
    GtkWidget *win, *entry, *chk, *btn, *vbox, *bbox, *hbox, *arrow;
    GtkEntryCompletion *completion;
    GtkTreeModel *completion_model;
    
    xfce_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");
    
    gtk_init(&argc, &argv);
    
    if(argc >= 2) {
        g_snprintf(title, 8192, _("Open %s with what program?"), argv[1]);
        xfrun_dialog.run_argument = argv[1];
    } else {
        g_strlcpy(title, _("Run program"), 8192);
        xfrun_dialog.run_argument = NULL;
    }
    
    xfrun_dialog.window = win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win), title);
    gtk_window_set_default_size(GTK_WINDOW(win), 400, 10);
    g_signal_connect(G_OBJECT(win), "delete-event",
                     G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(win), "key-press-event",
                     G_CALLBACK(xfrun_key_press), NULL);
    
    vbox = gtk_vbox_new(FALSE, BORDER/2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), BORDER);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    
    hbox = gtk_hbox_new(FALSE, BORDER/4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    completion = gtk_entry_completion_new();
    xfrun_dialog.completion_model = completion_model = xfrun_create_completion_model(&xfrun_dialog);
    gtk_entry_completion_set_model(completion, completion_model);
    gtk_entry_completion_set_text_column(completion, XFRUN_COL_COMMAND);
    gtk_entry_completion_set_popup_completion(completion, TRUE);
    gtk_entry_completion_set_inline_completion(completion, TRUE);
    g_signal_connect(G_OBJECT(completion), "match-selected",
                     G_CALLBACK(xfrun_match_selected), &xfrun_dialog);
    
    xfrun_dialog.entry = entry = gtk_entry_new();
    gtk_entry_set_completion(GTK_ENTRY(entry), completion);
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    gtk_widget_show(entry);
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
                     G_CALLBACK(xfrun_entry_focus_out), &xfrun_dialog);
    
    xfrun_dialog.arrow_btn = btn = gtk_button_new();
    gtk_container_set_border_width(GTK_CONTAINER(btn), 0);
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(btn), "clicked",
                     G_CALLBACK(xfrun_menu_button_clicked), &xfrun_dialog);
    
    arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_NONE);
    gtk_widget_show(arrow);
    gtk_container_add(GTK_CONTAINER(btn), arrow);
    
    xfrun_dialog.terminal_chk = chk = gtk_check_button_new_with_mnemonic(_("Run in _terminal"));
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(vbox), chk, FALSE, FALSE, 0);
    
    bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(bbox), BORDER);
    gtk_widget_show(bbox);
    gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
    
    btn = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_widget_show(btn);
    gtk_box_pack_end(GTK_BOX(bbox), btn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(btn), "clicked",
                     G_CALLBACK(gtk_main_quit), NULL);
    
    btn = xfce_create_mixed_button(GTK_STOCK_EXECUTE, _("_Run"));
    gtk_widget_show(btn);
    gtk_box_pack_end(GTK_BOX(bbox), btn, FALSE, FALSE, 0);
    GTK_WIDGET_SET_FLAGS(btn, GTK_CAN_DEFAULT);
    gtk_widget_grab_default(btn);
    g_signal_connect(G_OBJECT(btn), "clicked",
                     G_CALLBACK(xfrun_run_clicked), &xfrun_dialog);
    
    gtk_widget_realize(win);
    xfce_gtk_window_center_on_monitor_with_pointer(GTK_WINDOW(win));
    gtk_widget_show(win);
    
    gtk_main();
    
    return 0;
}
