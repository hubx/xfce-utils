/*  xfrun
 *  Copyright (C) 2000, 2002 Olivier Fourdan (fourdan@xfce.org)
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

#ifndef SHELL
#define SHELL "/bin/sh"
#endif

#define _(x) x
#define N_(x) x

#define HFILE ".xfce4/xfrun_history"

#define MAXHISTORY 10

/* typedefs */
typedef struct
{
  GtkWidget *dialog;
  GtkWidget *combo_entry;
  GList *history;
  gboolean in_terminal;
}
RunDialog;

static RunDialog *
run_dialog_new (void)
{
  RunDialog *rd = g_new (RunDialog, 1);

  rd->dialog = NULL;
  rd->combo_entry = NULL;
  rd->history = NULL;
  rd->in_terminal = 0;

  return rd;
}

GList *
get_history (void)
{
  FILE *fp;
  const char *home = g_getenv ("HOME");
  char *hfile = g_strconcat (home, "/", HFILE, NULL);
  GList *cbtemp = NULL;
  char line[MAXSTRLEN];
  char *check;
  int i = 0;

  if (!(fp = fopen (hfile, "r")))
    {
      g_free (hfile);
      cbtemp = g_list_append (cbtemp, "");

      return cbtemp;
    }

  line[MAXSTRLEN - 1] = '\0';

  /* no more than 10 history items */
  for (i = 0; i < MAXHISTORY && fgets (line, MAXSTRLEN - 1, fp); i++)
    {
      if ((line[0] == '\0') || (line[0] == '\n'))
	break;

      if ((check = strrchr (line, '\n')))
	*check = '\0';

      cbtemp = g_list_append (cbtemp, g_strdup (line));
    }

  g_free (hfile);
  fclose (fp);

  if (!cbtemp)
    cbtemp = g_list_append (cbtemp, "");

  return cbtemp;
}

void
put_history (const char *newest, GList * cb)
{
  FILE *fp;
  const char *home = g_getenv ("HOME");
  char *hfile = g_strconcat (home, "/", HFILE, NULL);
  GList *node;
  int i;

  if (!(fp = fopen (hfile, "w")))
    {
      g_print ("xfrun: Could not write history to file %s\n", hfile);
      g_free (hfile);
      return;
    }

  fprintf (fp, "%s\n", newest);
  i = 1;

  for (node = cb; node && i < MAXHISTORY; node = node->next)
    {
      char *cmd = (char *) node->data;

      if (cmd && *cmd != '\0' && (strcmp (cmd, newest) != 0))
	{
	  fprintf (fp, "%s\n", cmd);
	  i++;
	}
    }

  fclose (fp);
  g_free (hfile);
}

static gboolean
exec_comm (char *cmd, char **msg)
{
  GError *error = NULL;
  char *realcmd;
  gboolean retval;

  /* if xfrun were made a module we could also add 
   * Module and KillModule directives.
   * Well, just a thought (jasper) 
   */
  if (strncmp ("Term", cmd, 4) == 0)
    {
      char buf[MAXSTRLEN];
      char *tmpcmd, *end, *options = NULL;

      cmd += 5;

      snprintf (buf, MAXSTRLEN - 1, "%s", cmd);

      if ((end = strchr (buf, ' ')))
	{
	  *end = '\0';
	  end++;
	  options = end;
	}

      if ((tmpcmd = g_find_program_in_path (buf)))
	{
	  if (options)
	    realcmd = g_strconcat ("xfterm -e \"", tmpcmd, " ", options, "\"",
				   NULL);
	  else
	    realcmd = g_strconcat ("xfterm -e \"", tmpcmd, "\"", NULL);
	  g_free (tmpcmd);
	}
      else
	{
	  *msg = g_strconcat ("Program ", "\"", cmd, "\" not found in path",
			      NULL);
	  return FALSE;
	}

    }
  else
    realcmd = g_strdup (cmd);

  retval = g_spawn_command_line_async (realcmd, &error);

  g_free (realcmd);

  if (retval)
    return retval;
  else
    {
      *msg = g_strdup (error->message);
      return retval;
    }
}

static void
dialog_response_cb (GtkWidget * d, int response, RunDialog * rd)
{
  char *msg;

  if (response == GTK_RESPONSE_OK)
    {
      char buf[MAXSTRLEN];
      const char *command = gtk_entry_get_text (GTK_ENTRY (rd->combo_entry));

      if (rd->in_terminal)
	snprintf (buf, MAXSTRLEN - 1, "Term %s", command);
      else
	snprintf (buf, MAXSTRLEN - 1, "%s", command);

      buf[MAXSTRLEN - 1] = '\0';
      if (exec_comm (buf, &msg))
	put_history (command, rd->history);
      else
	{
	  fprintf (stderr, "xfrun: %s\n", msg);
	  g_free (msg);
	  return;
	}
    }

  gtk_widget_destroy (rd->dialog);
  gtk_main_quit ();
}

static void
dialog_close_cb (GtkWidget * d, RunDialog * rd)
{
  /* dialog_response_cb (d, GTK_RESPONSE_CANCEL, rd); */
  gtk_main_quit ();
}

static void
checkbox_toggled_cb (GtkWidget * cb, RunDialog * rd)
{
  rd->in_terminal = !rd->in_terminal;
}

static void
create_dialog (RunDialog * rd)
{
  GtkWidget *checkbox;
  GtkWidget *vbox;
  GtkWidget *combo;

  rd->dialog = gtk_dialog_new_with_buttons (_("Run program"),
					    NULL, 0,
					    GTK_STOCK_OK,
					    GTK_RESPONSE_OK,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_CANCEL, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (rd->dialog), GTK_RESPONSE_OK);
  g_signal_connect (G_OBJECT (rd->dialog), "response",
		    G_CALLBACK (dialog_response_cb), rd);
  g_signal_connect (G_OBJECT (rd->dialog), "close",
		    G_CALLBACK (dialog_close_cb), rd);

  gtk_window_set_default_size (GTK_WINDOW (rd->dialog), 400, 10);
  gtk_window_set_position (GTK_WINDOW (rd->dialog), GTK_WIN_POS_CENTER);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (rd->dialog)->vbox),
		      vbox, FALSE, TRUE, 0);

  combo = gtk_combo_new ();
  rd->combo_entry = GTK_COMBO (combo)->entry;
  gtk_combo_set_popdown_strings (GTK_COMBO (combo), rd->history);
  gtk_box_pack_start (GTK_BOX (vbox), combo, TRUE, TRUE, 2);

  gtk_combo_disable_activate (GTK_COMBO (combo));
  g_object_set (G_OBJECT (rd->combo_entry), "activates-default", TRUE, NULL);

  checkbox = gtk_check_button_new_with_mnemonic (_("Run in _terminal"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox),
				rd->in_terminal);
  g_signal_connect (G_OBJECT (checkbox), "toggled",
		    G_CALLBACK (checkbox_toggled_cb), rd);
  gtk_box_pack_start (GTK_BOX (vbox), checkbox, TRUE, TRUE, 2);

  gtk_editable_select_region (GTK_EDITABLE (rd->combo_entry), 0, -1);
  gtk_widget_grab_focus (rd->combo_entry);
  gtk_widget_show_all (rd->dialog);
}

int
main (int argc, char **argv)
{
  RunDialog *rd = run_dialog_new ();

  gtk_init (&argc, &argv);

  rd->history = get_history ();

  create_dialog (rd);

  gtk_main ();

  return 0;
}
