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

#ifndef _XFBDMGR2_H
#define _XFBDMGR2_H

#define _(x) x

#ifndef MAXSTRLEN
#define MAXSTRLEN 1024
#endif

#define LIST_TEXT "# xfce backdrop list"

typedef struct
{
  char *filename;
  char *last_path;
  gboolean saved;
  GtkWidget *window;
  GtkWidget *file_label;
  GtkWidget *save_button;
  GtkListStore *list;
  int num_items;
  GtkWidget *treeview;
}
BackdropList;

char *get_filename_dialog (const char *title, const char *path);

gboolean save_list_as (BackdropList * bl);

gboolean save_list (BackdropList * bl);

gboolean load_list (BackdropList * bl);


#endif
