/*  xfbdmgr
 *  Copyright (C) 2002 Jasper Huijsmans (j.b.huijsmans@hetnet.nl)
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

/*
 * Rewrite of xfbdmgr by Alex Lambert
 * xfbdmgr (c) 2000 by Alex Lambert (alex@penwing.uklinux.net)
 * distributed under the GPL
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>
#include <string.h>
#include "xfbdmgr.h"
#include "xfbdmgr_cb.h"

#define _(x) x
#define N_(x) x

#define HELP_FILE "utils-xfbdmgr.html"

static gboolean
unsaved_changes_dialog (BackdropList * bl)
{
  int response = GTK_RESPONSE_CANCEL;
  GtkWidget *dialog;
  char *text = _("There are unsaved changes.\n"
                 "Do you want to save?");

  dialog = gtk_message_dialog_new (GTK_WINDOW (bl->window),
				   GTK_DIALOG_MODAL |
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_QUESTION,
				   GTK_BUTTONS_NONE, text);

  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
			  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			  GTK_STOCK_NO, GTK_RESPONSE_NO, GTK_STOCK_YES,
			  GTK_RESPONSE_YES, NULL);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (dialog);

  switch (response)
    {
      case GTK_RESPONSE_YES:
        save_cb (NULL, bl);
        return TRUE;
        break;
      case GTK_RESPONSE_NO:
        return TRUE;
        break;
      case GTK_RESPONSE_CANCEL:
        return FALSE;
        break;
    }
    
  return FALSE;
}

void
quit_cb (GtkWidget * w, BackdropList * bl)
{
  if (!bl->saved)
    unsaved_changes_dialog (bl);

  gtk_widget_destroy (w);

  gtk_main_quit ();
}

void
save_cb (GtkWidget * w, BackdropList * bl)
{
  if (!bl->saved && save_list (bl))
    gtk_widget_set_sensitive (bl->save_button, FALSE);
}

void
save_as_cb (GtkWidget * w, BackdropList * bl)
{
  if (save_list_as (bl))
    gtk_widget_set_sensitive (bl->save_button, FALSE);
}


static void
new_cb (BackdropList * bl)
{
  if (!bl->saved && !unsaved_changes_dialog (bl))
    return;

  g_free (bl->filename);
  bl->filename = NULL;
  gtk_list_store_clear (bl->list);
  bl->num_items = 0;
  bl->saved = TRUE;
  gtk_widget_set_sensitive (bl->save_button, FALSE);
  gtk_label_set_text (GTK_LABEL (bl->file_label), _("None"));
}

static void
open_cb (BackdropList * bl)
{
  char *path = bl->filename ? bl->filename : bl->last_path;
  char *filename;
  
  if (!bl->saved && !unsaved_changes_dialog (bl))
    return;
  
  filename = get_filename_dialog (_("Select backdrop list"), path);

  if (!filename)
    return;

  g_free (bl->filename);
  bl->filename = filename;

  gtk_list_store_clear (bl->list);

  load_list (bl);
}

static void
add_cb (BackdropList * bl)
{
  char *path = bl->last_path ? bl->last_path : NULL;
  char *filename = get_filename_dialog (_("Select image file"), path);

  if (filename)
    {
      GtkTreeIter iter;

      gtk_list_store_append (bl->list, &iter);
      gtk_list_store_set (bl->list, &iter, 0, filename, -1);

      g_free (bl->last_path);
      path = g_path_get_dirname (filename);
      bl->last_path = g_strconcat (path, "/", NULL);
      g_free (path);
      g_free (filename);
      bl->saved = FALSE;
      bl->num_items++;
      gtk_widget_set_sensitive (bl->save_button, TRUE);
    }
}

static void
remove_cb (BackdropList * bl)
{
  GtkTreeSelection *sel =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (bl->treeview));
  GtkTreeIter iter;

  if (sel && gtk_tree_selection_get_selected (sel, NULL, &iter))
    {
      gtk_list_store_remove (bl->list, &iter);
      bl->num_items--;
      bl->saved = FALSE;
      gtk_widget_set_sensitive (bl->save_button, TRUE);
    }
}

static void
help_cb (BackdropList * bl)
{
  GError *error = NULL;
  char cmd[MAXSTRLEN];

  sprintf (cmd, "%s %s", "xfhelp", HELP_FILE);

  if (!g_spawn_command_line_async (cmd, &error))
    {
      GtkWidget *dialog;
      char text[MAXSTRLEN];
      char *msg;

      msg = g_strcompress (error->message);

      sprintf (text, "%s\n%s\n\n%s\n%s",
	       "The xfhelp script could not be started.",
	       "You may want to check if it's in your PATH.",
	       "The actual error message was:", msg);

      g_free (msg);

      dialog = gtk_message_dialog_new (GTK_WINDOW (bl->window),
				       GTK_DIALOG_MODAL |
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_MESSAGE_WARNING,
				       GTK_BUTTONS_OK, text);
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
}

void
tb_new_cb (GtkButton * b, BackdropList * bl)
{
  new_cb (bl);
}

void
tb_open_cb (GtkButton * b, BackdropList * bl)
{
  open_cb (bl);
}

void
tb_add_cb (GtkButton * b, BackdropList * bl)
{
  add_cb (bl);
}

void
tb_remove_cb (GtkButton * b, BackdropList * bl)
{
  remove_cb (bl);
}

void
tb_help_cb (GtkButton * b, BackdropList * bl)
{
  help_cb (bl);
}
GtkMenu *popup_menu = NULL;

/* popup menus */
typedef struct _Popup Popup;
struct _Popup
{
  char *label;
  gpointer callback;
};

