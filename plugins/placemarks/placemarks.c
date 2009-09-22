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

#include "placemarks.h"
#include "emerillon/emerillon.h"

G_DEFINE_TYPE (PlacemarksPlugin, placemarks_plugin, ETHOS_TYPE_PLUGIN)

struct _PlacemarksPluginPrivate
{
  GtkWidget *placemarks;
};

static void
activated (EthosPlugin *plugin)
{
  g_print("Activated!\n");
}

static void
deactivated (EthosPlugin *plugin)
{
  g_print("Deactivated!\n");
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
