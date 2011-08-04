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

#include "map-position.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "emerillon/emerillon.h"

static void
peas_activatable_iface_init (PeasActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (MapPositionPlugin, map_position_plugin, PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (PEAS_TYPE_ACTIVATABLE,
                                                               peas_activatable_iface_init));

struct _MapPositionPluginPrivate
{
  EmerillonWindow *window;
  ChamplainView *map_view;

  GtkStatusbar *statusbar;
  guint signal_id;
};

static void
moved_cb (GObject *gobject,
          GParamSpec *pspec,
          MapPositionPlugin *plugin)
{
  gdouble lat, lon;
  gchar *position;
  MapPositionPluginPrivate *priv;

  priv = MAP_POSITION_PLUGIN (plugin)->priv;
  g_object_get (priv->map_view,
                "latitude", &lat,
                "longitude", &lon,
                NULL);

  position = g_strdup_printf ("%f; %f", lat, lon);

  gtk_statusbar_pop (priv->statusbar, 0);
  gtk_statusbar_push (priv->statusbar, 0, position);

  g_free (position);
}

static void
map_position_plugin_activate (PeasActivatable *plugin)
{
  MapPositionPluginPrivate *priv;

  priv = MAP_POSITION_PLUGIN (plugin)->priv;
  priv->window = EMERILLON_WINDOW (emerillon_window_dup_default ());
  priv->map_view = emerillon_window_get_map_view (priv->window);

  priv->statusbar = GTK_STATUSBAR (emerillon_window_get_statusbar (priv->window));

  priv->signal_id = g_signal_connect (priv->map_view,
                                      "notify::latitude",
                                      G_CALLBACK (moved_cb),
                                      plugin);

  moved_cb (NULL, NULL, MAP_POSITION_PLUGIN (plugin));
}

static void
map_position_plugin_deactivate (PeasActivatable *plugin)
{
  MapPositionPluginPrivate *priv;

  priv = MAP_POSITION_PLUGIN (plugin)->priv;
  g_signal_handler_disconnect (priv->map_view, priv->signal_id);

  gtk_statusbar_pop (priv->statusbar, 0);
}

static void
map_position_plugin_class_init (MapPositionPluginClass *klass)
{
  g_type_class_add_private (klass, sizeof (MapPositionPluginPrivate));
}

static void
map_position_plugin_class_finalize(MapPositionPluginClass *klass)
{
}

static void
map_position_plugin_init (MapPositionPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              MAP_POSITION_TYPE_PLUGIN,
                                              MapPositionPluginPrivate);
}

static void
peas_activatable_iface_init (PeasActivatableInterface *iface)
{
  iface->activate = map_position_plugin_activate;
  iface->deactivate = map_position_plugin_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  map_position_plugin_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              PEAS_TYPE_ACTIVATABLE,
                                              MAP_POSITION_TYPE_PLUGIN);
}