static Popup popup_items[] = {
  {N_("_New list"), new_cb},
  {N_("_Open list ..."), open_cb},
  {"", NULL},
  {N_("Add to list ..."), add_cb},
  {N_("Remove from list"), remove_cb},
  {"", NULL},
  {N_("_Help"), help_cb}
};

static int n_popup_items = sizeof (popup_items) / sizeof (popup_items[0]);

static GtkMenu *
create_popup_menu (BackdropList * bl)
{
  int i;
  GtkWidget *menu = gtk_menu_new ();
  GtkWidget *menu_item;
  Popup *popup = popup_items;

  for (i = 0; i < n_popup_items; i++)
    {
      if (!strlen (popup->label))
	menu_item = gtk_separator_menu_item_new ();
      else
	{
	  menu_item = gtk_menu_item_new_with_mnemonic (_(popup->label));
	  g_signal_connect_swapped (GTK_OBJECT (menu_item), "activate",
				    GTK_SIGNAL_FUNC (popup->callback), bl);
	}

      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
      gtk_widget_show (menu_item);
      popup++;
    }

  return GTK_MENU (menu);
}

static void
do_popup_menu (GdkEventButton *button, gboolean removable, BackdropList * bl)
{
  GList *listitem;

  if (!popup_menu)
    popup_menu = create_popup_menu (bl);

  listitem = g_list_nth (GTK_MENU_SHELL (popup_menu)->children, 4);
  
  if (!removable)
    gtk_widget_set_sensitive (GTK_WIDGET (listitem->data), FALSE);
  else
    gtk_widget_set_sensitive (GTK_WIDGET (listitem->data), TRUE);
  
  gtk_menu_popup (popup_menu, NULL, NULL, NULL, NULL,
		  button->button, button->time);
}

gboolean
button_press_cb (GtkTreeView * treeview, GdkEventButton * button,
		 BackdropList * bl)
{
  GtkTreePath *path;
  gboolean valid_row;

  valid_row = gtk_tree_view_get_path_at_pos (treeview,
					     button->x, button->y,
					     &path, NULL, NULL, NULL);

  if (valid_row)
    {
      gtk_tree_view_set_cursor (treeview, path, NULL, FALSE);
    }
  else
    {
      GtkTreeSelection *sel = gtk_tree_view_get_selection (treeview);

      gtk_tree_selection_unselect_all (sel);
    }

  if (button->button == 3)
    {
      do_popup_menu (button, valid_row, bl);
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

gboolean
key_press_cb (GtkTreeView * treeview, GdkEventKey * key, BackdropList * bl)
{
  if (key->keyval == GDK_Delete)
    {
      remove_cb (bl);
      return TRUE;
    }

  return FALSE;
}

static gboolean
is_list_file (const char *file)
{
  FILE *fp;
  char buf [MAXSTRLEN];
  int size = strlen (LIST_TEXT);
  
  if (!(fp = fopen (file, "r")))
    return FALSE;
  
  if (fgets (buf, size+1, fp) && !strncmp (buf, LIST_TEXT, size))
    {
      fclose (fp);
      return TRUE;
    }
  
  fclose (fp);
  return FALSE;
}

void
on_drag_data_received (GtkWidget * w, GdkDragContext * context,
		       int x, int y, GtkSelectionData * data,
		       guint info, guint time, BackdropList * bl)
{
  char buf[MAXSTRLEN];
  char *file = NULL;
  char *end;

  buf[0] = '\0';
  strncpy (buf, (char *) data->data, MAXSTRLEN - 1);
  buf[MAXSTRLEN - 1] = '\0';

  if ((end = strchr (buf, '\n')))
    *end = '\0';
  
  if ((end = strchr (buf, '\r')))
    *end = '\0';
    if (buf[0])
    {
      file = buf;

      if (!strncmp ("file:", file, 5))
	{
	  file += 5;

	  if (!strncmp ("///", file, 3))
	    file += 2;
	}

      if (is_list_file (file) && 
          (bl->saved || unsaved_changes_dialog (bl)))
        {
          char *temp = bl->filename;
          
          gtk_list_store_clear (bl->list);
          bl->filename = g_strdup (file);
          
          if (!load_list (bl) && temp)
            {
              g_free (bl->filename);
              bl->filename = temp;
              load_list (bl);
            }
          else
            g_free (temp);
              
          
          if (bl->filename)
            gtk_label_set_text (GTK_LABEL (bl->file_label), bl->filename);
        }
      else
        {
            GtkTreeIter iter;
              
            gtk_list_store_append (bl->list, &iter);
            gtk_list_store_set (bl->list, &iter, 0, file, -1);
        }
    }

  gtk_drag_finish (context, (file != NULL),
		   (context->action == GDK_ACTION_MOVE), time);
}
