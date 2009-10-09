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

#ifndef __COPY_LINK_PLUGIN_H__
#define __COPY_LINK_PLUGIN_H__

#include <glib-object.h>
#include <ethos/ethos.h>
#include <ethos/ethos-ui.h>

G_BEGIN_DECLS

#define COPY_LINK_TYPE_PLUGIN            (copy_link_plugin_get_type())
#define COPY_LINK_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), COPY_LINK_TYPE_PLUGIN, CopyLinkPlugin))
#define COPY_LINK_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  COPY_LINK_TYPE_PLUGIN, CopyLinkPluginClass))
#define COPY_LINK_IS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), COPY_LINK_TYPE_PLUGIN))
#define COPY_LINK_IS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  COPY_LINK_TYPE_PLUGIN))
#define COPY_LINK_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  COPY_LINK_TYPE_PLUGIN, CopyLinkPluginClass))

typedef struct _CopyLinkPlugin        CopyLinkPlugin;
typedef struct _CopyLinkPluginClass   CopyLinkPluginClass;
typedef struct _CopyLinkPluginPrivate CopyLinkPluginPrivate;

struct _CopyLinkPlugin
{
  EthosPlugin parent;

  /*< private >*/
  CopyLinkPluginPrivate *priv;
};

struct _CopyLinkPluginClass
{
  EthosPluginClass parent_class;
};

GType        copy_link_plugin_get_type (void);
EthosPlugin* copy_link_plugin_new      (void);

G_END_DECLS

#endif /* __COPY_LINK_PLUGIN_H__ */
