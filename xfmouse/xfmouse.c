/*  xfmouse
 *  Copyright (C) 1999, 2002 Olivier Fourdan (fourdan@xfce.org)
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
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "icons/xfmouse.xpm"
#include "icons/xfmouse_icon.xpm"

#define ACCEL_MIN	1
#define ACCEL_MAX	30
#define DENOMINATOR	3
#define THRESH_MIN	1
#define THRESH_MAX	20
#define RCFILE          "xfmouserc"

#ifndef XFCE_CONFIG
#define XFCE_CONFIG SYSCONFDIR
#endif

#define _(x) x
#define N_(x) x

#ifndef MAXSTRLEN
#define MAXSTRLEN 1024
#endif

typedef struct
{
  GtkWidget *dialog;
  GtkWidget *rb_right;
  int radiovalue;
  GtkWidget *accel;
  int accelvalue;
  GtkWidget *threshold;
  int threshvalue;
}
MouseDialog;

MouseDialog *
mouse_dialog_new (void)
{
  MouseDialog *md = g_new (MouseDialog, 1);

  md->dialog = NULL;
  md->rb_right = NULL;
  md->radiovalue = 0;
  md->accel = NULL;
  md->accelvalue = 0;
  md->threshold = NULL;
  md->threshvalue = 0;

  return md;
}

void
set_defaults (MouseDialog * md)
{
  md->radiovalue = 1;		/* right handed mouse */
  md->accelvalue = 2 * DENOMINATOR;
  md->threshvalue = 4;
}

void
read_config (MouseDialog * md)
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
      return;
    }

  for (i = 0; i < 3 && fgets (buf, MAXSTRLEN - 1, fp); i++)
    {
      char *end;
      int val;

      if ((end = strchr (buf, '\n')))
	*end = '\0';

      switch (i)
	{
	case 0:
	  md->radiovalue = (strncmp ("Left", buf, 4) == 0) ? 0 : 1;
	  break;
	case 1:
	  val = atoi (buf);
	  md->accelvalue = (val < ACCEL_MIN) ? ACCEL_MIN :
	    (val > ACCEL_MAX) ? ACCEL_MAX : val;
	  break;
	case 2:
	  val = atoi (buf);
	  md->threshvalue = (val < THRESH_MIN) ? THRESH_MIN :
	    (val > THRESH_MAX) ? THRESH_MAX : val;
	  break;
	}
    }

  fclose (fp);
  g_free (rcfile);
  g_free (systemfile);
  return;
}

gboolean
write_config (MouseDialog * md)
{
  FILE *fp;
  const char *home = g_getenv ("HOME");
  char *rcfile = g_strconcat (home, "/.xfce4/", RCFILE, NULL);

  if (!(fp = fopen (rcfile, "w")))
    {
      g_printerr (_("xfmouse: could not write configuration to %s\n"),
                  rcfile);
      g_free (rcfile);
      return FALSE;
    }

  fprintf (fp, "%s\n", ((md->radiovalue) ? "Right" : "Left"));
  fprintf (fp, "%i\n", md->accelvalue);
  fprintf (fp, "%i\n", md->threshvalue);
  fclose (fp);

  g_free (rcfile);
  return TRUE;
}

void
set_mouse_values (MouseDialog * md)
{
  unsigned char map[5] = "\0\0\0\0\0";
  unsigned int buttons = 0;
  Display *tmpDpy;

  if (!(tmpDpy = XOpenDisplay ("")))
    {
      g_printerr (_("xfmouse: Error, cannot open display.\n"));
      g_printerr (_("Is X running and $DISPLAY correctly set ?\n"));
      return;
    }
  XSync (tmpDpy, False);
  XChangePointerControl (tmpDpy, 1, 1, md->accelvalue, DENOMINATOR,
			 md->threshvalue);
  buttons = XGetPointerMapping (tmpDpy, map, 5);
  if (md->radiovalue)		/* Right handed */
    {
      if (buttons > 2)
	{
	  map[0] = 1;
	  map[1] = 2;
	  map[2] = 3;
	}
      else
	{
	  map[0] = 2;
	  map[1] = 1;
	}
    }
  else				/* Left handed */
    {
      if (buttons > 2)
	{
	  map[0] = 3;
	  map[1] = 2;
	  map[2] = 1;
	}
      else
	{
	  map[0] = 1;
	  map[1] = 2;
	}
    }
  XSetPointerMapping (tmpDpy, map, buttons);
  XFlush (tmpDpy);
  XCloseDisplay (tmpDpy);
}

static void
dialog_response_cb (GtkWidget * d, int response, MouseDialog * md)
{
  if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY)
    {
      md->accelvalue = (int) gtk_range_get_value (GTK_RANGE (md->accel));
      md->threshvalue = (int) gtk_range_get_value (GTK_RANGE (md->threshold));
      set_mouse_values (md);

      if (response == GTK_RESPONSE_APPLY)
	return;
      else
	write_config (md);
    }

  gtk_widget_destroy (md->dialog);
  gtk_main_quit ();
}

static void
dialog_close_cb (GtkWidget * d, MouseDialog * md)
{
  gtk_main_quit ();
}

static void
radio_button_toggled_cb (GtkWidget * cb, MouseDialog * md)
{
  md->radiovalue = md->radiovalue ? 0 : 1;
}


