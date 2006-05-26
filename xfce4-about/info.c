/*  xfce4
 *  Copyright (C) 1999 Olivier Fourdan (fourdan@xfce.org)
 *                2002 Xavier MAILLARD (zedek@fxgsproject.org)
 *                2003 Jasper Huijsmans (huysmans@users.sourceforge.net)
 *                2003,2006 Benedikt Meurer (benny@xfce.org)
 *                2005 Jean-François Wauthy (pollux@xfce.org)
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

#ifdef HAVE_LC_MESSAGES
#include <locale.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_LIBGTKHTML
#include <libgtkhtml/gtkhtml.h>
#endif

#include <libxfce4util/libxfce4util.h>
#include <libxfcegui4/libxfcegui4.h>

#define SEARCHPATH	(DATADIR G_DIR_SEPARATOR_S "%F.%L:"	\
                         DATADIR G_DIR_SEPARATOR_S "%F.%l:"	\
                         DATADIR G_DIR_SEPARATOR_S "%F")

#define XFCE_COPYRIGHT	"COPYING"
#define XFCE_AUTHORS	"AUTHORS"
#define XFCE_INFO	"INFO"
#define XFCE_BSDL	"BSD"
#define XFCE_GPL	"GPL"
#define XFCE_LGPL	"LGPL"

#define BORDER 6

static GtkWidget *info;

static void
info_help_cb (GtkWidget * w, gpointer data)
{
  xfce_exec ("xfhelp4", FALSE, FALSE, NULL);
}

static gchar *
replace_version (gchar * buffer)
{
  gchar *dst;
  gchar *bp;

  bp = strstr (buffer, "@VERSION@");
  if (bp != NULL) {
    const gchar *version = xfce_version_string ();
    gchar *complete_version;
    gsize n = bp - buffer;

#ifdef RELEASE_LABEL
    if (strlen (RELEASE_LABEL))
      complete_version = g_strdup_printf ("%s (%s)", version, RELEASE_LABEL);
    else
#endif
      complete_version = g_strdup (version);

    dst = g_new (gchar, strlen (buffer) + strlen (complete_version) + 1);
    memcpy (dst, buffer, n);
    memcpy (dst + n, complete_version, strlen (complete_version));
    strcpy (dst + n + strlen (complete_version), buffer + n + strlen ("@VERSION@"));
    g_free (complete_version);
    g_free (buffer);

    return dst;
  }

  return buffer;
}

/****************/
/* Authors page */
/****************/
static void
create_tags (GtkTextBuffer * buffer)
{
  gtk_text_buffer_create_tag (buffer, "title",
                              "size", 11 * PANGO_SCALE, "pixels_above_lines", 16, "pixels_below_lines", 16, NULL);

  gtk_text_buffer_create_tag (buffer, "email",
                              "size", 9 * PANGO_SCALE,
                              "family", "monospace", "foreground", "blue", "underline", PANGO_UNDERLINE_SINGLE, NULL);

  gtk_text_buffer_create_tag (buffer, "author", "left_margin", 25, NULL);
}

static gboolean
add_author (FILE * file_authors, GtkTextBuffer *textbuffer, GtkTextIter *iter, const gchar *category, const gchar * title)
{
  gchar buf[80];

  gtk_text_buffer_insert_with_tags_by_name (textbuffer, iter, title, -1, "title", NULL);
  gtk_text_buffer_insert (textbuffer, iter, "\n", -1);
  while (fgets (buf, sizeof (buf), file_authors)) {
    g_strstrip (buf);
    if (strcmp (buf, category) == 0)
      break;
  }
  if (feof (file_authors) != 0)
    return FALSE;
  while (fgets (buf, sizeof (buf), file_authors)) {
    gchar **author = NULL;

    g_strstrip (buf);
    if (strlen (buf) == 0)
      break;

    author = g_strsplit (buf, ";", 0);

    gtk_text_buffer_insert_with_tags_by_name (textbuffer, iter, author[0], -1, "author", NULL);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, iter, " <", -1, "author", NULL);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, iter, author[1], -1, "email", NULL);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, iter, ">", -1, "author", NULL);
    gtk_text_buffer_insert (textbuffer, iter, "\n", -1);

    g_strfreev (author);
  }

  return TRUE;
}

