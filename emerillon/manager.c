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
#include <libpeas/peas.h>
#include "config-keys.h"

static PeasEngine *default_manager = NULL;

#define EMERILLON_MANAGER_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        EMERILLON_TYPE_MANAGER, \
        EmerillonManagerPrivate))

G_DEFINE_TYPE (EmerillonManager, emerillon_manager, PEAS_TYPE_ENGINE);

struct _EmerillonManagerPrivate
{
  GSettings *settings_plugins;
};

static gboolean
is_enabled (gchar ** conf, PeasPluginInfo *plugin)
{
  int i;
  for (i = 0; conf[i] != NULL; i++)
    {
      if (strcmp (conf[i], peas_plugin_info_get_module_name (plugin)) == 0)
        return TRUE;
    }

  return FALSE;
}

static void
emerillon_manager_initialize (EmerillonManager *manager)
{
  const GList  *list,
  *iter;
  gchar **conf_list;

  manager->priv->settings_plugins = g_settings_new (EMERILLON_SCHEMA_PLUGINS);
  conf_list = g_settings_get_strv (manager->priv->settings_plugins,
                                   EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS);

  list = peas_engine_get_plugin_list (PEAS_ENGINE(manager));
  for (iter = list; iter; iter = iter->next)
  {
    if (peas_plugin_info_is_loaded (iter->data))
    {
      continue;
    }
    else if (is_enabled (conf_list, iter->data) &&
             !peas_engine_load_plugin (PEAS_ENGINE(manager), iter->data))
    {
      g_warning ("%s: %s",
                 peas_plugin_info_get_module_name (iter->data),
                 "Error loading");
	}
  }

  g_strfreev (conf_list);
}

static void
emerillon_manager_init (EmerillonManager *self)
{
  self->priv = EMERILLON_MANAGER_GET_PRIVATE (self);
  emerillon_manager_initialize(self);
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

      default_manager = PEAS_ENGINE (object);
      g_object_add_weak_pointer (object, (gpointer) &default_manager);
    }
  else
    {
      object = g_object_ref (default_manager);
    }

  return object;
}

static void
emerillon_manager_plugin_loaded (PeasEngine    *engine,
                                 PeasPluginInfo *plugin_info)
{
  gchar **conf_list;
  GPtrArray *tmp;
  GVariant *value;
  int i;
  gboolean res;
  EmerillonManager *manager = EMERILLON_MANAGER (engine);

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

  g_ptr_array_add (tmp, g_strdup (peas_plugin_info_get_module_name (plugin_info)));

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
emerillon_manager_plugin_unloaded (PeasEngine    *engine,
                                   PeasPluginInfo *plugin_info)
{
  gboolean res;
  int i;
  gchar **conf_list;
  GPtrArray *tmp;
  GVariant *value;

  EmerillonManager *manager = EMERILLON_MANAGER (engine);
	
  tmp = g_ptr_array_new_with_free_func (g_free);

  conf_list = g_settings_get_strv (manager->priv->settings_plugins,
                                   EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS);

  for (i=0; conf_list[i] != NULL; i++)
  {
    if (!strcmp (conf_list[i], peas_plugin_info_get_module_name (plugin_info)) == 0)
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
emerillon_manager_class_init (EmerillonManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  PeasEngineClass *ethos_class = PEAS_ENGINE_CLASS (klass);

  object_class->constructor = emerillon_manager_constructor;
  object_class->dispose = emerillon_manager_dispose;
  object_class->finalize = emerillon_manager_finalize;

  ethos_class->load_plugin = emerillon_manager_plugin_loaded;
  ethos_class->unload_plugin = emerillon_manager_plugin_unloaded;

  g_type_class_add_private (object_class, sizeof (EmerillonManagerPrivate));
}

PeasEngine *
emerillon_manager_dup_default (void)
{
  return g_object_new (EMERILLON_TYPE_MANAGER, NULL);
}
