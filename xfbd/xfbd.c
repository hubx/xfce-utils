/*  xfbd
 *  Copyright (C) 1999,2002 Olivier Fourdan (fourdan@xfce.org)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <time.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <libxfcegui4/libxfcegui4.h>

#include "icons/xfbd_icon.xpm"

#define RCFILE "xfbdrc"
#define LIST_TEXT "# xfce backdrop list"
#define XFBDMGR "xfbdmgr"

#ifndef XFCE_CONFIG
#define XFCE_CONFIG SYSCONFDIR
#endif

#define _(x) x
#define N_(x) x

#ifndef MAXSTRLEN
#define MAXSTRLEN 1024
#endif

#define PREVIEW_WIDTH 500
#define PREVIEW_HEIGHT 300

GdkPixbuf *icon = NULL;

enum
{
  TARGET_STRING,
  TARGET_ROOTWIN,
  TARGET_URL
};

/* Don't use 'text/plain' as target.
 * Otherwise backdrop lists can not be dropped
 */
static GtkTargetEntry xfbd_target_table[] = {
  {"STRING", 0, TARGET_STRING},
  {"text/uri-list", 0, TARGET_URL},
};

static guint n_xfbd_targets =
  sizeof (xfbd_target_table) / sizeof (xfbd_target_table[0]);

enum
{
  TILED,
  STRETCHED,
  AUTO,
  NUM_RADIO_OPTIONS
};

typedef struct
{
  GtkWidget *dialog;
  GdkPixbuf *pixbuf;
  GtkWidget *preview_frame;
  GtkWidget *preview_image;
  GtkWidget *filename_entry;
  GtkWidget *edit_list_button;
  char *filename;
  gboolean is_list;
  GSList *file_list;
  int radiovalue;
}
BackdropDialog;

BackdropDialog *
backdrop_dialog_new (void)
{
  BackdropDialog *bd = g_new (BackdropDialog, 1);

  bd->dialog = NULL;
  bd->pixbuf = NULL;
  bd->preview_frame = NULL;
  bd->preview_image = NULL;
  bd->filename_entry = NULL;
  bd->edit_list_button = NULL;
  bd->filename = NULL;
  bd->is_list = FALSE;
  bd->file_list = NULL;
  bd->radiovalue = AUTO;

  return bd;
}

void
read_config (BackdropDialog * bd)
{
  FILE *fp;
  char buf[MAXSTRLEN];
  const char *home = g_getenv ("HOME");
  char *rcfile = g_strconcat (home, "/.xfce4/", RCFILE, NULL);
  char *systemfile = g_strconcat (XFCE_CONFIG, RCFILE, NULL);
  int i;

  if (!(fp = fopen (rcfile, "r")) && !(fp = fopen (systemfile, "r")))
    {
      g_free (rcfile);
      g_free (systemfile);

      g_printerr (_("xfbd: No configuration file found\n"));
      return;
    }

  for (i = 0; i < 2 && fgets (buf, MAXSTRLEN - 1, fp); i++)
    {
      char *end;

      if ((end = strchr (buf, '\n')))
	*end = '\0';

      switch (i)
	{
	case 0:
	  if (strlen (buf))
	    bd->filename = g_strdup (buf);
	  else
	    i = 2;		/* no filename, quit loop */
	  break;
	case 1:
	  if (!strncmp ("auto", buf, 4))
	    bd->radiovalue = AUTO;
	  else if (!strncmp ("tiled", buf, 5))
	    bd->radiovalue = TILED;
	  else if (!strncmp ("stretched", buf, 9))
	    bd->radiovalue = STRETCHED;
	  break;
	}
    }

  fclose (fp);
  g_free (rcfile);
  g_free (systemfile);
  return;
}

gboolean
write_config (BackdropDialog * bd)
{
  FILE *fp;
  const char *home = g_getenv ("HOME");
  char *rcfile = g_strconcat (home, "/.xfce4/", RCFILE, NULL);
  char *view;

  if (!(fp = fopen (rcfile, "w")))
    {
      g_printerr (_("xfbd: could not write configuration to %s\n"), rcfile);
      g_free (rcfile);
      return FALSE;
    }

  switch (bd->radiovalue)
    {
    case TILED:
      view = "tiled";
      break;
    case STRETCHED:
      view = "stretched";
      break;
    default:
      view = "auto";
      break;
    }

  fprintf (fp, "%s\n", bd->filename);
  fprintf (fp, "%s\n", view);
  fclose (fp);

  g_free (rcfile);
  return TRUE;
}

static Atom prop = 0;

