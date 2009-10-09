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
#include <ethos/ethos.h>
#include <ethos/ethos-ui.h>

G_BEGIN_DECLS

#define MOUSE_POSITION_TYPE_PLUGIN            (map_position_plugin_get_type())
#define MOUSE_POSITION_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOUSE_POSITION_TYPE_PLUGIN, MousePositionPlugin))
#define MOUSE_POSITION_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOUSE_POSITION_TYPE_PLUGIN, MousePositionPluginClass))
#define MOUSE_POSITION_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOUSE_POSITION_TYPE_PLUGIN))
#define MOUSE_POSITION_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOUSE_POSITION_TYPE_PLUGIN))
#define MOUSE_POSITION_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOUSE_POSITION_TYPE_PLUGIN, MousePositionPluginClass))

typedef struct _MousePositionPlugin        MousePositionPlugin;
typedef struct _MousePositionPluginClass   MousePositionPluginClass;
typedef struct _MousePositionPluginPrivate MousePositionPluginPrivate;

struct _MousePositionPlugin
{
  EthosPlugin parent;

  /*< private >*/
  MousePositionPluginPrivate *priv;
};

struct _MousePositionPluginClass
{
  EthosPluginClass parent_class;
};

GType        map_position_plugin_get_type (void);
EthosPlugin* map_position_plugin_new      (void);

G_END_DECLS

#endif /* __MOUSE_POSITION_PLUGIN_H__ */