static void
add_credits_page (GtkNotebook * notebook, const gchar * name, gboolean hscrolling)
{
  gchar *authors_filename = NULL;
  FILE *file_authors;

  GtkWidget *label;
  GtkWidget *view;
  GtkWidget *sw;
  GtkWidget *textview;
  GtkTextBuffer *textbuffer;
  GtkTextIter iter;

  gchar buf[80];

  authors_filename = g_strconcat (DATADIR, G_DIR_SEPARATOR_S, XFCE_AUTHORS, NULL);
  file_authors = fopen (authors_filename, "r");

  if (!file_authors) {
    xfce_err ("%s%s", _("Unable to load "), authors_filename);
    g_free (authors_filename);
    return;
  }

  label = gtk_label_new (name);
  gtk_widget_show (label);

  view = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (view), BORDER);
  gtk_frame_set_shadow_type (GTK_FRAME (view), GTK_SHADOW_IN);
  gtk_widget_show (view);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  hscrolling ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (sw);
  gtk_container_add (GTK_CONTAINER (view), sw);

  /* add the text */
  /* ============ */
  textview = gtk_text_view_new ();
  textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));

  create_tags (textbuffer);

  gtk_text_buffer_get_iter_at_offset (textbuffer, &iter, 0);

  /* Project lead */
  if (!add_author (file_authors, textbuffer, &iter, "[Lead]", _("Project Lead")))
    g_error ("%s file is corrupted !", authors_filename);

  /* Core developers */
  if (!add_author (file_authors, textbuffer, &iter, "[Core]", _("Core developers")))
    g_error ("%s file is corrupted !", authors_filename);

  /* Contributors */
  if (!add_author (file_authors, textbuffer, &iter, "[Contributors]", _("Contributors")))
    g_error ("%s file is corrupted !", authors_filename);

  /* Hosting */
  gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter,
                                            _("Web Hosting and Mailing Lists provided by"), -1, "title", NULL);
  gtk_text_buffer_insert (textbuffer, &iter, "\n", -1);
  while (fgets (buf, sizeof (buf), file_authors)) {
    g_strstrip (buf);
    if (strcmp (buf, "[Hosting]") == 0)
      break;
  }
  if (feof (file_authors) != 0)
    g_error ("%s file is corrupted !", authors_filename);
  while (fgets (buf, sizeof (buf), file_authors)) {
    gchar **author = NULL;

    g_strstrip (buf);
    if (strlen (buf) == 0)
      break;

    author = g_strsplit (buf, ";", 0);

    gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, author[0], -1, "author", NULL);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, " <", -1, "author", NULL);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, author[1], -1, "email", NULL);
    if (author[2]) {
      gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, " - ", -1, "author", NULL);
      gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, author[2], -1, "email", NULL);
    }
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, ">", -1, "author", NULL);
    gtk_text_buffer_insert (textbuffer, &iter, "\n", -1);

    g_strfreev (author);
  }

  /* Server admins */
  if (!add_author (file_authors, textbuffer, &iter, "[Server administration]", _("Server maintained by")))
    g_error ("%s file is corrupted !", authors_filename);

  /* Translations supervision */
  if (!add_author (file_authors, textbuffer, &iter, "[Translations supervision]", _("Translations supervision")))
    g_error ("%s file is corrupted !", authors_filename);

  /* Translators */
  gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, _("Translators"), -1, "title", NULL);
  gtk_text_buffer_insert (textbuffer, &iter, "\n", -1);
  while (fgets (buf, sizeof (buf), file_authors)) {
    g_strstrip (buf);
    if (strcmp (buf, "[Translators]") == 0)
      break;
  }
  if (feof (file_authors) != 0)
    g_error ("%s file is corrupted !", authors_filename);
  while (fgets (buf, sizeof (buf), file_authors)) {
    gchar **author = NULL;
    gchar *text = NULL;

    g_strstrip (buf);
    if (strlen (buf) == 0)
      break;

    author = g_strsplit (buf, ";", 0);

    text = g_strdup_printf ("%s: %s <", author[0], author[1]);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, text, -1, "author", NULL);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, author[2], -1, "email", NULL);
    gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, ">", -1, "author", NULL);
    gtk_text_buffer_insert (textbuffer, &iter, "\n", -1);

    g_free (text);
    g_strfreev (author);
  }

  /* Thanks */
  gtk_text_buffer_insert (textbuffer, &iter, "\n", -1);
  gtk_text_buffer_insert (textbuffer, &iter, _("If you know of anyone missing from this list, please let us know on <"),
                          -1);
  gtk_text_buffer_insert_with_tags_by_name (textbuffer, &iter, "xfce4-dev@xfce.org", -1, "email", NULL);
  gtk_text_buffer_insert (textbuffer, &iter, ">.\n\n", -1);
  gtk_text_buffer_insert (textbuffer, &iter, _("Thanks to all who helped making this software available."), -1);
  gtk_text_buffer_insert (textbuffer, &iter, "\n", -1);

  fclose (file_authors);
  g_free (authors_filename);

  /* Show all */
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (textview), BORDER);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (textview), BORDER);

  gtk_widget_show (textview);
  gtk_container_add (GTK_CONTAINER (sw), textview);

  gtk_notebook_append_page (notebook, view, label);
}

