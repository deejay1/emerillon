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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "add-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#define ADD_DIALOG_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        TYPE_ADD_DIALOG, \
        AddDialogPrivate))

G_DEFINE_TYPE (AddDialog, add_dialog, GTK_TYPE_DIALOG);

struct _AddDialogPrivate
{
  GtkWidget *entry;
};

static void     build_ui        (AddDialog *self);

static void
add_dialog_init (AddDialog *self)
{
  self->priv = ADD_DIALOG_GET_PRIVATE (self);
}

static void
add_dialog_dispose (GObject *object)
{
  //AddDialog *self = ADD_DIALOG (object);

  G_OBJECT_CLASS (add_dialog_parent_class)->dispose (object);
}

static void
add_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (add_dialog_parent_class)->finalize (object);
}

static GObject *
add_dialog_constructor (GType type,
                        guint n_construct_properties,
                        GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (add_dialog_parent_class)->constructor (
      type, n_construct_properties, construct_params);

  build_ui (ADD_DIALOG (object));

  return object;
}


static void
add_dialog_class_init (AddDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = add_dialog_dispose;
  object_class->finalize = add_dialog_finalize;
  object_class->constructor = add_dialog_constructor;

  g_type_class_add_private (object_class, sizeof (AddDialogPrivate));
}

GtkWidget *
add_dialog_new (void)
{
  return g_object_new (TYPE_ADD_DIALOG, NULL);
}

static void
build_ui (AddDialog *self)
{
  GtkWidget *area, *hbox, *label;
  GtkDialog *dialog = GTK_DIALOG (self);

  gtk_window_set_title (GTK_WINDOW (self), _("New placemark"));

  gtk_dialog_set_has_separator (dialog, FALSE);
  gtk_dialog_add_button (dialog, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (dialog, GTK_STOCK_ADD, GTK_RESPONSE_OK);
  gtk_dialog_set_default_response (dialog, GTK_RESPONSE_OK);

  area = gtk_dialog_get_content_area (dialog);
  hbox = gtk_hbox_new (FALSE, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);

  label = gtk_label_new (_("Name:"));
  gtk_container_add (GTK_CONTAINER (hbox), label);

  self->priv->entry = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (hbox), self->priv->entry);
  gtk_widget_grab_focus (self->priv->entry);
  gtk_widget_show_all (hbox);

  gtk_container_add (GTK_CONTAINER (area), hbox);
}

const gchar *
add_dialog_get_name (AddDialog *dialog)
{
  return gtk_entry_get_text (GTK_ENTRY (dialog->priv->entry));
}
