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

#ifndef __SEARCH_PLUGIN_H__
#define __SEARCH_PLUGIN_H__

#include <glib-object.h>
#include <ethos/ethos.h>
#include <ethos/ethos-ui.h>

G_BEGIN_DECLS

#define SEARCH_TYPE_PLUGIN            (search_plugin_get_type())
#define SEARCH_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SEARCH_TYPE_PLUGIN, SearchPlugin))
#define SEARCH_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  SEARCH_TYPE_PLUGIN, SearchPluginClass))
#define SEARCH_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SEARCH_TYPE_PLUGIN))
#define SEARCH_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  SEARCH_TYPE_PLUGIN))
#define SEARCH_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  SEARCH_TYPE_PLUGIN, SearchPluginClass))

typedef struct _SearchPlugin        SearchPlugin;
typedef struct _SearchPluginClass   SearchPluginClass;
typedef struct _SearchPluginPrivate SearchPluginPrivate;

struct _SearchPlugin
{
  EthosPlugin parent;

  /*< private >*/
  SearchPluginPrivate *priv;
};

struct _SearchPluginClass
{
  EthosPluginClass parent_class;
};

GType        search_plugin_get_type (void);
EthosPlugin* search_plugin_new      (void);

G_END_DECLS

#endif /* __SEARCH_PLUGIN_H__ */
