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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xfbdmgr.h"
#include "xfbdmgr_cb.h"

#ifndef XFCE_DATA
#define XFCE_DATA DATADIR
#endif

enum
{
  TARGET_STRING,
  TARGET_ROOTWIN,
  TARGET_URL
};

static GtkTargetEntry xfbdmgr_target_table[] = {
  {"STRING", 0, TARGET_STRING},
  {"text/uri-list", 0, TARGET_URL},
};

static guint n_xfbdmgr_targets =
  sizeof (xfbdmgr_target_table) / sizeof (xfbdmgr_target_table[0]);

static BackdropList *
backdrop_list_new (void)
{
  BackdropList *bl = g_new (BackdropList, 1);

  bl->filename = NULL;
  bl->last_path = g_strconcat (XFCE_DATA, "/backdrops/", NULL);
  bl->saved = TRUE;
  bl->window = NULL;
  bl->file_label = NULL;
  bl->list = NULL;
  bl->num_items = 0;
  bl->treeview = NULL;

  return bl;
}

static void
fs_ok_cb (GtkDialog * fs)
{
  gtk_dialog_response (fs, GTK_RESPONSE_OK);
}

static void
fs_cancel_cb (GtkDialog * fs)
{
  gtk_dialog_response (fs, GTK_RESPONSE_CANCEL);
}

char *
get_filename_dialog (const char *title, const char *path)
{
  const char *t = title ? title : _("Select file");
  GtkWidget *fs = gtk_file_selection_new (t);
  char *name = NULL;
  const char *temp;

  if (path)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (fs), path);
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
			    "clicked", G_CALLBACK (fs_ok_cb), fs);

  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (fs)->cancel_button),
			    "clicked", G_CALLBACK (fs_cancel_cb), fs);

  gtk_window_set_position (GTK_WINDOW (fs), GTK_WIN_POS_CENTER);
  
  if (gtk_dialog_run (GTK_DIALOG (fs)) == GTK_RESPONSE_OK)
    {
      temp = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));

      if (temp && strlen (temp))
	name = g_strdup (temp);
      else
	name = NULL;
    }

  gtk_widget_destroy (fs);

  return name;
}

static gboolean
retry_saving_dialog (BackdropList * bl)
{
  GtkWidget *dialog;
  char text[MAXSTRLEN];
  int response = GTK_RESPONSE_NO;

  sprintf (text, 
           _("There was a problem saving file %s\n\n"
             "Do you want to choose a new location?"),
           bl->filename);

  dialog = gtk_message_dialog_new (GTK_WINDOW (bl->window),
				   GTK_DIALOG_MODAL |
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_WARNING,
				   GTK_BUTTONS_YES_NO, text);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  switch (response)
    {
    case GTK_RESPONSE_YES:
      return TRUE;
      break;
    default:
      return FALSE;
      break;
    }

}

gboolean
save_list (BackdropList * bl)
{
  if (!bl->num_items)
    return FALSE;

  if (!bl->filename)
    return save_list_as (bl);
  else
    {
      FILE *fp;

      if (!(fp = fopen (bl->filename, "w"))) 
        {
          if (retry_saving_dialog (bl))
            return save_list_as (bl);
          else
            return FALSE;
        }
      else
	{
	  GtkTreePath *tp = gtk_tree_path_new_first ();
	  int i;

	  fprintf (fp, "%s\n", LIST_TEXT);

	  for (i = 0; i < bl->num_items; i++)
	    {
	      GtkTreeModel *m = GTK_TREE_MODEL (bl->list);
	      GtkTreeIter it;
	      char *name;

	      gtk_tree_model_get_iter (m, &it, tp);
	      gtk_tree_model_get (m, &it, 0, &name, -1);

	      fprintf (fp, "%s\n", name);

	      g_free (name);
	      gtk_tree_path_next (tp);
	    }

	  bl->saved = TRUE;
	  fclose (fp);
	  return TRUE;
	}
    }
}

static gboolean
confirm_overwrite (const char *file, GtkWidget * window)
{
  GtkWidget *dialog;
  char text[MAXSTRLEN];
  int response = GTK_RESPONSE_NO;

  sprintf (text, 
           _("File %s already exist\n\n"
             "Do you want to overwrite this file?"),
	   file);

  dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				   GTK_DIALOG_MODAL |
				   GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_WARNING,
				   GTK_BUTTONS_YES_NO, text);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  switch (response)
    {
    case GTK_RESPONSE_YES:
      return TRUE;
      break;
    default:
      return FALSE;
      break;
    }
}

