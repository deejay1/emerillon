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

#include "config-keys.h"

static EthosManager *default_manager = NULL;

#define EMERILLON_MANAGER_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        EMERILLON_TYPE_MANAGER, \
        EmerillonManagerPrivate))

G_DEFINE_TYPE (EmerillonManager, emerillon_manager, ETHOS_TYPE_MANAGER);

struct _EmerillonManagerPrivate
{
  GSettings *settings_plugins;
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

static gboolean
is_enabled (gchar ** conf, EthosPluginInfo *plugin)
{
  int i;
  for (i = 0; conf[i] != NULL; i++)
    {
      if (strcmp (conf[i], ethos_plugin_info_get_id (plugin)) == 0)
        return TRUE;
    }

  return FALSE;
}

static void
emerillon_manager_plugin_loaded (EthosManager    *emanager,
                                 EthosPluginInfo *plugin_info)
{
  gchar **conf_list;
  GPtrArray *tmp;
  GVariant *value;
  int i;
  gboolean res;
  EmerillonManager *manager = EMERILLON_MANAGER (emanager);

  tmp = g_ptr_array_new_with_free_func (g_free);

  conf_list = g_settings_get_strv (manager->priv->settings_plugins,
                                   EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS);


  /* if the plugin is already in the list, stop */
  if (is_enabled (conf_list, plugin_info))
  {
    g_strfreev (conf_list);
    return;
  }

  /* rebuilding as a pointer array, sorry */
  for (i=0;conf_list[i] != NULL;i++)
    g_ptr_array_add (tmp,g_strdup(conf_list[i]));

  g_ptr_array_add (tmp, g_strdup (ethos_plugin_info_get_id (plugin_info)));

  value = g_variant_new_strv  ((const gchar * const *) tmp->pdata, tmp->len);
  res = g_settings_set_value (manager->priv->settings_plugins,
                             EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
                             value);
  if (!res)
    g_warning ("gsettings: Key %s is not writable!",
               EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS);

  g_ptr_array_free (tmp, TRUE);
  g_strfreev (conf_list);
  g_variant_unref(value);
}

static void
emerillon_manager_plugin_unloaded (EthosManager    *emanager,
                                   EthosPluginInfo *plugin_info)
{
  gboolean res;
  int i;
  gchar **conf_list;
  GPtrArray *tmp;
  GVariant *value;
  
  EmerillonManager *manager = EMERILLON_MANAGER (emanager);

  tmp = g_ptr_array_new_with_free_func (g_free);

  conf_list = g_settings_get_strv (manager->priv->settings_plugins,
                                   EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS);

  for (i=0; conf_list[i] != NULL; i++)
  {
    if (!strcmp (conf_list[i], ethos_plugin_info_get_id (plugin_info)) == 0)
      g_ptr_array_add (tmp,g_strdup(conf_list[i]));
  }
  value = g_variant_new_strv  ((const gchar * const *) tmp->pdata, tmp->len);
  res = g_settings_set_value (manager->priv->settings_plugins,
                             EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
                             value);

  if (!res)
    g_warning ("gsettings: Key %s is not writable!",
               EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS);

  g_ptr_array_free (tmp, TRUE);
  g_strfreev (conf_list);
  g_variant_unref (value);
}

static void
emerillon_manager_initialized (EthosManager *emanager)
{
  GList  *list,
  *iter;
  gchar **conf_list;
  GError *error = NULL;

  EmerillonManager *manager = EMERILLON_MANAGER (emanager);

  g_return_if_fail (ETHOS_IS_MANAGER (manager));

  manager->priv->settings_plugins = g_settings_new (EMERILLON_SCHEMA_PLUGINS);
  conf_list = g_settings_get_strv (manager->priv->settings_plugins,
                                   EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS);

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
  g_strfreev (conf_list);
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
