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

#include "placemarks.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "emerillon/emerillon.h"

G_DEFINE_TYPE (PlacemarksPlugin, placemarks_plugin, ETHOS_TYPE_PLUGIN)

enum {
  COL_ID,
  COL_NAME,
  COL_LAT,
  COL_LON,
  COL_ZOOM,
  COL_UI_ID,
  COL_COUNT
};

struct _PlacemarksPluginPrivate
{
  EmerillonWindow *window;
  ChamplainView *map_view;

  GtkActionGroup *action_group;
  guint ui_id;

  GtkTreeModel *model;
  GtkWidget *menu;
};

static void
go_cb (GtkAction *action,
       PlacemarksPlugin *plugin)
{
  GtkTreeIter iter;
  PlacemarksPluginPrivate *priv;
  const gchar *id;
  gboolean found = FALSE;
  GtkTreeIter found_iter;
  GValue value = {0};
  gfloat lat, lon;
  gint zoom;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  id = gtk_action_get_name (action);

  gtk_tree_model_get_iter_first (priv->model, &iter);

  do {
    const gchar *vid;

    gtk_tree_model_get_value (priv->model, &iter, COL_ID, &value);
    vid = g_value_get_string (&value);
    if (strcmp (id, vid) == 0)
      {
        found = TRUE;
        found_iter = iter;
      }

    g_value_unset (&value);
  } while (gtk_tree_model_iter_next (priv->model, &iter) && !found);

  if (!found)
    return;

  gtk_tree_model_get_value (priv->model, &found_iter, COL_LAT, &value);
  lat = g_value_get_float (&value);
  g_value_unset (&value);

  gtk_tree_model_get_value (priv->model, &found_iter, COL_LON, &value);
  lon = g_value_get_float (&value);
  g_value_unset (&value);

  gtk_tree_model_get_value (priv->model, &found_iter, COL_ZOOM, &value);
  zoom = g_value_get_int (&value);
  g_value_unset (&value);

  champlain_view_set_zoom_level (priv->map_view, zoom);
  champlain_view_center_on (priv->map_view, lat, lon);
}

static guint
append_menu_item (PlacemarksPlugin *plugin,
                  const gchar *id,
                  const gchar *name)
{
  PlacemarksPluginPrivate *priv;
  GtkUIManager *manager;
  GError *error = NULL;
  gchar * item_ui_definition;
  GtkActionEntry actions[] = {
    { id,
      NULL,
      name,
      NULL,
      N_("Go to this placemark"),
      G_CALLBACK (go_cb) }
  };
  guint ui_id;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  manager = emerillon_window_get_ui_manager (priv->window);

  item_ui_definition = g_strconcat (
    "<ui>"
      "<menubar name=\"MainMenu\">"
        "<placeholder name=\"PluginsMenu\">"
          "<menu name=\"Placemarks\" action=\"PlacemarksMenu\">"
            "<placeholder name=\"PlacemarksGoItems\" action=\"PlacemarksGoItems\">"
              "<menuitem action=\"", id, "\"/>"
            "</placeholder>"
          "</menu>"
        "</placeholder>"
      "</menubar>"
    "</ui>", NULL);


  gtk_action_group_add_actions (priv->action_group,
                                actions,
                                G_N_ELEMENTS (actions),
                                plugin);

  ui_id = gtk_ui_manager_add_ui_from_string (manager,
                                                   item_ui_definition,
                                                   -1, &error);
  if (ui_id == 0)
    {
      g_error ("Error adding UI %s", error->message);
      g_error_free (error);
    }

  g_free (item_ui_definition);
  return ui_id;
}