gboolean
save_list_as (BackdropList * bl)
{
  char *path;
  char *filename;

  if (!bl->num_items)
    return FALSE;

  path = bl->filename ? bl->filename : _("new.list");
  filename = get_filename_dialog (_("Save as ..."), path);

  if (!filename)
    return FALSE;

  if (g_file_test (filename, G_FILE_TEST_EXISTS) &&
      !confirm_overwrite (filename, bl->window))
    {
      g_free (filename);
      return save_list_as (bl);
    }

  g_free (bl->filename);
  bl->filename = filename;
  if (save_list (bl))
    {
      gtk_label_set_text (GTK_LABEL (bl->file_label), bl->filename);
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

gboolean
load_list (BackdropList * bl)
{
  FILE *fp;
 
  if (!bl->filename)
    return FALSE;

  if (!(fp = fopen (bl->filename, "r")))
    {
      g_free (bl->filename);
      bl->filename = NULL;
      return FALSE;
    }
  else
    {
      char buf[MAXSTRLEN];
      char *end;
      int size = strlen (LIST_TEXT);

      if (!fgets (buf, MAXSTRLEN-1, fp) || strncmp (buf, LIST_TEXT, size))
	{
	  g_free (bl->filename);
          bl->filename = NULL;
	  fclose (fp);
	  return FALSE;
	}

      buf[MAXSTRLEN - 1] = '\0';

      while (fgets (buf, MAXSTRLEN - 1, fp))
	{
	  GtkTreeIter it;
	  if ((end = strchr (buf, '\n')))
	    *end = '\0';

	  gtk_list_store_append (bl->list, &it);
	  gtk_list_store_set (bl->list, &it, 0, buf, -1);
	  bl->num_items++;
	}

      bl->saved = TRUE;
      
      if (bl->save_button)
        {
          gtk_widget_set_sensitive (bl->save_button, FALSE);
          gtk_label_set_text (GTK_LABEL (bl->file_label), bl->filename);
        }
        
      fclose (fp);
      return TRUE;
    }
}

static void
create_treeview (BackdropList * bl)
{
  GtkTreeSelection *sel;
  GtkTreeModel *model = GTK_TREE_MODEL (bl->list);
  GtkTreeViewColumn *col;
  GtkCellRenderer *cr;

  bl->treeview = gtk_tree_view_new_with_model (model);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (bl->treeview), TRUE);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (bl->treeview));
  gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);

  cr = gtk_cell_renderer_text_new ();
  col =
    gtk_tree_view_column_new_with_attributes (_("Image files"), cr,
					      "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (bl->treeview), col);

  /* DND */
  gtk_drag_dest_set (bl->treeview, GTK_DEST_DEFAULT_ALL,
		     xfbdmgr_target_table, n_xfbdmgr_targets,
		     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  g_signal_connect (G_OBJECT (bl->treeview), "drag_data_received",
		    G_CALLBACK (on_drag_data_received), bl);

  
  /* other signals */
  g_signal_connect (G_OBJECT (bl->treeview), "button_press_event",
		    G_CALLBACK (button_press_cb), bl);

  g_signal_connect (G_OBJECT (bl->treeview), "key_press_event",
		    G_CALLBACK (key_press_cb), bl);
}

static GtkWidget *
create_toolbar (BackdropList * bl)
{
  GtkWidget *toolbar = gtk_toolbar_new ();
  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_NEW,
			    _("New list ..."),
			    "Create new backdrop list",
			    (GtkSignalFunc) tb_new_cb, bl, -1);

  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_OPEN,
			    _("Open list ..."),
			    "Open backdrop list file",
			    (GtkSignalFunc) tb_open_cb, bl, -1);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_ADD,
			    _("Add to list"),
			    "Add image file to list",
			    (GtkSignalFunc) tb_add_cb, bl, -1);

  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_DELETE, 
                            _("Remove from list"),
			    "Remove image file from list",
			    (GtkSignalFunc) tb_remove_cb, bl, -1);

  gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

  gtk_toolbar_insert_stock (GTK_TOOLBAR (toolbar), GTK_STOCK_HELP,
			    _("Help"), 
                            "Open documentation",
			    (GtkSignalFunc) tb_help_cb, bl, -1);

  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_orientation (GTK_TOOLBAR (toolbar),
			       GTK_ORIENTATION_HORIZONTAL);

  return toolbar;
}