static void
set_root_pixmap_property (Pixmap pix)
{
  Display *d = GDK_DISPLAY ();
  Window w = GDK_ROOT_WINDOW ();

  if (!prop)
    prop = XInternAtom (d, "_XROOTPMAP_ID", False);

  gdk_error_trap_push ();
  XChangeProperty (d, w, prop, XA_PIXMAP, 32, PropModeReplace,
		   (unsigned char *) &pix, 1);
  gdk_flush ();
  if (gdk_error_trap_pop ())
    {
      int error;
      
      gdk_error_trap_push ();
      XChangeProperty (d, w, prop, XA_PIXMAP, 32, PropModeAppend,
		       (unsigned char *) &pix, 1);
      gdk_flush ();
      error = gdk_error_trap_pop ();
      if (error)
        g_printerr (_("xfbd: could not set property: error number %d\n"),
                      error);
    }
}

GdkPixbuf *
get_scaled_pixbuf (GdkPixbuf *pb, 
                   int minwidth, int minheight, 
                   int maxwidth, int maxheight)
{
  int pixwidth = gdk_pixbuf_get_width (pb);
  int pixheight = gdk_pixbuf_get_height (pb);
  int w, h;
  
  if (maxwidth > 0 && maxheight > 0 && 
      (pixwidth > maxwidth || pixheight > maxheight))
    {
      w = maxwidth;
      h = maxheight;
    }
  else if (minwidth > 0 && minheight > 0 && 
           (pixwidth < minwidth || pixheight < minheight))
    {
      w = minwidth;
      h = minheight;
    }
  else
    {
      w = pixwidth;
      h = pixheight;
    }
  
  return gdk_pixbuf_scale_simple (pb, w, h, GDK_INTERP_BILINEAR);
}
    
void
set_backdrop (BackdropDialog * bd)
{
  GdkPixbuf *pb, *pb_scaled;
  GdkPixmap *pix;
  GdkBitmap *mask;

  if (!bd->preview_image)
    return;

  if ((pb = bd->pixbuf))
    {
      int width, height, pixwidth, pixheight;

      switch (bd->radiovalue)
	{
	case TILED:
          pb_scaled = get_scaled_pixbuf (pb, -1, -1, -1, -1);
	  break;
	case STRETCHED:
	  gdk_window_get_geometry (GDK_ROOT_PARENT (), NULL, NULL, &width,
				   &height, NULL);
          pb_scaled = get_scaled_pixbuf (pb, width, height, width, height);
	  break;
	default:
	  gdk_window_get_geometry (GDK_ROOT_PARENT (), NULL, NULL, &width,
				   &height, NULL);
	  pixwidth = gdk_pixbuf_get_width (pb);
	  pixheight = gdk_pixbuf_get_height (pb);
	  if (2 * pixwidth < width || 2 * pixheight < height)
            pb_scaled = get_scaled_pixbuf (pb, -1, -1, -1, -1);
          else
            pb_scaled = get_scaled_pixbuf (pb, width, height, width, height);
	  break;
	}

      gdk_pixbuf_render_pixmap_and_mask (pb_scaled, &pix, &mask, 0);

      gdk_window_set_back_pixmap (GDK_ROOT_PARENT (), pix, 0);
      gdk_window_clear (GDK_ROOT_PARENT ());
      gdk_flush ();

      set_root_pixmap_property (GDK_PIXMAP_XID (pix));

      g_object_unref (pb_scaled);
    }
}

static char *
select_image_file (BackdropDialog * bd)
{
  FILE *fp;
  char buf[MAXSTRLEN];
  char *imagefile = NULL;
  int size = strlen (LIST_TEXT);

  if (!(fp = fopen (bd->filename, "r")))
    return NULL;

  if (!fgets (buf, size + 1, fp))
    {
      fclose (fp);
      return NULL;
    }

  if (strncmp (LIST_TEXT, buf, size))
    {
      /* not a list file, must be a image file */
      fclose (fp);
      bd->is_list = FALSE;
      
      if (bd->dialog)
        gtk_widget_set_sensitive (bd->edit_list_button, FALSE);
      
      return g_strdup (bd->filename);
    }
  else
    {
      int n = 0;
      buf[MAXSTRLEN - 1] = '\0';

      while (fgets (buf, MAXSTRLEN - 1, fp))
	{
	  GdkPixbuf *testpb;
	  char *end;

	  if ((end = strchr (buf, '\n')))
	    *end = '\0';

	  if (g_file_test (buf, G_FILE_TEST_EXISTS) &&
	      (testpb = gdk_pixbuf_new_from_file (buf, NULL)))
	    {
	      n++;
	      g_object_unref (testpb);
	      bd->file_list = g_slist_append (bd->file_list, g_strdup (buf));
	    }
	}

      if (n == 0)
	{
	  fclose (fp);
	  return NULL;
	}
      else
	{
	  int i;
	  GSList *listitem;

          srand (time (0));
	  i = rand () % n;

	  if ((listitem = g_slist_nth (bd->file_list, i)))
	    imagefile = g_strdup (listitem->data);

	  fclose (fp);
          bd->is_list = TRUE;
          
          if (bd->dialog)
            gtk_widget_set_sensitive (bd->edit_list_button, TRUE);
          
	  return imagefile;
	}
    }
}

