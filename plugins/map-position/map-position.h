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

#ifndef __MOUSE_POSITION_PLUGIN_H__
#define __MOUSE_POSITION_PLUGIN_H__

#include <glib-object.h>
#include <libpeas/peas.h>

G_BEGIN_DECLS

#define MAP_POSITION_TYPE_PLUGIN            (map_position_plugin_get_type())
#define MAP_POSITION_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MAP_POSITION_TYPE_PLUGIN, MapPositionPlugin))
#define MAP_POSITION_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MAP_POSITION_TYPE_PLUGIN, MapPositionPluginClass))
#define MAP_POSITION_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAP_POSITION_TYPE_PLUGIN))
#define MAP_POSITION_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MAP_POSITION_TYPE_PLUGIN))
#define MAP_POSITION_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MAP_POSITION_TYPE_PLUGIN, MapPositionPluginClass))

typedef struct _MapPositionPlugin        MapPositionPlugin;
typedef struct _MapPositionPluginClass   MapPositionPluginClass;
typedef struct _MapPositionPluginPrivate MapPositionPluginPrivate;

struct _MapPositionPlugin
{
  PeasExtensionBase parent;

  /*< private >*/
  MapPositionPluginPrivate *priv;
};

struct _MapPositionPluginClass
{
  PeasExtensionBaseClass parent_class;
};

GType                map_position_plugin_get_type (void);
G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module);

G_END_DECLS

#endif /* __MOUSE_POSITION_PLUGIN_H__ */
