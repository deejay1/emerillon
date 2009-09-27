/*
 * Copyright (C) 2009 Novopia Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ___MANAGE_DIALOG_H__
#define ___MANAGE_DIALOG_H__

#include <gtk/gtk.h>
#include <champlain/champlain.h>

G_BEGIN_DECLS

typedef struct _ManageDialog ManageDialog;
typedef struct _ManageDialogClass ManageDialogClass;
typedef struct _ManageDialogPrivate ManageDialogPrivate;

#define TYPE_MANAGE_DIALOG            (manage_dialog_get_type ())
#define MANAGE_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MANAGE_DIALOG, ManageDialog))
#define MANAGE_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  TYPE_MANAGE_DIALOG, ManageDialogClass))
#define IS_MANAGE_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MANAGE_DIALOG))
#define IS_MANAGE_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  TYPE_MANAGE_DIALOG))
#define MANAGE_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  TYPE_MANAGE_DIALOG, ManageDialogClass))

#define MANAGE_DIALOG_ERROR           (manage_dialog_error_quark ())

struct _ManageDialog
{
  GtkDialog parent;
  ManageDialogPrivate *priv;
};

struct _ManageDialogClass
{
  GtkDialogClass parent_class;
};

GType         manage_dialog_get_type        (void) G_GNUC_CONST;

GtkWidget *   manage_dialog_new             (GtkTreeModel *model);

void          manage_dialog_set_model       (ManageDialog *dialog,
                                             GtkTreeModel *model);

G_END_DECLS

#endif
