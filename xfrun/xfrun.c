/*  xfrun4
 *  Copyright (C) 2000, 2002 Olivier Fourdan (fourdan@xfce.org)
 *  Copyright (C) 2002 Jasper Huijsmans (huysmans@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#undef GTK_DISABLE_DEPRECATED

#include <sys/types.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libxfce4util/util.h>
#include <libxfcegui4/dialogs.h>

#ifndef PATH_MAX
#define DEFAULT_LENGTH 1024
#else
#if (PATH_MAX < 1024)
#define DEFAULT_LENGTH 1024
#else
#define DEFAULT_LENGTH PATH_MAX
#endif
#endif

#define _(x) x
#define N_(x) x

#define HFILE "xfrun_history"

#define MAXHISTORY 10

typedef struct
{
    gchar *command;
    gboolean in_terminal;
}
XFCommand;

GtkWidget *dialog;
GtkWidget *checkbox;
GList *history = NULL;
char *fileman = NULL;

void set_history_checkbox(GtkList *list, GtkWidget *child)
{
    gint ipos = gtk_list_child_position(list, child);
    GList *hitem = g_list_nth(history, ipos);
    XFCommand *current = hitem -> data;
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox),
				 current->in_terminal);
}

static gboolean do_run(const char *cmd, gboolean in_terminal)
{
    gchar *execute, *path;
    gboolean success;

    g_return_val_if_fail(cmd != NULL, FALSE);

    /* this is only used to prevent us to open a directory in the 
     * users's home dir with the same name as an executable,
     * e.g. evolution */
    path = g_find_program_in_path(cmd);
        
    /* open directory in terminal or file manager */
    if (g_file_test (cmd, G_FILE_TEST_IS_DIR) && !path)
    {
	if(in_terminal)
	    execute = g_strconcat("xfterm4 ", cmd, NULL);
	else 
	    execute = g_strconcat(fileman, " ", cmd, NULL);
    }
    else
    {
	if(in_terminal)
	    execute = g_strconcat("xfterm4 -e ", cmd, NULL);
	else
	    execute = g_strdup(cmd);
    }

    g_free(path);
    success = exec_command(execute);
    g_free(execute);
    
    return success;
}

static char *get_fileman(void)
{
    const char *var = g_getenv("FILEMAN");

    if (var && strlen(var))
	return g_strdup(var);
    else
	return g_strdup("xffm");	    
}

GList *get_history(void)
{
    FILE *fp;
    char *hfile = xfce_get_userfile(HFILE, NULL);
    char line[DEFAULT_LENGTH];
    char *check;
    GList *cbtemp = NULL;
    XFCommand *current;

    int i = 0;

    if(!(fp = fopen(hfile, "r")))
    {
        g_free(hfile);

        return NULL;
    }

    line[DEFAULT_LENGTH - 1] = '\0';

    /* no more than MAXHISTORY history items */
    for(i = 0; i < MAXHISTORY && fgets(line, DEFAULT_LENGTH - 1, fp); i++)
    {
        if((line[0] == '\0') || (line[0] == '\n'))
            break;

        current = g_new0(XFCommand, 1);
	
        if((check = strrchr(line, '\n')))
            *check = '\0';

	if ((check = strrchr(line, ' ')))
	{
	    *check = '\0';
	    check++;
	    current->in_terminal = (atoi(check) != 0);
	} 
	else 
	{
	    current->in_terminal = FALSE;
	}
	
	current->command = g_strdup(line);
        cbtemp = g_list_append(cbtemp, current);
    }

    g_free(hfile);
    fclose(fp);

    return cbtemp;
}

void put_history(const char *newest, gboolean in_terminal, GList * cb)
{
    FILE *fp;
    char *hfile = xfce_get_userfile(HFILE, NULL);
    GList *node;
    int i;

    if(!(fp = fopen(hfile, "w")))
    {
        g_warning("xfrun4: Could not write history to file %s\n", hfile);
        g_free(hfile);
        return;
    }

    fprintf(fp, "%s %d\n", newest, in_terminal);
    i = 1;

    for(node = cb; node != NULL && i < MAXHISTORY; node = node->next) {
	    XFCommand *current = (XFCommand *)node->data;

        if(current->command && strlen(current->command) &&
          (strcmp(current->command, newest) != 0))
        {
            fprintf(fp, "%s %d\n", current->command, current->in_terminal);
            i++;
        }
    }

    fclose(fp);
    g_free(hfile);
}

static void free_hitem(XFCommand *hitem)
{
    g_free(hitem->command);
    g_free(hitem);
}

int main(int argc, char **argv)
{
    GtkWidget *button;
    GtkWidget *combo_entry, *combo_list;
    GList *hitem, *hstrings;
    GtkWidget *vbox;
    GtkWidget *combo;
    XFCommand *current;

    gtk_init(&argc, &argv);

    history = get_history();

    fileman = get_fileman();
    
    dialog = gtk_dialog_new_with_buttons(_("Run program"), NULL, GTK_DIALOG_NO_SEPARATOR, NULL);
    
    button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
    gtk_widget_show(button);
    
    button = mixed_button_new(GTK_STOCK_OK, _("_Run"));
    gtk_widget_show(button);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);

    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 10);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

    combo = gtk_combo_new();
    
    for (hitem = history, hstrings = NULL; hitem != NULL; hitem = hitem->next) {
	    current = hitem->data;
	    hstrings = g_list_append(hstrings,current->command);
    }

    if (hstrings != NULL) {
        gtk_combo_set_popdown_strings(GTK_COMBO(combo), hstrings);
        g_list_free(hstrings);
    }

    gtk_box_pack_start(GTK_BOX(vbox), combo, TRUE, TRUE, 0);
    gtk_widget_show(combo);
		     
    combo_entry = GTK_COMBO(combo)->entry;
    combo_list = GTK_COMBO(combo)->list;

    gtk_combo_disable_activate(GTK_COMBO(combo));
    g_object_set(G_OBJECT(combo_entry), "activates-default", TRUE, NULL);

    checkbox = gtk_check_button_new_with_mnemonic(_("Run in _terminal"));
    
    if (history != NULL && (current = history->data) != NULL) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox),
				    current->in_terminal);
    }

    gtk_widget_show(checkbox);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, TRUE, TRUE, 0);

    gtk_editable_select_region(GTK_EDITABLE(combo_entry), 0, -1);
    gtk_widget_grab_focus(combo_entry);

    g_signal_connect(G_OBJECT(combo_list), "select_child",
		     G_CALLBACK(set_history_checkbox), NULL);

    for (;;) {
	    int response;
	
	    response = gtk_dialog_run(GTK_DIALOG(dialog));

	    if (response == GTK_RESPONSE_OK) {
	        const gchar *command;
	        gboolean in_terminal;
	    
	        command = gtk_entry_get_text(GTK_ENTRY(combo_entry));

	        in_terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        checkbox));

	        if (do_run(command, in_terminal)) {
		        put_history(command, in_terminal, history);
		        break;
	        }
	    }
	    else
	        break;
    }
    
    gtk_widget_destroy(dialog);
    g_free(fileman);

    if (history != NULL) {
        g_list_foreach(history, (GFunc)free_hitem, NULL);
        g_list_free(history);
    }

    return 0;
}
