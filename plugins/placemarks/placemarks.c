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

#include "add-dialog.h"
#include "manage-dialog.h"
#include "placemarks-model.h"

G_DEFINE_TYPE (PlacemarksPlugin, placemarks_plugin, ETHOS_TYPE_PLUGIN)

struct _PlacemarksPluginPrivate
{
  EmerillonWindow *window;
  ChamplainView *map_view;

  GtkActionGroup *action_group;
  GtkActionGroup *menu_action_group;
  guint ui_id;
  guint placemark_count;

  GtkTreeModel *model;
  GtkWidget *menu;

  guint deleted_cb_id;
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
  gfloat lat, lon;
  gint zoom;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  id = gtk_action_get_name (action);

  gtk_tree_model_get_iter_first (priv->model, &iter);

  do
    {
      gchar *vid;

      gtk_tree_model_get (priv->model, &iter, COL_ID, &vid, -1);
      if (strcmp (id, vid) == 0)
        {
          found = TRUE;
          found_iter = iter;
        }

      g_free (vid);
    }
  while (gtk_tree_model_iter_next (priv->model, &iter) && !found);

  if (!found)
    return;

  gtk_tree_model_get (priv->model, &found_iter,
      COL_LAT, &lat,
      COL_LON, &lon,
      COL_ZOOM, &zoom,
      -1);

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


  gtk_action_group_add_actions (priv->menu_action_group,
                                actions,
                                G_N_ELEMENTS (actions),
                                plugin);

  ui_id = gtk_ui_manager_add_ui_from_string (manager,
                                             item_ui_definition,
                                             -1, &error);
  if (ui_id == 0)
    {
      g_warning ("Error adding UI %s", error->message);
      g_error_free (error);
    }

  g_free (item_ui_definition);
  return ui_id;
}

static gboolean
save_placemarks (PlacemarksPlugin *plugin)
{
  PlacemarksPluginPrivate *priv;
  GKeyFile *file;
  GtkTreeIter iter;
  gchar *data, *filename, *path;
  GError *error = NULL;
  gint i = 0;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  file = g_key_file_new ();

  if (gtk_tree_model_get_iter_first (priv->model, &iter))
    {
      do
        {
          gchar *id;
          gchar *name;
          gfloat lat, lon;
          gint zoom;

          id = g_strdup_printf ("Placemark%d", i++),

          gtk_tree_model_get (priv->model, &iter,
              COL_NAME, &name,
              COL_LAT, &lat,
              COL_LON, &lon,
              COL_ZOOM, &zoom,
              -1);

          g_key_file_set_string (file, id, "name", name);
          g_key_file_set_double (file, id, "latitude", lat);
          g_key_file_set_double (file, id, "longitude", lon);
          g_key_file_set_integer (file, id, "zoom", zoom);

          g_free (id);
          g_free (name);
        }
      while (gtk_tree_model_iter_next (priv->model, &iter));
    }

  data = g_key_file_to_data (file, NULL, NULL);
  filename = g_build_filename (g_get_user_data_dir (),
                               "emerillon",
                               "placemarks.ini",
                               NULL);

  path = g_path_get_dirname (filename);
  if (g_mkdir_with_parents (path, 0700) != 0)
    g_error ("Error creating %s directory", path);
  g_free (path);

  if (!g_file_set_contents (filename, data, -1, &error))
    {
      g_warning ("Error writing %s: %s", filename, error->message);
      g_error_free (error);
    }

  g_key_file_free (file);
  return FALSE;
}

static void
add_menu (PlacemarksPlugin *plugin,
          const gchar *id,
          const gchar *name,
          GtkTreeIter *iter)
{
  gint ui_id;
  PlacemarksPluginPrivate *priv;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  ui_id = append_menu_item (plugin, id, name);

  gtk_list_store_set (GTK_LIST_STORE (priv->model), iter,
                      COL_UI_ID, ui_id,
                      -1);

}

