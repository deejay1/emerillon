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

#ifndef ___ADD_DIALOG_H__
#define ___ADD_DIALOG_H__

#include <gtk/gtk.h>
#include <champlain/champlain.h>

G_BEGIN_DECLS

typedef struct _AddDialog AddDialog;
typedef struct _AddDialogClass AddDialogClass;
typedef struct _AddDialogPrivate AddDialogPrivate;

#define TYPE_ADD_DIALOG            (add_dialog_get_type ())
#define ADD_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_ADD_DIALOG, AddDialog))
#define ADD_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  TYPE_ADD_DIALOG, AddDialogClass))
#define IS_ADD_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_ADD_DIALOG))
#define IS_ADD_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  TYPE_ADD_DIALOG))
#define ADD_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  TYPE_ADD_DIALOG, AddDialogClass))

#define ADD_DIALOG_ERROR           (add_dialog_error_quark ())

struct _AddDialog
{
  GtkDialog parent;
  AddDialogPrivate *priv;
};

struct _AddDialogClass
{
  GtkDialogClass parent_class;
};

GType         add_dialog_get_type        (void) G_GNUC_CONST;

GtkWidget *   add_dialog_new             (void);

const gchar * add_dialog_get_name        (AddDialog *dialog);

G_END_DECLS

#endif
