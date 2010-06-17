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

#include "manage-dialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <emerillon/emerillon.h>

#include "placemarks-model.h"
#include "../../cut-paste/empathy-cell-renderer-activatable.h"

#define MANAGE_DIALOG_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        TYPE_MANAGE_DIALOG, \
        ManageDialogPrivate))

G_DEFINE_TYPE (ManageDialog, manage_dialog, GTK_TYPE_DIALOG);

enum
{
  PROP_0,
  PROP_MODEL,
};

struct _ManageDialogPrivate
{
  GtkTreeModel *model;
};

static void     build_ui        (ManageDialog *self);

static void
manage_dialog_init (ManageDialog *self)
{
  self->priv = MANAGE_DIALOG_GET_PRIVATE (self);
  self->priv->model = NULL;
}

static void
manage_dialog_dispose (GObject *object)
{
  ManageDialog *self = MANAGE_DIALOG (object);

  if (self->priv->model != NULL)
    {
      g_object_unref (self->priv->model);
      self->priv->model = NULL;
    }

  G_OBJECT_CLASS (manage_dialog_parent_class)->dispose (object);
}

static void
manage_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (manage_dialog_parent_class)->finalize (object);
}

static GObject *
manage_dialog_constructor (GType type,
                        guint n_construct_properties,
                        GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (manage_dialog_parent_class)->constructor (
      type, n_construct_properties, construct_params);

  build_ui (MANAGE_DIALOG (object));

  return object;
}

static void
manage_dialog_set_property (GObject *object,
                            guint property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
  ManageDialog *self = MANAGE_DIALOG (object);
  switch (property_id)
    {
      case PROP_MODEL:
        manage_dialog_set_model (self, g_value_get_object (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
manage_dialog_class_init (ManageDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = manage_dialog_dispose;
  object_class->finalize = manage_dialog_finalize;
  object_class->constructor = manage_dialog_constructor;
  object_class->set_property = manage_dialog_set_property;

  g_object_class_install_property (object_class,
      PROP_MODEL,
      g_param_spec_object ("model",
          "Model",
          "The GtkTreeModel of the GtkTreeView",
          GTK_TYPE_TREE_MODEL,
          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (object_class, sizeof (ManageDialogPrivate));
}

GtkWidget *
manage_dialog_new (GtkTreeModel *model)
{
  return g_object_new (TYPE_MANAGE_DIALOG,
                       "model", model,
                       NULL);
}

static void
delete_activated_cb (EmpathyCellRendererActivatable *cell,
                     const gchar *path_string,
                     ManageDialog *dialog)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *name;
  GtkWidget *msg_dialog;
  gint response;
  guint ui_id;
  ChamplainMarker *marker;

  model = dialog->priv->model;

  if (!gtk_tree_model_get_iter_from_string (model, &iter, path_string))
    return;

  gtk_tree_model_get (model, &iter,
                      COL_NAME, &name,
                      COL_UI_ID, &ui_id,
                      COL_MARKER, &marker,
                      -1);

  msg_dialog = gtk_message_dialog_new (GTK_WINDOW (dialog),
                                       GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION,
                                       GTK_BUTTONS_YES_NO,
                                       _("You are about to remove the %s placemark!\n"
                                         "Are you sure you want to proceed?"),
                                       name);

  response = gtk_dialog_run (GTK_DIALOG (msg_dialog));
  gtk_widget_hide (msg_dialog);

  if (response == GTK_RESPONSE_YES)
    {
      GtkUIManager *manager;
      GtkWidget * window;

      window = emerillon_window_dup_default ();
      manager = emerillon_window_get_ui_manager (EMERILLON_WINDOW (window));

      gtk_ui_manager_remove_ui (manager,
                                ui_id);

      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
      clutter_actor_destroy (CLUTTER_ACTOR(marker));
      g_object_unref (window);
    }

  g_free (name);
}

static void
build_ui (ManageDialog *self)
{
  GtkWidget *area, *treeview, *scrolled;
  GtkCellRenderer *cell, *trash_cell;
  GtkTreeViewColumn *column;
  GtkDialog *dialog = GTK_DIALOG (self);

  gtk_window_set_title (GTK_WINDOW (self), _("Organize Placemarks"));

  gtk_dialog_set_has_separator (dialog, FALSE);
  gtk_dialog_add_button (dialog, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
  gtk_dialog_set_default_response (dialog, GTK_RESPONSE_CLOSE);
  gtk_widget_set_size_request (GTK_WIDGET (dialog), 400, 300);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

  area = gtk_dialog_get_content_area (dialog);

  treeview = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), self->priv->model);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), COL_NAME);
  /* FIXME: This works but the menu don't get reloaded in the correct order
   * gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
   */

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
      GTK_POLICY_AUTOMATIC,
      GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
      GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (scrolled), treeview);
  gtk_box_pack_start (GTK_BOX (area), scrolled, TRUE, TRUE, 0);

  cell = gtk_cell_renderer_text_new ();
  g_object_set (cell,
                "ellipsize", PANGO_ELLIPSIZE_END,
                NULL);

  column = gtk_tree_view_column_new_with_attributes (_("Name"),
                                                     cell,
                                                     "text", COL_NAME,
                                                     NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_NAME);
  gtk_tree_view_column_set_expand (column, TRUE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  /* Delete icon */
  trash_cell = empathy_cell_renderer_activatable_new ();
  gtk_tree_view_column_pack_start (column, trash_cell, FALSE);
  g_object_set (trash_cell,
                "icon-name", GTK_STOCK_DELETE,
                NULL);
  g_signal_connect (trash_cell, "path-activated",
                    G_CALLBACK (delete_activated_cb),
                    dialog);

  column = gtk_tree_view_column_new_with_attributes (_("Latitude"),
                                                     cell,
                                                     "text", COL_LAT_STR,
                                                     NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_LAT);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  column = gtk_tree_view_column_new_with_attributes (_("Longitude"),
                                                     cell,
                                                     "text", COL_LON_STR,
                                                     NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_LON);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  column = gtk_tree_view_column_new_with_attributes (_("Zoom"),
                                                     cell,
                                                     "text", COL_ZOOM_STR,
                                                     NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_ZOOM);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  gtk_widget_show_all (area);
}

void
manage_dialog_set_model (ManageDialog *dialog,
                         GtkTreeModel *model)
{
  g_return_if_fail (IS_MANAGE_DIALOG (dialog));

  if (dialog->priv->model != NULL)
    g_object_unref (dialog->priv->model);

  dialog->priv->model = g_object_ref (model);
}