static void
load_placemarks (PlacemarksPlugin *plugin)
{
  gchar *filename = NULL;
  GKeyFile *file;
  GError *error = NULL;
  gchar **groups = NULL;
  guint i;
  gsize group_count;
  PlacemarksPluginPrivate *priv;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  filename = g_build_filename (g_get_user_data_dir (),
                               "emerillon",
                               "placemarks.ini",
                               NULL);

  file = g_key_file_new ();
  if (!g_key_file_load_from_file (file,
                                 filename,
                                 G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
                                 &error))
    {
      g_error ("Error loading %s: %s", filename, error->message);
      g_error_free (error);
      g_key_file_free (file);
      return;
    }
  g_free (filename);

  groups = g_key_file_get_groups (file, &group_count);

  for (i = 0; i < group_count; i++)
    {
      GtkTreeIter iter;
      gchar *name;
      gfloat lat, lon;
      gint zoom;
      guint ui_id;

      name = g_key_file_get_string (file, groups[i], "name", &error);
      if (error)
        {
          g_error ("Error loading name key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          name = g_strdup ("A placemark");
        }

      lat = g_key_file_get_double (file, groups[i], "latitude", &error);
      if (error)
        {
          g_error ("Error loading latitude key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          lat = 0.0;
        }

      lon = g_key_file_get_double (file, groups[i], "longitude", &error);
      if (error)
        {
          g_error ("Error loading longitude key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          lon = 0.0;
        }

      zoom = g_key_file_get_integer (file, groups[i], "zoom", &error);
      if (error)
        {
          g_error ("Error loading longitude key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          zoom = 0;
        }

      ui_id = append_menu_item (plugin, groups[i], name);

      gtk_list_store_append (GTK_LIST_STORE (priv->model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (priv->model), &iter,
                          COL_ID, groups[i],
                          COL_NAME, name,
                          COL_LAT, lat,
                          COL_LON, lon,
                          COL_ZOOM, zoom,
                          COL_UI_ID, ui_id,
                          -1);

      g_free (name);
    }

  g_strfreev (groups);
}

static void
add_cb (GtkAction *action,
        PlacemarksPlugin *plugin)
{
  PlacemarksPluginPrivate *priv;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;

}

static void
manage_cb (GtkAction *action,
           PlacemarksPlugin *plugin)
{
  PlacemarksPluginPrivate *priv;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;

}

static const gchar * const ui_definition =
    "<ui>"
      "<menubar name=\"MainMenu\">"
        "<placeholder name=\"PluginsMenu\">"
          "<menu name=\"Placemarks\" action=\"PlacemarksMenu\">"
            "<menuitem name=\"PlacemarksAddMenu\" action=\"PlacemarksAdd\"/>"
            "<menuitem name=\"PlacemarksManageMenu\" action=\"PlacemarksManage\"/>"
            "<separator/>"
            "<placeholder name=\"PlacemarksGoItems\" action=\"PlacemarksGoItems\"/>"
          "</menu>"
        "</placeholder>"
      "</menubar>"
    "</ui>";

static const GtkActionEntry action_entries[] =
{
  { "PlacemarksMenu",   NULL, N_("_Placemarks") },

  { "PlacemarksAdd",
    GTK_STOCK_ADD,
    N_("Placemark this location"),
    NULL,
    N_("Add current location to your placemarks"),
    G_CALLBACK (add_cb) },
  { "PlacemarksManage",
    GTK_STOCK_EDIT,
    N_("Organize placemarks..."),
    NULL,
    N_("Edit and delete existing placemarks"),
    G_CALLBACK (manage_cb) }
};

static void
activated (EthosPlugin *plugin)
{
  PlacemarksPluginPrivate *priv;
  GtkUIManager *manager;
  GtkListStore *store;
  GtkWidget *menu_item;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  priv->window = EMERILLON_WINDOW (emerillon_window_dup_default ());
  priv->map_view = emerillon_window_get_map_view (priv->window);

  manager = emerillon_window_get_ui_manager (priv->window);

  priv->action_group = gtk_action_group_new ("PlacemarksActions");
  gtk_action_group_set_translation_domain (priv->action_group,
                                           GETTEXT_PACKAGE);
  gtk_action_group_add_actions (priv->action_group,
                                action_entries,
                                G_N_ELEMENTS (action_entries),
                                plugin);
  gtk_ui_manager_insert_action_group (manager,
                                      priv->action_group,
                                      -1);

  priv->ui_id = gtk_ui_manager_add_ui_from_string (manager,
                                                   ui_definition,
                                                   -1, NULL);
  g_warn_if_fail (priv->ui_id != 0);

  menu_item = gtk_ui_manager_get_widget (manager,
                                         "/MainMenu/PluginsMenu/Placemarks");
  priv->menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu_item));

  store = gtk_list_store_new (COL_COUNT,
                              G_TYPE_STRING,       /* ID */
                              G_TYPE_UINT,         /* UI ID */
                              G_TYPE_STRING,       /* Name */
                              G_TYPE_FLOAT,        /* Latitude */
                              G_TYPE_FLOAT,        /* Longitude */
                              G_TYPE_INT);         /* Zoom level */
  priv->model = GTK_TREE_MODEL (store);

  load_placemarks (PLACEMARKS_PLUGIN (plugin));
}

static void
deactivated (EthosPlugin *plugin)
{
  GtkUIManager *manager;
  PlacemarksPluginPrivate *priv;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  manager = emerillon_window_get_ui_manager (priv->window);

  gtk_ui_manager_remove_ui (manager,
                            priv->ui_id);

  gtk_ui_manager_remove_action_group (manager,
                                      priv->action_group);
}

static void
placemarks_plugin_class_init (PlacemarksPluginClass *klass)
{
  EthosPluginClass *plugin_class;

  g_type_class_add_private (klass, sizeof (PlacemarksPluginPrivate));

  plugin_class = ETHOS_PLUGIN_CLASS (klass);
  plugin_class->activated = activated;
  plugin_class->deactivated = deactivated;
}

static void
placemarks_plugin_init (PlacemarksPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              PLACEMARKS_TYPE_PLUGIN,
                                              PlacemarksPluginPrivate);
}

EthosPlugin*
placemarks_plugin_new (void)
{
  return g_object_new (PLACEMARKS_TYPE_PLUGIN, NULL);
}

G_MODULE_EXPORT EthosPlugin*
ethos_plugin_register (void)
{
  return placemarks_plugin_new ();
}
