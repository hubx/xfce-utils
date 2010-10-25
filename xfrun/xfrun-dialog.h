/*
 * xfrun - a simple quick run dialog with saved history and completion
 *
 * Copyright (c) 2006 Brian J. Tarricone <bjt23@cornell.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __XFRUN_DIALOG_H__
#define __XFRUN_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XFRUN_TYPE_DIALOG       (xfrun_dialog_get_type())
#define XFRUN_DIALOG(object)    (G_TYPE_CHECK_INSTANCE_CAST((object), XFRUN_TYPE_DIALOG, XfrunDialog))
#define XFRUN_IS_DIALOG(object) (G_TYPE_CHECK_INSTANCE_TYPE((object), XFRUN_TYPE_DIALOG))

typedef struct _XfrunDialog        XfrunDialog;
typedef struct _XfrunDialogClass   XfrunDialogClass;
typedef struct _XfrunDialogPrivate XfrunDialogPrivate;

struct _XfrunDialog
{
    GtkWindow parent;

    /*< private >*/
    XfrunDialogPrivate *priv;
};

struct _XfrunDialogClass
{
    GtkWindowClass parent_class;

    /*< signals >*/
    void (*closed)(XfrunDialog *dialog);
};

GType xfrun_dialog_get_type                        (void) G_GNUC_CONST;

GtkWidget *xfrun_dialog_new                        (void);

G_CONST_RETURN gchar *xfrun_dialog_get_run_argument(XfrunDialog *dialog);

void xfrun_dialog_set_destroy_on_close             (XfrunDialog *dialog,
                                                    gboolean destroy_on_close);
gboolean xfrun_dialog_get_destroy_on_close         (XfrunDialog *dialog);

void xfrun_dialog_set_working_directory            (XfrunDialog *dialog,
                                                    const gchar *working_directory);

void xfrun_dialog_select_text                      (XfrunDialog *dialog);

G_CONST_RETURN gchar *xfrun_dialog_get_working_directory
                                                   (XfrunDialog *dialog);

G_END_DECLS

#endif  /* __XFRUN_DIALOG_H__ */