static GtkTreeIter
add_placemark (PlacemarksPlugin *plugin,
               const gchar *id,
               const gchar *name,
               gfloat lat,
               gfloat lon,
               gint zoom)
{
  gchar *lat_str, *lon_str, *zoom_str;
  GtkTreeIter iter;
  PlacemarksPluginPrivate *priv;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;

  lat_str = g_strdup_printf ("%f", lat);
  lon_str = g_strdup_printf ("%f", lon);
  zoom_str = g_strdup_printf ("%d", zoom);

  gtk_list_store_append (GTK_LIST_STORE (priv->model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (priv->model), &iter,
                      COL_ID, id,
                      COL_NAME, name,
                      COL_LAT, lat,
                      COL_LAT_STR, lat_str,
                      COL_LON, lon,
                      COL_LON_STR, lon_str,
                      COL_ZOOM, zoom,
                      COL_ZOOM_STR, zoom_str,
                      -1);

  g_free (lat_str);
  g_free (lon_str);
  g_free (zoom_str);

  priv->placemark_count++;
  return iter;
}

static gboolean
clear_menus (PlacemarksPlugin *plugin)
{
  GtkUIManager *manager;
  PlacemarksPluginPrivate *priv;
  GtkTreeIter iter;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  manager = emerillon_window_get_ui_manager (priv->window);

  if (gtk_tree_model_get_iter_first (priv->model, &iter))
    {
      do
        {
          guint ui_id;

          gtk_tree_model_get (priv->model, &iter, COL_UI_ID, &ui_id, -1);
          gtk_ui_manager_remove_ui (manager, ui_id);
        }
      while (gtk_tree_model_iter_next (priv->model, &iter));
    }

  gtk_ui_manager_remove_action_group (manager,
                                      priv->menu_action_group);

  return FALSE;
}

static gboolean
load_menus (PlacemarksPlugin *plugin)
{
  GtkUIManager *manager;
  PlacemarksPluginPrivate *priv;
  GtkTreeIter iter;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  manager = emerillon_window_get_ui_manager (priv->window);

  priv->menu_action_group = gtk_action_group_new ("PlacemarksGoActions");
  gtk_action_group_set_translation_domain (priv->menu_action_group,
                                           GETTEXT_PACKAGE);
  gtk_ui_manager_insert_action_group (manager,
                                      priv->menu_action_group,
                                      -1);

  if (gtk_tree_model_get_iter_first (priv->model, &iter))
    {
      do
        {
          gchar *name;
          gchar *id;

          gtk_tree_model_get (priv->model, &iter,
                              COL_ID, &id,
                              COL_NAME, &name,
                              -1 );

          add_menu (plugin, id, name, &iter);

          g_free (id);
          g_free (name);
        }
      while (gtk_tree_model_iter_next (priv->model, &iter));
    }
  return FALSE;
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
      g_warning ("Error loading %s: %s", filename, error->message);
      g_error_free (error);
      //g_key_file_free (file);
      return;
    }
  g_free (filename);

  groups = g_key_file_get_groups (file, &group_count);
  priv->placemark_count = group_count;

  for (i = 0; i < group_count; i++)
    {
      gchar *name;
      gfloat lat, lon;
      gint zoom;

      name = g_key_file_get_string (file, groups[i], "name", &error);
      if (error)
        {
          g_warning ("Error loading name key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          name = g_strdup ("A placemark");
        }

      lat = g_key_file_get_double (file, groups[i], "latitude", &error);
      if (error)
        {
          g_warning ("Error loading latitude key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          lat = 0.0;
        }

      lon = g_key_file_get_double (file, groups[i], "longitude", &error);
      if (error)
        {
          g_warning ("Error loading longitude key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          lon = 0.0;
        }

      zoom = g_key_file_get_integer (file, groups[i], "zoom", &error);
      if (error)
        {
          g_warning ("Error loading longitude key of group %s: %s", groups[i], error->message);
          g_error_free (error);
          error = NULL;
          zoom = 0;
        }

      add_placemark (plugin, groups[i], name, lat, lon, zoom);

      g_free (name);
    }

  g_strfreev (groups);
}

static void
add_cb (GtkAction *action,
        PlacemarksPlugin *plugin)
{
  PlacemarksPluginPrivate *priv;
  gdouble lat, lon;
  gint zoom;
  GtkWidget *dialog;
  const gchar *name;
  gchar *id;
  gint response;
  GtkTreeIter iter;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  dialog = add_dialog_new ();
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (priv->window));

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  name = add_dialog_get_name (ADD_DIALOG (dialog));
  gtk_widget_hide (dialog);

  if (response != GTK_RESPONSE_OK)
    return;

  g_object_get (priv->map_view,
                "latitude", &lat,
                "longitude", &lon,
                "zoom-level", &zoom,
                NULL);

  id = g_strdup_printf ("Placemark%d", priv->placemark_count),

  iter = add_placemark (plugin, id, name, lat, lon, zoom);
  add_menu (plugin, id, name, &iter);

  save_placemarks (plugin);

  g_free (id);
}

static void
manage_cb (GtkAction *action,
           PlacemarksPlugin *plugin)
{
  PlacemarksPluginPrivate *priv;
  GtkWidget *dialog;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  dialog = manage_dialog_new (priv->model);

  g_signal_connect_swapped (dialog,
                            "response",
                            G_CALLBACK (gtk_widget_destroy),
                            dialog);

  gtk_widget_show (dialog);
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
row_deleted_cb (GtkTreeModel *tree_model,
                GtkTreePath *path,
                PlacemarksPlugin *plugin)
{
  g_idle_add ((GSourceFunc) save_placemarks, plugin);
  g_idle_add ((GSourceFunc) clear_menus, plugin);
  g_idle_add ((GSourceFunc) load_menus, plugin);
}

static void
activated (EthosPlugin *plugin)
{
  PlacemarksPluginPrivate *priv;
  GtkUIManager *manager;
  GtkListStore *store;

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

  store = gtk_list_store_new (COL_COUNT,
                              G_TYPE_STRING,       /* ID */
                              G_TYPE_STRING,       /* Name */
                              G_TYPE_FLOAT,        /* Latitude */
                              G_TYPE_STRING,       /* Latitude as a string */
                              G_TYPE_FLOAT,        /* Longitude */
                              G_TYPE_STRING,       /* Longitude as a string */
                              G_TYPE_INT,          /* Zoom level */
                              G_TYPE_STRING,       /* Zoom level as a string */
                              G_TYPE_UINT);        /* UI ID */
  priv->model = GTK_TREE_MODEL (store);
  priv->deleted_cb_id  =  g_signal_connect (priv->model,
                                            "row-deleted",
                                            G_CALLBACK (row_deleted_cb),
                                            plugin);

  load_placemarks (PLACEMARKS_PLUGIN (plugin));
  load_menus (PLACEMARKS_PLUGIN (plugin));
}

static void
deactivated (EthosPlugin *plugin)
{
  GtkUIManager *manager;
  PlacemarksPluginPrivate *priv;

  priv = PLACEMARKS_PLUGIN (plugin)->priv;
  manager = emerillon_window_get_ui_manager (priv->window);

  clear_menus (PLACEMARKS_PLUGIN (plugin));

  gtk_ui_manager_remove_ui (manager, priv->ui_id);
  gtk_ui_manager_remove_action_group (manager,
                                      priv->action_group);

  g_object_unref (priv->model);
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
  plugin->priv->placemark_count = 0;
  plugin->priv->deleted_cb_id = 0;
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