gboolean
set_preview_image (BackdropDialog * bd)
{
  GdkPixbuf *pb = NULL;
  GdkPixbuf *pb_scaled = NULL;
  char *imagefile = NULL;

  if (!bd->filename || !(imagefile = select_image_file (bd)))
      g_printerr (_("xfbd: no image file\n"));
  else
    {
      GError *error = NULL;	/* don't forget the NULL or gtk will crash :-( */

      pb = gdk_pixbuf_new_from_file (imagefile, &error);
      g_free (imagefile);
      
      if (!pb)
        g_printerr ("xfbd: %s\n", error->message);
    }

  bd->pixbuf = pb;

  if (pb)
    pb_scaled = get_scaled_pixbuf (pb, -1, -1, PREVIEW_WIDTH, PREVIEW_HEIGHT);
  else
    pb_scaled = get_scaled_pixbuf (icon, -1, -1, PREVIEW_WIDTH, PREVIEW_HEIGHT);
    
  if (!bd->preview_image)
    bd->preview_image = gtk_image_new_from_pixbuf (pb_scaled);
  else
    gtk_image_set_from_pixbuf (GTK_IMAGE (bd->preview_image), pb_scaled);

  g_object_unref (pb_scaled);
  
  if (pb)
    return TRUE;
  else
    return FALSE;
}

void
on_drag_data_received (GtkWidget * w, GdkDragContext * context,
		       int x, int y, GtkSelectionData * data,
		       guint info, guint time, BackdropDialog * bd)
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

      g_free (bd->filename);
      bd->filename = g_strdup (file);
      if (set_preview_image (bd))
	gtk_entry_set_text (GTK_ENTRY (bd->filename_entry), bd->filename);
    }

  gtk_drag_finish (context, (file != NULL),
		   (context->action == GDK_ACTION_MOVE), time);
}

void
dialog_response_cb (GtkWidget * d, int response, BackdropDialog * bd)
{
  if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY)
    {
      set_backdrop (bd);

      if (response == GTK_RESPONSE_APPLY)
	return;
      else
	write_config (bd);
    }

  gtk_widget_destroy (bd->dialog);
  gtk_main_quit ();
}

void
dialog_close_cb (GtkWidget * d, BackdropDialog * bd)
{
  gtk_main_quit ();
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

static char *
get_filename_dialog (const char *path, BackdropDialog *bd)
{
  GtkWidget *fs = preview_file_selection_new (_("Select backdrop image"), TRUE);
  char *name = NULL;
  const char *temp;

  if (path)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (fs), path);

  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
			    "clicked", G_CALLBACK (fs_ok_cb), fs);

  g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (fs)->cancel_button),
			    "clicked", G_CALLBACK (fs_cancel_cb), fs);

  gtk_window_set_transient_for (GTK_WINDOW (fs), GTK_WINDOW (bd->dialog));
  
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

static void
browse_cb (GtkWidget * b, BackdropDialog * bd)
{
  char *path;

  path = get_filename_dialog (bd->filename, bd);

  if (path)
    {
      g_free (bd->filename);
      bd->filename = path;
      gtk_entry_set_text (GTK_ENTRY (bd->filename_entry), path);
      set_preview_image (bd);
    }
}

static char *
get_list_from_xfbdmgr (BackdropDialog * bd, gboolean edit)
{
  char *cmd [4];
  char *end;
  char *list = NULL;
  GError *error = NULL; /* this must be NULL or GTK will crash :-( */
  char *out;
  
  cmd[0] = XFBDMGR;
  
  if (edit)
    {
      cmd[1] = "-e";
      cmd[2] = bd->filename;
    }
  else
    {
      cmd[1] = "-n";
      cmd[2] = NULL;
    }
  
  cmd[3] = NULL;
  
  gtk_widget_hide (bd->dialog);
  gdk_flush ();

  if (g_spawn_sync (NULL, cmd, NULL, G_SPAWN_SEARCH_PATH, 
                    NULL, NULL, &out, NULL, NULL, &error))
    {
      if ((end = strchr (out, '\n')))
        *end = '\0';
          
      if (strlen (out))
        list = g_strdup (out);
    }
  else
    {
      char *msg = g_strcompress (error->message);
      g_printerr ("xfbd: %s\n", msg);
      g_free (msg);
    }

  gtk_widget_show (bd->dialog);
  return list;
}