static void
create_xfbdmgr (BackdropList * bl)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *sw;
  GtkWidget *button_box;
  GtkWidget *save_as_button;
  GtkWidget *quit_button;
  GtkWidget *toolbar;
  
  bl->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (bl->window), _("Backdrop List Manager"));
  gtk_window_set_default_size (GTK_WINDOW (bl->window), 350, 400);
  gtk_window_set_position (GTK_WINDOW (bl->window), GTK_WIN_POS_CENTER);
  
  g_signal_connect (G_OBJECT (bl->window), "destroy", G_CALLBACK (quit_cb), bl);
  
  vbox = gtk_vbox_new (FALSE, 8);
  gtk_container_add (GTK_CONTAINER (bl->window), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
  
  /* toolbar */
  toolbar = create_toolbar (bl);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);
  
  /* show file name */
  
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  
  label = gtk_label_new (_("File:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  
  bl->file_label = gtk_label_new (bl->filename ? bl->filename : _("None"));
  gtk_misc_set_alignment (GTK_MISC (bl->file_label), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), bl->file_label, TRUE, TRUE, 0);
  
  /* scrollwindow for tree */
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  
  create_treeview (bl);
  gtk_container_add (GTK_CONTAINER (sw), bl->treeview);
  
  /* label */
  gtk_box_pack_start (GTK_BOX (vbox), 
		      gtk_label_new (_("Use drag and drop or right click menu")),
		      FALSE, FALSE, 4);
                      
  /* separator */
  gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), FALSE, TRUE, 4);
  
  /* buttons */
  button_box = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX (vbox), button_box, FALSE, TRUE, 0);
  gtk_box_set_spacing (GTK_BOX (button_box), 8);
  
  bl->save_button = gtk_button_new_from_stock (GTK_STOCK_SAVE);
  save_as_button = gtk_button_new_from_stock (GTK_STOCK_SAVE_AS);
  quit_button = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  
  gtk_box_pack_start (GTK_BOX (button_box), quit_button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (button_box), bl->save_button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (button_box), save_as_button, FALSE, FALSE, 2);
                      
  gtk_button_box_set_layout (GTK_BUTTON_BOX (button_box), GTK_BUTTONBOX_END);
  
  g_signal_connect (G_OBJECT (quit_button), "clicked",
		    G_CALLBACK (quit_cb), bl);
                    
  g_signal_connect (G_OBJECT (bl->save_button), "clicked",
		    G_CALLBACK (save_cb), bl);
                    
  g_signal_connect (G_OBJECT (save_as_button), "clicked",
		    G_CALLBACK (save_as_cb), bl);
                    
  gtk_widget_set_sensitive (bl->save_button, FALSE);
  
  gtk_widget_show_all (bl->window);
}

int
main (int argc, char **argv)
{
  gboolean echo_filename = FALSE;
  BackdropList *bl = backdrop_list_new ();
  
  gtk_init (&argc, &argv);
  
  if (argc == 2)
    {
      if (!strncmp ("-n", argv[1], 2))
	echo_filename = TRUE;
      else if (g_file_test (argv[1], G_FILE_TEST_EXISTS))
	bl->filename = g_strdup (argv[1]);
    }
  else
    if (argc == 3 && !strncmp ("-e", argv[1], 2) &&
	g_file_test (argv[2], G_FILE_TEST_EXISTS))
    {
      echo_filename = TRUE;
      bl->filename = g_strdup (argv[2]);
    }

  bl->list = gtk_list_store_new (1, G_TYPE_STRING);
    
  if (bl->filename)
    {
      if (!load_list (bl))
	{
	  g_printerr (_("xfbdmgr: could not load file %s\n"), 
                      bl->filename);
	  g_free (bl->filename);
	}
    }

  create_xfbdmgr (bl);
    
  gtk_main ();
    
  if (echo_filename && bl->filename)
    g_print ("%s\n", bl->filename);
  
  g_free (bl->filename);
  return 0;
}
