/*  xfrun4
 *  Copyright (C) 2000, 2002 Olivier Fourdan (fourdan@xfce.org)
 *  Copyright (C) 2002 Jasper Huijsmans (huysmans@users.sourceforge.net)
 *  Copyright (C) 2003 Eduard Roccatello (master@spine-group.org)
 *  Copyright (C) 2003,2004 Edscott Wilson Garcia <edscott@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#undef GTK_DISABLE_DEPRECATED

#include <sys/types.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>


#ifndef PATH_MAX
#define DEFAULT_LENGTH 1024
#else
#if (PATH_MAX < 1024)
#define DEFAULT_LENGTH 1024
#else
#define DEFAULT_LENGTH PATH_MAX
#endif
#endif

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
GCompletion *complete;
GList *history = NULL;
gint nComplete;
char *fileman = NULL;
gboolean use_xfc_combo=FALSE;
gboolean open_with=FALSE;
char *argument;

#if HAVE_LIBDBH && HAVE_XFHEADERS
#include "xfcombo.i"
#endif

gboolean run_completion (GtkWidget *widget,
                         GdkEventKey *event,
                         gpointer user_data);

gboolean tabFocusStop   (GtkWidget *widget,
                         GdkEventKey *event,
                         gpointer user_data);


gboolean
run_completion (GtkWidget *widget,
                GdkEventKey *event,
                gpointer user_data)
{
    /* Check for GDK_Tab */
    if (event->keyval == 0xff09) {
        GList *similar = NULL;
        const gchar *text = NULL;
        const gchar *prefix = NULL;
        gboolean selected = FALSE;
        gint len, selstart, i;

        g_signal_handlers_block_by_func (GTK_OBJECT (widget), run_completion, NULL);

        text = gtk_entry_get_text(GTK_ENTRY(widget));
        len = g_utf8_strlen(text, -1);
        if (len!=0) {
            /* seek for a selection */
            if ((selected = gtk_editable_get_selection_bounds(GTK_EDITABLE(widget), &selstart, NULL)) && selstart != 0) {
                nComplete++;
                prefix = g_strndup(text, selstart);
            }
            else {
                nComplete = 0;
                prefix = text;
            }

            /* make the completion */
            if ((similar = g_completion_complete(complete, prefix, NULL)) != NULL) {
                if (selected && selstart != 0) {
                    if (nComplete >= g_list_length(similar))
                        nComplete = 0;
                    for (i=0; i<nComplete; i++) {
                        if (similar->next!=NULL)
                            similar = similar->next;
                    }
                }
                gtk_entry_set_text(GTK_ENTRY(widget), similar->data);
                gtk_editable_select_region(GTK_EDITABLE(widget), (selstart == 0 ? len : selstart), -1);
            }
        }
        else {
            /* popup the combobox list if there's no text */
            g_signal_emit_by_name((gpointer)widget, "activate", NULL);
        }
        g_signal_handlers_unblock_by_func (GTK_OBJECT (widget), run_completion, NULL);
    }
    /* Check for return hit and send GTK_RESPONSE_OK to the dialog */
    else if (event->keyval == 0xff0d) {
        g_signal_stop_emission_by_name (GTK_OBJECT (widget), "key-press-event");
        gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    }
    return FALSE;
}

gboolean tabFocusStop   (GtkWidget *widget,
                         GdkEventKey *event,
                         gpointer user_data)
{
    g_signal_stop_emission_by_name (GTK_OBJECT (widget), "key-press-event");
    return TRUE;
}
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
    if (path && g_file_test (path, G_FILE_TEST_IS_DIR)){
	g_free(path);
	path=NULL;
    }
        
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
    if (open_with){
	gchar *g=g_strconcat(execute," ",argument,NULL);
	g_free(execute);
	execute=g;
    }
    g_free(path);
    success = xfce_exec (execute, FALSE, FALSE, NULL);
    g_free(execute);
    
    return success;
}

static char *get_fileman(void)
{
    const char *var = g_getenv("FILEMAN");

    if (var && strlen(var))
	return g_strdup(var);
    else
	return g_strdup("xftree4");	    
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
        g_warning(_("xfrun4: Could not write history to file %s\n"), hfile);
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

void runit(GtkEntry * entry, gpointer user_data){
    const gchar *command;
    gboolean in_terminal;

    command = gtk_entry_get_text(entry);

    in_terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        checkbox));

    if (do_run(command, in_terminal)) {
	        if (!use_xfc_combo) put_history(command, in_terminal, history);
#if HAVE_LIBDBH && HAVE_XFHEADERS

		else if (use_xfc_combo) {
		    gchar *f=g_build_filename(RUN_DBH_FILE,NULL); 
      		    XFC_save_to_history(f,(char *)command);
      		    save_flags((char *)command,in_terminal,FALSE);
		    g_free(f);
		}
#endif
    }
}

#if HAVE_LIBDBH && HAVE_XFHEADERS

