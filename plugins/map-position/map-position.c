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

#include "cut-paste/totem-plugin.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "emerillon/emerillon.h"

#define MAP_POSITION_TYPE_PLUGIN            (map_position_plugin_get_type())
#define MAP_POSITION_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MAP_POSITION_TYPE_PLUGIN, MapPositionPlugin))
#define MAP_POSITION_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MAP_POSITION_TYPE_PLUGIN, MapPositionPluginClass))
#define MAP_POSITION_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAP_POSITION_TYPE_PLUGIN))
#define MAP_POSITION_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MAP_POSITION_TYPE_PLUGIN))
#define MAP_POSITION_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MAP_POSITION_TYPE_PLUGIN, MapPositionPluginClass))


typedef struct
{
  EmerillonWindow *window;
  ChamplainView *map_view;

  GtkStatusbar *statusbar;
  guint signal_id;
} MapPositionPluginPrivate;

TOTEM_PLUGIN_REGISTER (MAP_POSITION_TYPE_PLUGIN, MapPositionPlugin, map_position_plugin);

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
impl_activate (PeasActivatable *plugin)
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
impl_deactivate (PeasActivatable *plugin)
{
  MapPositionPluginPrivate *priv;

  priv = MAP_POSITION_PLUGIN (plugin)->priv;
  g_signal_handler_disconnect (priv->map_view, priv->signal_id);

  gtk_statusbar_pop (priv->statusbar, 0);
}