/***************/
/* Other pages */
/***************/
static void
add_page (GtkNotebook * notebook, const gchar * name, const gchar * filename, gboolean hscrolling)
{
  /*gchar buffer[PATH_MAX + 1]; */
  GtkTextBuffer *textbuffer;
  GtkWidget *textview;
  GtkWidget *label;
  GtkWidget *view;
  GtkWidget *sw;
  GError *err;
  gchar *path;
  gchar *hfilename;
  gchar *buf;
  gsize n;

  err = NULL;

  label = gtk_label_new (name);
  gtk_widget_show (label);

  hfilename = g_strconcat (DATADIR, G_DIR_SEPARATOR_S, filename, NULL);
  path = xfce_get_file_localized (hfilename);
  g_free (hfilename);
  /* xfce_get_path_localized(buffer, sizeof(buffer),
     SEARCHPATH, filename, G_FILE_TEST_IS_REGULAR); */

  if (g_file_get_contents (path, &buf, &n, &err)) {
    buf = replace_version (buf);
    g_free (path);
  }

  if (err != NULL) {
    xfce_err ("%s", err->message);
    g_error_free (err);
  }
  else {
    view = gtk_frame_new (NULL);
    gtk_container_set_border_width (GTK_CONTAINER (view), BORDER);
    gtk_frame_set_shadow_type (GTK_FRAME (view), GTK_SHADOW_IN);
    gtk_widget_show (view);

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    hscrolling ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (sw);
    gtk_container_add (GTK_CONTAINER (view), sw);

    textbuffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (textbuffer, buf, strlen (buf));

    textview = gtk_text_view_new_with_buffer (textbuffer);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (textview), BORDER);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (textview), BORDER);

    gtk_widget_show (textview);
    gtk_container_add (GTK_CONTAINER (sw), textview);

    gtk_notebook_append_page (notebook, view, label);

    g_free (buf);
  }
}

int
main (int argc, char **argv)
{
  GtkWidget *header;
  GtkWidget *vbox, *vbox2;
  GtkWidget *notebook;
  GtkWidget *buttonbox;
  GtkWidget *info_ok_button;
  GtkWidget *info_help_button;
  GdkPixbuf *logo_pb;
  char *text;

  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  gtk_init (&argc, &argv);

  info = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (info), _("About Xfce 4"));
  gtk_dialog_set_has_separator (GTK_DIALOG (info), FALSE);
  gtk_window_stick (GTK_WINDOW (info));

  logo_pb = xfce_themed_icon_load ("xfce4-logo", 48);
  gtk_window_set_icon (GTK_WINDOW (info), logo_pb);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (info)->vbox), vbox2, TRUE, TRUE, 0);

  /* header with logo */
  text =
    g_strdup_printf
    ("%s\n<span size=\"smaller\" style=\"italic\">%s</span>",
     _("Xfce Desktop Environment"), _("Copyright 2002-2006 by Olivier Fourdan"));
  header = xfce_create_header (logo_pb, text);
  gtk_widget_show (header);
  gtk_box_pack_start (GTK_BOX (vbox2), header, FALSE, FALSE, 0);
  g_free (text);
  g_object_unref (logo_pb);

  vbox = gtk_vbox_new (FALSE, BORDER);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), BORDER);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (vbox2), vbox, TRUE, TRUE, 0);

  /* the notebook */
  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_widget_set_size_request (notebook, -1, 270);
  gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

  /* add pages */
#ifdef VENDOR_INFO
  add_page (GTK_NOTEBOOK (notebook), VENDOR_INFO, VENDOR_INFO, FALSE);
#endif
  add_page (GTK_NOTEBOOK (notebook), _("Info"), XFCE_INFO, FALSE);
  //  add_page (GTK_NOTEBOOK (notebook), _("Credits"), XFCE_AUTHORS, FALSE);
  add_credits_page (GTK_NOTEBOOK (notebook), _("Credits"), FALSE);
  add_page (GTK_NOTEBOOK (notebook), _("Copyright"), XFCE_COPYRIGHT, TRUE);
  add_page (GTK_NOTEBOOK (notebook), _("BSDL"), XFCE_BSDL, TRUE);
  add_page (GTK_NOTEBOOK (notebook), _("LGPL"), XFCE_LGPL, TRUE);
  add_page (GTK_NOTEBOOK (notebook), _("GPL"), XFCE_GPL, TRUE);

  /* buttons */
  buttonbox = GTK_DIALOG (info)->action_area;

  info_help_button = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_widget_show (info_help_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), info_help_button, FALSE, FALSE, 0);

  info_ok_button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_widget_show (info_ok_button);
  gtk_box_pack_start (GTK_BOX (buttonbox), info_ok_button, FALSE, FALSE, 0);

  gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (buttonbox), info_help_button, TRUE);

  g_signal_connect (info, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (info, "destroy-event", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (info_ok_button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (info_help_button, "clicked", G_CALLBACK (info_help_cb), NULL);

  xfce_gtk_window_center_on_monitor_with_pointer (GTK_WINDOW (info));
  gtk_widget_show (info);

  gtk_main ();

  return (EXIT_SUCCESS);
}
