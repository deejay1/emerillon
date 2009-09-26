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

#include "manager.h"

#include <string.h>
#include <ethos/ethos.h>
#include <gconf/gconf-client.h>

#include "config-keys.h"

static EthosManager *default_manager = NULL;

#define EMERILLON_MANAGER_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        EMERILLON_TYPE_MANAGER, \
        EmerillonManagerPrivate))

G_DEFINE_TYPE (EmerillonManager, emerillon_manager, ETHOS_TYPE_MANAGER);

struct _EmerillonManagerPrivate
{
  GConfClient *client;
};

static void
emerillon_manager_init (EmerillonManager *self)
{
  self->priv = EMERILLON_MANAGER_GET_PRIVATE (self);
}

static void
emerillon_manager_dispose (GObject *object)
{
  //EmerillonManager *self = EMERILLON_MANAGER (object);

  G_OBJECT_CLASS (emerillon_manager_parent_class)->dispose (object);
}

static void
emerillon_manager_finalize (GObject *object)
{
  G_OBJECT_CLASS (emerillon_manager_parent_class)->finalize (object);
}

static GObject *
emerillon_manager_constructor (GType type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  if (default_manager == NULL)
    {
      object = G_OBJECT_CLASS (emerillon_manager_parent_class)->constructor (
      type, n_construct_properties, construct_params);

      default_manager = ETHOS_MANAGER (object);
      g_object_add_weak_pointer (object, (gpointer) &default_manager);
    }
  else
    {
      object = g_object_ref (default_manager);
    }

  return object;
}

static void
gconf_list_free (GSList *conf)
{
  GSList *iter;

  for (iter = conf; iter; iter = iter->next)
    g_free (iter->data);

  g_slist_free (conf);
}

static gboolean
is_enabled (GSList *conf, EthosPluginInfo *plugin)
{
  GSList *iter;

  for (iter = conf; iter; iter = iter->next)
    {
      if (strcmp (iter->data, ethos_plugin_info_get_id (plugin)) == 0)
        return TRUE;
    }

  return FALSE;
}

static void
emerillon_manager_plugin_loaded (EthosManager    *emanager,
                                 EthosPluginInfo *plugin_info)
{
  GSList *conf_list;
  GError *error = NULL;
  EmerillonManager *manager = EMERILLON_MANAGER (emanager);

  conf_list = gconf_client_get_list (manager->priv->client,
      EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
      GCONF_VALUE_STRING,
      &error);

  if (error)
    {
      g_warning ("gconf: %s", error->message);
      g_error_free (error);
      error = NULL;
      return;
    }

  /* if the plugin is already in the list, stop */
  if (is_enabled (conf_list, plugin_info))
    {
      gconf_list_free (conf_list);
      return;
    }

  conf_list = g_slist_append (conf_list, g_strdup (ethos_plugin_info_get_id (plugin_info)));

  gconf_client_set_list (manager->priv->client,
      EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
      GCONF_VALUE_STRING,
      conf_list,
      &error);

  if (error)
    {
      g_warning ("gconf: %s", error->message);
      g_error_free (error);
      error = NULL;
    }

  gconf_list_free (conf_list);
}

static void
emerillon_manager_plugin_unloaded (EthosManager    *emanager,
                                   EthosPluginInfo *plugin_info)
{
  GSList *conf_list;
  GSList *iter;
  GError *error = NULL;
  EmerillonManager *manager = EMERILLON_MANAGER (emanager);

  conf_list = gconf_client_get_list (manager->priv->client,
      EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
      GCONF_VALUE_STRING,
      &error);

  if (error)
    {
      g_warning ("gconf: %s", error->message);
      g_error_free (error);
      error = NULL;
      return;
    }

  for (iter = conf_list; iter; iter = iter->next)
    {
      if (strcmp (iter->data, ethos_plugin_info_get_id (plugin_info)) == 0)
        conf_list = g_slist_delete_link (conf_list, iter);
    }

  gconf_client_set_list (manager->priv->client,
      EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
      GCONF_VALUE_STRING,
      conf_list,
      &error);

  if (error)
    {
      g_warning ("gconf: %s", error->message);
      g_error_free (error);
      error = NULL;
    }

  gconf_list_free (conf_list);
}

static void
emerillon_manager_initialized (EthosManager *emanager)
{
  GList  *list,
         *iter;
  GSList *conf_list;
  GError *error = NULL;
  EmerillonManager *manager = EMERILLON_MANAGER (emanager);

  g_return_if_fail (ETHOS_IS_MANAGER (manager));

  manager->priv->client = gconf_client_get_default ();
  conf_list = gconf_client_get_list (manager->priv->client,
      EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
      GCONF_VALUE_STRING,
      &error);

  if (error)
    {
      g_warning ("gconf: %s", error->message);
      g_error_free (error);
      error = NULL;
    }

  list = ethos_manager_get_plugin_info (emanager);
  for (iter = list; iter; iter = iter->next)
    {
      if (ethos_plugin_info_get_active (iter->data))
        {
          continue;
        }
      else if (is_enabled (conf_list, iter->data) &&
               !ethos_manager_load_plugin (emanager, iter->data, &error))
        {
          g_warning ("%s: %s",
                     ethos_plugin_info_get_id (iter->data),
                     error ? error->message : "Error loading");

          if (error)
            {
              //ethos_plugin_info_add_error (iter->data, error);
              g_error_free (error);
              error = NULL;
            }
        }
    }

  g_list_free (list);
  gconf_list_free (conf_list);
}

static void
emerillon_manager_class_init (EmerillonManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  EthosManagerClass *ethos_class = ETHOS_MANAGER_CLASS (klass);

  object_class->constructor = emerillon_manager_constructor;
  object_class->dispose = emerillon_manager_dispose;
  object_class->finalize = emerillon_manager_finalize;

  ethos_class->initialized = emerillon_manager_initialized;
  ethos_class->plugin_loaded = emerillon_manager_plugin_loaded;
  ethos_class->plugin_unloaded = emerillon_manager_plugin_unloaded;

  g_type_class_add_private (object_class, sizeof (EmerillonManagerPrivate));
}

EthosManager *
emerillon_manager_dup_default (void)
{
  return g_object_new (EMERILLON_TYPE_MANAGER, NULL);
}