GtkWidget *
create_image_from_xpm_data (const char **data)
{
  GdkPixbuf *pb = gdk_pixbuf_new_from_xpm_data (data);
  GtkWidget *im = gtk_image_new_from_pixbuf (pb);

  return im;
}

void
create_dialog (MouseDialog * md)
{
  GtkWidget *vbox;
  GtkWidget *frame1;
  GtkWidget *frame2;
  GtkWidget *hbox1;
  GtkWidget *table;
  GtkWidget *rb_left;
  GtkWidget *accel_label;
  GtkWidget *thresh_label;
  GtkWidget *im;

  md->dialog = gtk_dialog_new_with_buttons (_("Mouse Configuration"),
					    NULL, 0,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_OK,
					    GTK_STOCK_APPLY,
					    GTK_RESPONSE_APPLY,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (md->dialog), GTK_RESPONSE_OK);
  g_signal_connect (G_OBJECT (md->dialog), "response",
		    G_CALLBACK (dialog_response_cb), md);
  g_signal_connect (G_OBJECT (md->dialog), "close",
		    G_CALLBACK (dialog_close_cb), md);

  gtk_window_set_default_size (GTK_WINDOW (md->dialog), 400, 10);
  gtk_window_set_position (GTK_WINDOW (md->dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_icon (GTK_WINDOW (md->dialog),
		       gdk_pixbuf_new_from_xpm_data (xfmouse_icon_xpm));
                       
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (md->dialog)->vbox), vbox, FALSE, TRUE, 0);
                      
  frame1 = gtk_frame_new (_("Button Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start (GTK_BOX (vbox), frame1, FALSE, TRUE, 0);
  
  hbox1 = gtk_hbox_new (TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 8);
  gtk_container_add (GTK_CONTAINER (frame1), hbox1);
  
  im = create_image_from_xpm_data ((const char **)xfmouse_xpm);
  gtk_box_pack_start (GTK_BOX (hbox1), im, FALSE, FALSE, 0);
  
  rb_left = gtk_radio_button_new_with_mnemonic (NULL, _("_Left"));
  md->rb_right =
    gtk_radio_button_new_with_mnemonic_from_widget
    (GTK_RADIO_BUTTON (rb_left), _("_Right"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				(md->rb_right), md->radiovalue);
  g_signal_connect (G_OBJECT (md->rb_right), "toggled",
		    G_CALLBACK (radio_button_toggled_cb), md);
  gtk_box_pack_start (GTK_BOX (hbox1), rb_left, FALSE, FALSE, 4);
  gtk_box_pack_start (GTK_BOX (hbox1), md->rb_right, FALSE, FALSE, 4);
  
  frame2 = gtk_frame_new (_("Motion Settings"));
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, TRUE, 0);
  
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 8);
  gtk_container_add (GTK_CONTAINER (frame2), table);
  
  accel_label = gtk_label_new (_("Acceleration: "));
  gtk_misc_set_alignment (GTK_MISC (accel_label), 0, 0);
  
  md->accel = gtk_hscale_new_with_range (ACCEL_MIN, ACCEL_MAX, 1);
  gtk_range_set_value (GTK_RANGE (md->accel), md->accelvalue);
  
  gtk_table_attach (GTK_TABLE (table), accel_label, 0, 1, 0, 1, 0, 0, 2, 4);
  gtk_table_attach (GTK_TABLE (table), md->accel, 1, 2,
		    0, 1, GTK_FILL | GTK_EXPAND, 0, 2, 4);
                    
  thresh_label = gtk_label_new (_("Threshold: "));
  gtk_misc_set_alignment (GTK_MISC (thresh_label), 0, 0);
  
  md->threshold = gtk_hscale_new_with_range (THRESH_MIN, THRESH_MAX, 1);
  gtk_range_set_value (GTK_RANGE (md->threshold), md->threshvalue);
  
  gtk_table_attach (GTK_TABLE (table), thresh_label, 0, 1, 1, 2, 0, 0, 2, 4);
  gtk_table_attach (GTK_TABLE (table), md->threshold, 1,
		    2, 1, 2, GTK_FILL | GTK_EXPAND, 0, 2, 4);
                    
  gtk_widget_show_all (md->dialog);
}

void
usage (const char *name)
{
  g_printerr (_("Usage : %s [OPTIONS]\n"), name);
  g_printerr (_("   Where OPTIONS are :\n"));
  g_printerr (_("   -i : interactive\n"));
  g_printerr (_("   -d : apply configuration and exit (default)\n\n"));
  g_printerr (_("%s is part of the XFce distribution, "
                "written by Olivier Fourdan\n\n"), name);
}

int
main (int argc, char **argv)
{
  MouseDialog *md = mouse_dialog_new ();
  
  set_defaults (md);
  read_config (md);
  
  if (argc == 2 && strcmp (argv[1], "-i") == 0)
    {
      gtk_init (&argc, &argv);
      create_dialog (md);
      gtk_main ();
      return 0;
    }

  if ((argc == 1) || (argc == 2 && strcmp (argv[1], "-d") == 0))
    {
      set_mouse_values (md);
      return 0;
    }

  usage (argv[0]);
  return 0;
}
