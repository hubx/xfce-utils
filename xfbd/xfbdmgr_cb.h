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

#ifndef _XFBDMGR2_CB_H
#define _XFBDMGR2_CB_H

void quit_cb (GtkWidget * w, BackdropList * bl);

void save_cb (GtkWidget * w, BackdropList * bl);

void save_as_cb (GtkWidget * w, BackdropList * bl);

gboolean button_press_cb (GtkTreeView * treeview, GdkEventButton * button,
                          BackdropList * bl);
                          
gboolean key_press_cb (GtkTreeView * treeview, GdkEventKey * key, 
                       BackdropList * bl);

void changed_cb (GtkTreeView * treeview, GdkDragContext * drag_context,
                 BackdropList * bl);

void tb_new_cb (GtkButton * b, BackdropList * bl);

void tb_open_cb (GtkButton * b, BackdropList * bl);

void tb_add_cb (GtkButton * b, BackdropList * bl);

void tb_remove_cb (GtkButton * b, BackdropList * bl);

void tb_help_cb (GtkButton * b, BackdropList * bl);

void on_drag_data_received (GtkWidget * w, GdkDragContext * context,
                            int x, int y, GtkSelectionData * data,
                            guint info, guint time, BackdropList * bl);
                       
#endif
