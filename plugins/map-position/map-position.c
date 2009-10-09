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

G_DEFINE_TYPE (MousePositionPlugin, map_position_plugin, ETHOS_TYPE_PLUGIN)

struct _MousePositionPluginPrivate
{
  EmerillonWindow *window;
  ChamplainView *map_view;

  GtkStatusbar *statusbar;
  guint signal_id;
};

static void
moved_cb (GObject *gobject,
          GParamSpec *pspec,
          MousePositionPlugin *plugin)
{
  gdouble lat, lon;
  gchar *position;
  MousePositionPluginPrivate *priv;

  priv = MOUSE_POSITION_PLUGIN (plugin)->priv;
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
activated (EthosPlugin *plugin)
{
  MousePositionPluginPrivate *priv;

  priv = MOUSE_POSITION_PLUGIN (plugin)->priv;
  priv->window = EMERILLON_WINDOW (emerillon_window_dup_default ());
  priv->map_view = emerillon_window_get_map_view (priv->window);

  priv->statusbar = GTK_STATUSBAR (emerillon_window_get_statusbar (priv->window));

  priv->signal_id = g_signal_connect (priv->map_view,
                                      "notify::latitude",
                                      G_CALLBACK (moved_cb),
                                      plugin);

  moved_cb (NULL, NULL, MOUSE_POSITION_PLUGIN (plugin));
}

static void
deactivated (EthosPlugin *plugin)
{
  MousePositionPluginPrivate *priv;

  priv = MOUSE_POSITION_PLUGIN (plugin)->priv;
  g_signal_handler_disconnect (priv->map_view, priv->signal_id);
}

static void
map_position_plugin_class_init (MousePositionPluginClass *klass)
{
  EthosPluginClass *plugin_class;

  g_type_class_add_private (klass, sizeof (MousePositionPluginPrivate));

  plugin_class = ETHOS_PLUGIN_CLASS (klass);
  plugin_class->activated = activated;
  plugin_class->deactivated = deactivated;
}

static void
map_position_plugin_init (MousePositionPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              MOUSE_POSITION_TYPE_PLUGIN,
                                              MousePositionPluginPrivate);
}

EthosPlugin*
map_position_plugin_new (void)
{
  return g_object_new (MOUSE_POSITION_TYPE_PLUGIN, NULL);
}

G_MODULE_EXPORT EthosPlugin*
ethos_plugin_register (void)
{
  return map_position_plugin_new ();
}