void alt_runit(GtkEntry * entry, gpointer user_data){
    const gchar *command;
    gboolean in_terminal;
    command = gtk_entry_get_text(entry);

    in_terminal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
                        checkbox));

    if (do_run(command, in_terminal)) {
	    gchar *f=g_build_filename(RUN_DBH_FILE,NULL); 
	    XFC_save_to_history(f,(char *)command);
	    save_flags((char *)command,in_terminal,FALSE);
	    g_free(f);
	    gtk_dialog_response (GTK_DIALOG(dialog),GTK_RESPONSE_NONE);
    }
}
#endif

int main(int argc, char **argv)
{
    GtkWidget *button;
    GtkWidget *combo_entry, *combo_list;
    GList *hitem, *hstrings;
    GtkWidget *vbox;
    GtkWidget *combo;
    XFCommand *current;
    gchar *title;

#if 0
#ifdef ENABLE_NLS
    /* This is required for UTF-8 at least - Please don't remove it */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
    textdomain (GETTEXT_PACKAGE);
#endif
#else
    xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
#endif

    gtk_init(&argc, &argv);

    if (argc >= 2 && g_file_test(argv[1], G_FILE_TEST_EXISTS)){
	argument=argv[1];
	open_with=TRUE;
    }
    history = get_history();

    fileman = get_fileman();

    complete = g_completion_new(NULL);

    if (open_with) title=g_strdup_printf(_("Open %s with what program?"),argument);
    else title=g_strdup(_("Run program"));
    dialog = gtk_dialog_new_with_buttons(title, NULL, GTK_DIALOG_NO_SEPARATOR, NULL);
    g_free(title);
    button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
    gtk_widget_show(button);
    
    button = xfce_create_mixed_button (GTK_STOCK_OK, _("_Run"));
    gtk_widget_show(button);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);

    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 10);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);

    combo = gtk_combo_new();
    gtk_combo_set_case_sensitive(GTK_COMBO(combo), TRUE);

#if HAVE_LIBDBH && HAVE_XFHEADERS

    if ((xfc_fun=load_xfc()) != NULL) use_xfc_combo=TRUE;  
#endif
    
    gtk_box_pack_start(GTK_BOX(vbox), combo, TRUE, TRUE, 0);
		     
    combo_entry = GTK_COMBO(combo)->entry;
    combo_list = GTK_COMBO(combo)->list;

    g_object_set(G_OBJECT(combo_entry), "activates-default", FALSE, NULL);

    checkbox = gtk_check_button_new_with_mnemonic(_("Run in _terminal"));
    gtk_box_pack_start(GTK_BOX(vbox), checkbox, TRUE, TRUE, 0);
    
    for (hitem = history, hstrings = NULL; hitem != NULL; hitem = hitem->next) {
	    current = hitem->data;
	    hstrings = g_list_append(hstrings,current->command);
    }

    if (use_xfc_combo) {
#if HAVE_LIBDBH && HAVE_XFHEADERS
	    combo_info = XFC_init_combo((GtkCombo *)combo);
	    combo_info->activate_func = alt_runit;
    	    xfc_fun->extra_key_completion = extra_key_completion;
	    xfc_fun->extra_key_data = (gpointer)combo_entry;
	    combo_info->entry = (GtkEntry *)combo_entry;
	    /*combo_info->activate_user_data=(gpointer)combo_info;*/
	    set_run_combo(combo_info);
#else
	    g_assert_not_reached();
#endif
    } 
    else { 
      if (hstrings != NULL) {
        gtk_combo_set_popdown_strings(GTK_COMBO(combo), hstrings);
        g_completion_add_items(complete, hstrings);
        g_list_free(hstrings);
      }

      if (history != NULL && (current = history->data) != NULL) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox),
				    current->in_terminal);
      }

      g_signal_connect(G_OBJECT(combo_list), "select_child",
            G_CALLBACK(set_history_checkbox), NULL);

      g_signal_connect(GTK_WIDGET(combo_entry), "key-press-event",
            G_CALLBACK(run_completion), NULL);

      g_signal_connect_after(GTK_WIDGET(combo_entry), "key-press-event",
            G_CALLBACK(tabFocusStop), NULL);
    }
    
    gtk_editable_select_region(GTK_EDITABLE(combo_entry), 0, -1);
    gtk_widget_grab_focus(combo_entry);
    
    gtk_widget_show(checkbox);
    gtk_widget_show(combo);

    while (1) {
	    int response;

	    response = gtk_dialog_run(GTK_DIALOG(dialog));

	    if (response == GTK_RESPONSE_OK) {
		    	runit(GTK_ENTRY(combo_entry),NULL);
			break;
	    }
	    else
	        break;
    }

    gtk_widget_destroy(dialog);
    if (!use_xfc_combo) g_completion_free(complete);
    g_free(fileman);

    if (history != NULL) {
        g_list_foreach(history, (GFunc)free_hitem, NULL);
        g_list_free(history);
    }
    
#if HAVE_LIBDBH && HAVE_XFHEADERS

    if (use_xfc_combo) {
	XFC_destroy_combo(combo_info);
    	unload_xfc();
    }
#endif
    
    return 0;
}