static void
new_list_cb (GtkWidget * b, BackdropDialog * bd)
{
  char *newlist;

  newlist = get_list_from_xfbdmgr (bd, FALSE);
  
  if (newlist)
    {
      g_free (bd->filename);
      bd->filename = newlist;
      
      if (set_preview_image (bd))
	gtk_entry_set_text (GTK_ENTRY (bd->filename_entry), bd->filename);
    }
}

static void
edit_list_cb (GtkWidget * b, BackdropDialog * bd)
{
  char *list;
  
  g_free (bd->filename);
  bd->filename = g_strdup (gtk_entry_get_text (GTK_ENTRY (bd->filename_entry)));
  
  list = get_list_from_xfbdmgr (bd, TRUE);
  
  if (list)
    {
      g_free (bd->filename);
      bd->filename = list;
      
      if (set_preview_image (bd))
	gtk_entry_set_text (GTK_ENTRY (bd->filename_entry), bd->filename);
    }
}

static void
rb_tiled_clicked_cb (GtkWidget * b, BackdropDialog * bd)
{
  bd->radiovalue = TILED;
}

static void
rb_stretched_clicked_cb (GtkWidget * b, BackdropDialog * bd)
{
  bd->radiovalue = STRETCHED;
}

static void
rb_auto_clicked_cb (GtkWidget * b, BackdropDialog * bd)
{
  bd->radiovalue = AUTO;
}

