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

#include "copy-link.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "emerillon/emerillon.h"

G_DEFINE_TYPE (CopyLinkPlugin, copy_link_plugin, ETHOS_TYPE_PLUGIN)

#define OSM_ID "copy_link_osm"
#define GOOGLE_ID "copy_link_google"
#define YAHOO_ID "copy_link_yahoo"

struct _CopyLinkPluginPrivate
{
  EmerillonWindow *window;
  ChamplainView *map_view;

  GtkActionGroup *action_group;
  guint ui_id;
  guint osm_ui_id;
  guint google_ui_id;
  guint yahoo_ui_id;
};

#define LEN 255

static void
copy_cb (GtkAction *action,
         CopyLinkPlugin *plugin)
{
  const gchar *id;
  gdouble lat, lon;
  gchar slat[LEN], slon[LEN];
  gint zoom;
  gchar *url = NULL;
  GtkClipboard *clipboard;
  CopyLinkPluginPrivate *priv;

  priv = COPY_LINK_PLUGIN (plugin)->priv;
  g_object_get (priv->map_view,
                "latitude", &lat,
                "longitude", &lon,
                "zoom-level", &zoom,
                NULL);

  id = gtk_action_get_name (action);
  g_ascii_dtostr (slat, LEN, lat);
  g_ascii_dtostr (slon, LEN, lon);

  if (strcmp (id, OSM_ID) == 0)
    {
      url = g_strdup_printf ("http://www.openstreetmap.org/?lat=%s&lon=%s&zoom=%d", slat, slon, zoom);
    }
  else if (strcmp (id, GOOGLE_ID) == 0)
    {
      url = g_strdup_printf ("http://maps.google.com?ll=%s,%s&z=%d", slat, slon, zoom);
    }
  else if (strcmp (id, YAHOO_ID) == 0)
    {
      zoom += 1;
      if (zoom < 2)
        zoom = 2;
      url = g_strdup_printf ("http://maps.yahoo.com/#mvt=m&lat=%s&lon=%s&zoom=%d", slat, slon, zoom);
    }

  clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text (clipboard, url, -1);

  g_free (url);
}

static guint
append_menu_item (CopyLinkPlugin *plugin,
                  const gchar *id,
                  const gchar *name)
{
  CopyLinkPluginPrivate *priv;
  GtkUIManager *manager;
  GError *error = NULL;
  gchar * item_ui_definition;
  GtkActionEntry actions[] = {
    { id,
      NULL,
      name,
      NULL,
      N_("Copy to clipboard the link to this web service"),
      G_CALLBACK (copy_cb) }
  };
  guint ui_id;

  priv = COPY_LINK_PLUGIN (plugin)->priv;
  manager = emerillon_window_get_ui_manager (priv->window);

  item_ui_definition = g_strconcat (
    "<ui>"
      "<menubar name=\"MainMenu\">"
          "<menu name=\"Edit\" action=\"Edit\">"
            "<placeholder name=\"EditPluginMenu\">"
              "<menu name=\"CopyLinkMenu\" action=\"CopyLinkAction\">"
                "<menuitem action=\"", id, "\"/>"
              "</menu>"
            "</placeholder>"
          "</menu>"
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
      g_warning ("Error adding UI %s", error->message);
      g_error_free (error);
    }

  g_free (item_ui_definition);
  return ui_id;
}

static void
load_menus (CopyLinkPlugin *plugin)
{
  CopyLinkPluginPrivate *priv;

  priv = COPY_LINK_PLUGIN (plugin)->priv;

  priv->osm_ui_id = append_menu_item (plugin, OSM_ID, _("OpenStreetMap"));
  priv->yahoo_ui_id = append_menu_item (plugin, YAHOO_ID, _("Yahoo! Maps"));
  priv->google_ui_id = append_menu_item (plugin, GOOGLE_ID, _("Google Maps"));
}

static const gchar * const ui_definition =
    "<ui>"
      "<menubar name=\"MainMenu\">"
          "<menu name=\"Edit\" action=\"Edit\">"
            "<placeholder name=\"EditPluginMenu\">"
              "<menu name=\"CopyLinkMenu\" action=\"CopyLinkAction\" />"
            "</placeholder>"
          "</menu>"
      "</menubar>"
    "</ui>";

static const GtkActionEntry action_entries[] =
{
  { "CopyLinkAction",   NULL, N_("_Copy link to") },

};

static void
activated (EthosPlugin *plugin)
{
  CopyLinkPluginPrivate *priv;
  GtkUIManager *manager;
  GError *error = NULL;

  priv = COPY_LINK_PLUGIN (plugin)->priv;
  priv->window = EMERILLON_WINDOW (emerillon_window_dup_default ());
  priv->map_view = emerillon_window_get_map_view (priv->window);

  manager = emerillon_window_get_ui_manager (priv->window);

  priv->action_group = gtk_action_group_new ("CopyLinkActions");
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
                                                   -1, &error);
  if (priv->ui_id == 0)
    {
      g_warning ("Error adding UI %s", error->message);
      g_error_free (error);
    }

  load_menus (COPY_LINK_PLUGIN (plugin));
}

static void
deactivated (EthosPlugin *plugin)
{
  GtkUIManager *manager;
  CopyLinkPluginPrivate *priv;

  priv = COPY_LINK_PLUGIN (plugin)->priv;
  manager = emerillon_window_get_ui_manager (priv->window);

  gtk_ui_manager_remove_ui (manager, priv->osm_ui_id);
  gtk_ui_manager_remove_ui (manager, priv->google_ui_id);
  gtk_ui_manager_remove_ui (manager, priv->yahoo_ui_id);

  gtk_ui_manager_remove_ui (manager, priv->ui_id);
}

static void
copy_link_plugin_class_init (CopyLinkPluginClass *klass)
{
  EthosPluginClass *plugin_class;

  g_type_class_add_private (klass, sizeof (CopyLinkPluginPrivate));

  plugin_class = ETHOS_PLUGIN_CLASS (klass);
  plugin_class->activated = activated;
  plugin_class->deactivated = deactivated;
}

static void
copy_link_plugin_init (CopyLinkPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              COPY_LINK_TYPE_PLUGIN,
                                              CopyLinkPluginPrivate);
}

EthosPlugin*
copy_link_plugin_new (void)
{
  return g_object_new (COPY_LINK_TYPE_PLUGIN, NULL);
}

G_MODULE_EXPORT EthosPlugin*
ethos_plugin_register (void)
{
  return copy_link_plugin_new ();
}
