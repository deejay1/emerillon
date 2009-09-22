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

#include "search.h"
#include "emerillon/emerillon.h"

G_DEFINE_TYPE (SearchPlugin, search_plugin, ETHOS_TYPE_PLUGIN)

struct _SearchPluginPrivate
{
  GtkWidget *search;
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
search_plugin_class_init (SearchPluginClass *klass)
{
  EthosPluginClass *plugin_class;

  g_type_class_add_private (klass, sizeof (SearchPluginPrivate));

  plugin_class = ETHOS_PLUGIN_CLASS (klass);
  plugin_class->activated = activated;
  plugin_class->deactivated = deactivated;
}

static void
search_plugin_init (SearchPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              SEARCH_TYPE_PLUGIN,
                                              SearchPluginPrivate);
}

EthosPlugin*
search_plugin_new (void)
{
  return g_object_new (SEARCH_TYPE_PLUGIN, NULL);
}

G_MODULE_EXPORT EthosPlugin*
ethos_plugin_register (void)
{
  return search_plugin_new ();
}