void
create_dialog (BackdropDialog * bd)
{
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *file_label;
  GtkWidget *view_label;
  GtkWidget *browse_button;
  GtkWidget *buttonbox;
  GtkWidget *hbox;
  GtkWidget *new_list_button;
  GtkWidget *rb_tiled;
  GtkWidget *rb_stretched;
  GtkWidget *rb_auto;

  bd->dialog = gtk_dialog_new_with_buttons ("Backdrop Configuration",
					    NULL, 0,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_OK,
					    GTK_STOCK_APPLY,
					    GTK_RESPONSE_APPLY,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (bd->dialog), GTK_RESPONSE_OK);
  g_signal_connect (G_OBJECT (bd->dialog), "response",
		    G_CALLBACK (dialog_response_cb), bd);
  g_signal_connect (G_OBJECT (bd->dialog), "close",
		    G_CALLBACK (dialog_close_cb), bd);

  g_object_set (G_OBJECT (bd->dialog), "window-position",
		GTK_WIN_POS_CENTER, NULL);
  gtk_window_set_icon (GTK_WINDOW (bd->dialog), icon);

  vbox = gtk_vbox_new (FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (bd->dialog)->vbox),
		      vbox, FALSE, TRUE, 0);

  bd->preview_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (bd->preview_frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox), bd->preview_frame, TRUE, TRUE, 0);
  gtk_widget_set_size_request (bd->preview_frame, 
                               PREVIEW_WIDTH, PREVIEW_HEIGHT);

  gtk_container_add (GTK_CONTAINER (bd->preview_frame), bd->preview_image);

  gtk_drag_dest_set (bd->preview_frame, GTK_DEST_DEFAULT_ALL,
		     xfbd_target_table, n_xfbd_targets,
		     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  g_signal_connect (G_OBJECT (bd->preview_frame), "drag_data_received",
		    G_CALLBACK (on_drag_data_received), bd);

  table = gtk_table_new (3, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

  file_label = gtk_label_new ("File:");
  gtk_misc_set_alignment (GTK_MISC (file_label), 0, 0.5);

  bd->filename_entry = gtk_entry_new ();
  if (bd->filename)
    gtk_entry_set_text (GTK_ENTRY (bd->filename_entry), bd->filename);
  else
    gtk_entry_set_text (GTK_ENTRY (bd->filename_entry), "");

  buttonbox = gtk_hbutton_box_new ();
  gtk_box_set_spacing (GTK_BOX(buttonbox), 8);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox), GTK_BUTTONBOX_START);
  
  browse_button = gtk_button_new_with_mnemonic ("_Browse...");
  new_list_button = gtk_button_new_with_label (_("New list"));
  bd->edit_list_button = gtk_button_new_with_label (_("Edit list"));
  
  gtk_box_pack_start (GTK_BOX (buttonbox), browse_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (buttonbox), new_list_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (buttonbox), bd->edit_list_button, 
                      FALSE, FALSE, 0);
  
  g_signal_connect (G_OBJECT (browse_button), "clicked",
		    G_CALLBACK (browse_cb), bd);

  g_signal_connect (G_OBJECT (new_list_button), "clicked",
                    G_CALLBACK (new_list_cb), bd);
  
  g_signal_connect (G_OBJECT (bd->edit_list_button), "clicked",
                    G_CALLBACK (edit_list_cb), bd);

  if (!bd->is_list)
    gtk_widget_set_sensitive (bd->edit_list_button, FALSE);

  view_label = gtk_label_new ("Show:");
  gtk_misc_set_alignment (GTK_MISC (view_label), 0, 0.5);

  hbox = gtk_hbox_new (FALSE, 4);

  rb_tiled = gtk_radio_button_new_with_mnemonic (NULL, "_Tiled");
  gtk_box_pack_start (GTK_BOX (hbox), rb_tiled, FALSE, FALSE, 4);

  rb_stretched = gtk_radio_button_new_with_mnemonic_from_widget
    (GTK_RADIO_BUTTON (rb_tiled), "_Stretched");
  gtk_box_pack_start (GTK_BOX (hbox), rb_stretched, FALSE, FALSE, 4);

  rb_auto = gtk_radio_button_new_with_mnemonic_from_widget
    (GTK_RADIO_BUTTON (rb_tiled), "A_utomatic");
  gtk_box_pack_start (GTK_BOX (hbox), rb_auto, FALSE, FALSE, 4);

  gtk_table_attach (GTK_TABLE (table), file_label, 0, 1, 0, 1,
		    GTK_FILL, 0, 2, 4);
  gtk_table_attach (GTK_TABLE (table), bd->filename_entry, 1, 2, 0, 1,
		    GTK_EXPAND | GTK_FILL, 0, 2, 4);

  gtk_table_attach (GTK_TABLE (table), buttonbox, 1, 2, 1, 2,
		    GTK_EXPAND | GTK_FILL, 0, 2, 4);

  gtk_table_attach (GTK_TABLE (table), view_label, 0, 1, 2, 3,
		    GTK_FILL, 0, 2, 4);
  gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, 2, 3,
		    GTK_EXPAND | GTK_FILL, 0, 2, 4);

  switch (bd->radiovalue)
    {
    case TILED:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_tiled), TRUE);
      break;
    case STRETCHED:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_stretched), TRUE);
      break;
    default:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rb_auto), TRUE);
      break;
    }

  g_signal_connect (G_OBJECT (rb_tiled), "clicked",
		    G_CALLBACK (rb_tiled_clicked_cb), bd);
  g_signal_connect (G_OBJECT (rb_stretched), "clicked",
		    G_CALLBACK (rb_stretched_clicked_cb), bd);
  g_signal_connect (G_OBJECT (rb_auto), "clicked",
		    G_CALLBACK (rb_auto_clicked_cb), bd);

  gtk_widget_show_all (bd->dialog);
}

void
usage (const char *name)
{
  fprintf (stderr, _("Usage : %s [OPTIONS]\n"), name);
  fprintf (stderr, _("   Where OPTIONS are :\n"));
  fprintf (stderr,
	   _("   -i : interactive, prompts for backdrop to display\n"));
  fprintf (stderr,
	   _("   -d : display, reads configuration and exit (default)\n\n"));
  fprintf (stderr,
	   _
	   ("%s is part of the XFce distribution, written by Olivier Fourdan\n\n"),
	   name);
}

int
main (int argc, char **argv)
{
  BackdropDialog *bd = backdrop_dialog_new ();

  gtk_init (&argc, &argv);

  read_config (bd);
  icon = gdk_pixbuf_new_from_xpm_data (xfbd_icon_xpm);

  if (argc > 2 && g_file_test (argv[argc - 1], G_FILE_TEST_EXISTS))
    {
      g_free (bd->filename);
      bd->filename = g_strdup (argv[argc - 1]);
      bd->radiovalue = AUTO;

      set_preview_image (bd);
      set_backdrop (bd);
      return 0;
    }

  if (argc == 1 || (argc == 2 && !strncmp ("-d", argv[1], 2)))
    {
      set_preview_image (bd);
      set_backdrop (bd);
      return 0;
    }

  if (argc == 2 && !strncmp ("-i", argv[1], 2))
    {

      set_preview_image (bd);
      create_dialog (bd);

      gtk_main ();
      return 0;
    }

  usage (argv[0]);
  return 0;
}
