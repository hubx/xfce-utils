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

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef MAXSTRLEN
#define MAXSTRLEN 1024
#endif

#define _(x) x
#define N_(x) x

#define HFILE ".xfce4/xfrun_history"

#define MAXHISTORY 10

GtkWidget *dialog;

/*  Button with text and stock icon
 *  -------------------------------
 *  Taken from ROX Filer (http://rox.sourceforge.net)
 *  by Thomas Leonard
*/
GtkWidget *mixed_button_new(const char *stock, const char *message)
{
    GtkWidget *button, *align, *image, *hbox, *label;

    button = gtk_button_new();
    label = gtk_label_new_with_mnemonic(message);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), button);

    image = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_BUTTON);
    hbox = gtk_hbox_new(FALSE, 2);

    align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);

    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(button), align);
    gtk_container_add(GTK_CONTAINER(align), hbox);
    gtk_widget_show_all(align);

    return button;
}

/* from xfce4 */
void report_error(const char *text, GtkWidget *parent)
{
    GtkWidget *dlg;

    dlg = gtk_message_dialog_new(parent ? GTK_WINDOW(parent) : NULL,
                                    GTK_DIALOG_MODAL |
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
                                    text);

    gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);

    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

/* Executing commands. Taken from xfce4. */

/* '~' doesn't get expanded by g_spawn_* */
/* this has to be statically allocated for putenv !! */
static char newpath[MAXSTRLEN + 1];

static void expand_path(void)
{
    static gboolean first = TRUE;

    /* we don't have to do this every time */
    if(first)
    {
        const char *path = g_getenv("PATH");
        const char *home = g_getenv("HOME");
        int homelen = strlen(home);
        const char *c;
        char *s;

        if(!path || !strlen(path))
            return;

        c = path;
        s = newpath;

        strcpy(s, "PATH=");
        s += 5;

        while(*c)
        {
            if(*c == '~')
            {
                strcpy(s, home);
                s += homelen;
            }
            else
            {
                *s = *c;
                s++;
            }

            c++;
        }

        *s = '\0';
        first = FALSE;

        putenv(newpath);
    }
}

static gboolean exec_cmd(const char *cmd, gboolean in_terminal)
{
    GError *error = NULL;       /* this must be NULL to prevent crash :( */
    char execute[MAXSTRLEN + 1];

    if(!cmd)
        return;

    /* make sure '~' is expanded in the users PATH */
    expand_path();

    if(in_terminal)
        snprintf(execute, MAXSTRLEN, "xfterm -e %s", cmd);
    else
        snprintf(execute, MAXSTRLEN, "%s", cmd);

    if(!g_spawn_command_line_async(execute, &error))
    {
        char *msg;

        msg = g_strcompress(error->message);

        report_error(msg, dialog);

        g_free(msg);
	return FALSE;
    }
    
    return TRUE;
}

GList *get_history(void)
{
    FILE *fp;
    const char *home = g_getenv("HOME");
    char *hfile = g_strconcat(home, "/", HFILE, NULL);
    GList *cbtemp = NULL;
    char line[MAXSTRLEN];
    char *check;
    int i = 0;

    if(!(fp = fopen(hfile, "r")))
    {
        g_free(hfile);
        cbtemp = g_list_append(cbtemp, "");

        return cbtemp;
    }

    line[MAXSTRLEN - 1] = '\0';

    /* no more than 10 history items */
    for(i = 0; i < MAXHISTORY && fgets(line, MAXSTRLEN - 1, fp); i++)
    {
        if((line[0] == '\0') || (line[0] == '\n'))
            break;

        if((check = strrchr(line, '\n')))
            *check = '\0';

        cbtemp = g_list_append(cbtemp, g_strdup(line));
    }

    g_free(hfile);
    fclose(fp);

    if(!cbtemp)
        cbtemp = g_list_append(cbtemp, "");

    return cbtemp;
}

void put_history(const char *newest, GList * cb)
{
    FILE *fp;
    const char *home = g_getenv("HOME");
    char *hfile = g_strconcat(home, "/", HFILE, NULL);
    GList *node;
    int i;

    if(!(fp = fopen(hfile, "w")))
    {
        g_print("xfrun: Could not write history to file %s\n", hfile);
        g_free(hfile);
        return;
    }

    fprintf(fp, "%s\n", newest);
    i = 1;

    for(node = cb; node && i < MAXHISTORY; node = node->next)
    {
        char *cmd = (char *)node->data;

        if(cmd && *cmd != '\0' && (strcmp(cmd, newest) != 0))
        {
            fprintf(fp, "%s\n", cmd);
            i++;
        }
    }

    fclose(fp);
    g_free(hfile);
}

int main(int argc, char **argv)
{
    GtkWidget *button;
    GtkWidget *combo_entry;
    GList *history;
    gboolean in_terminal;
    GtkWidget *checkbox;
    GtkWidget *vbox;
    GtkWidget *combo;

    gtk_init(&argc, &argv);

    history = get_history();

    dialog = gtk_dialog_new_with_buttons(_("Run program"), NULL, 0, NULL);
    
    button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_widget_show(button);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
    
    button = mixed_button_new(GTK_STOCK_OK, _("_Run"));
    gtk_widget_show(button);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);

    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 10);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    vbox = gtk_vbox_new(FALSE, 4);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);

    combo = gtk_combo_new();
    combo_entry = GTK_COMBO(combo)->entry;
    gtk_combo_set_popdown_strings(GTK_COMBO(combo), history);
    gtk_box_pack_start(GTK_BOX(vbox), combo, TRUE, TRUE, 2);
    gtk_widget_show(combo);

    gtk_combo_disable_activate(GTK_COMBO(combo));
    g_object_set(G_OBJECT(combo_entry), "activates-default", TRUE, NULL);

    checkbox = gtk_check_button_new_with_mnemonic(_("Run in _terminal"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), FALSE);
    gtk_widget_show(checkbox);
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, TRUE, TRUE, 2);

    gtk_editable_select_region(GTK_EDITABLE(combo_entry), 0, -1);
    gtk_widget_grab_focus(combo_entry);

    while (1)
    {
	int response = GTK_RESPONSE_NONE;
	
	response = gtk_dialog_run(GTK_DIALOG(dialog));

	if (response == GTK_RESPONSE_OK)
	{
	    char *msg;
	    const char *command;
	    gboolean in_terminal;
	    
	    command = gtk_entry_get_text(GTK_ENTRY(combo_entry));

	    in_terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox));

	    if (exec_cmd(command, in_terminal))
	    {
		put_history(command, history);
		break;
	    }
	}
	else
	    break;
    }
    
    gtk_widget_destroy(dialog);

    return 0;
}
