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

#ifndef __PLACEMARKS_PLUGIN_H__
#define __PLACEMARKS_PLUGIN_H__

#include <glib-object.h>
#include <ethos/ethos.h>
#include <ethos/ethos-ui.h>

G_BEGIN_DECLS

#define PLACEMARKS_TYPE_PLUGIN            (placemarks_plugin_get_type())
#define PLACEMARKS_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLACEMARKS_TYPE_PLUGIN, PlacemarksPlugin))
#define PLACEMARKS_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PLACEMARKS_TYPE_PLUGIN, PlacemarksPluginClass))
#define PLACEMARKS_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLACEMARKS_TYPE_PLUGIN))
#define PLACEMARKS_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PLACEMARKS_TYPE_PLUGIN))
#define PLACEMARKS_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PLACEMARKS_TYPE_PLUGIN, PlacemarksPluginClass))

typedef struct _PlacemarksPlugin        PlacemarksPlugin;
typedef struct _PlacemarksPluginClass   PlacemarksPluginClass;
typedef struct _PlacemarksPluginPrivate PlacemarksPluginPrivate;

struct _PlacemarksPlugin
{
  EthosPlugin parent;

  /*< private >*/
  PlacemarksPluginPrivate *priv;
};

struct _PlacemarksPluginClass
{
  EthosPluginClass parent_class;
};

GType        placemarks_plugin_get_type (void);
EthosPlugin* placemarks_plugin_new      (void);

G_END_DECLS

#endif /* __PLACEMARKS_PLUGIN_H__ */
