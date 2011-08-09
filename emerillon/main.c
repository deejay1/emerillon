/*
 * Copyright (C) 2008 Marco Barisione <marco@barisione.org>
 * Copyright (C) 2009 Novopia Inc.
 * Copyright (C) 2010 Emerillon Contributors
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

#include <glib.h>
#include <glib/gi18n.h>
#include <clutter-gtk/clutter-gtk.h>
#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#ifdef HAVE_INTROSPECTION
#include <gobject-introspection-1.0/girepository.h>
#endif

#include <stdlib.h>

#include "window.h"
#include "config-keys.h"

static void
display_version ()
{
  g_print (_("%s - Version %s\n"), g_get_application_name (), PACKAGE_VERSION);
  exit (0);
}

static gdouble lat = 2555;
static gdouble lon = 2555;
// Defines if all required coordinates were set on the command line
static gboolean coords_set = FALSE;

static GOptionEntry entries[]  =
{
  { "version", 'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, display_version,
      N_("Show version information and exit"), NULL},
  {NULL}
};

int
main (int argc,
      char **argv)
{
  PeasEngine *engine;
  GtkWidget *window;
  ChamplainView *map_view = NULL;
  GError *error = NULL;
  GFile *plugin_dir;
  gchar *user_data;
  gchar *plugin_dirs[3] = {EMERILLON_PLUGINDIR,
                           NULL,
                           NULL};
  gchar **dir = NULL;
  GSettings *settings;

  user_data = g_build_path (G_DIR_SEPARATOR_S,
                            g_get_user_data_dir (),
                            "emerillon",
                            "plugins",
                            NULL);
  plugin_dirs[1] = user_data;

  bindtextdomain (PACKAGE, EMERILLON_LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);

  g_thread_init (NULL);

  gtk_clutter_init_with_args (&argc, &argv,
      _("- map viewer"), entries, GETTEXT_PACKAGE, &error);
  //gtk_init (&argc, &argv);

  g_set_application_name (_("Emerillon Map Viewer"));

  window = emerillon_window_dup_default ();
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_widget_show (window);

  /* Go to the position specified on the command line with a sensible default zoom */
  if (coords_set)
  {
    map_view = emerillon_window_get_map_view ((EmerillonWindow *) window);
    g_object_set (map_view, "zoom-level", 14, NULL);
    champlain_view_center_on (map_view, lat, lon);
  } else {
    g_object_set(window, "auto-update", TRUE, NULL);
  }

  /* Create the user plugin directory */
  plugin_dir = g_file_new_for_path (plugin_dirs[1]);
  if (!g_file_query_exists (plugin_dir, NULL))
    {
      g_file_make_directory_with_parents (plugin_dir, NULL, &error);
      if (error)
        {
          g_warning ("%s", error->message);
          g_error_free (error);
        }
    }
  g_object_unref (plugin_dir);

  /* Setup the plugin infrastructure */
  engine = peas_engine_get_default();
  peas_engine_enable_loader (engine, "python");
  g_irepository_require (g_irepository_get_default (),
                         "Peas", "1.0", 0, NULL);
  
  for (dir = plugin_dirs; *dir != NULL; dir++)
  {
    peas_engine_add_search_path (engine, *dir, NULL);
  }
  settings = g_settings_new (EMERILLON_SCHEMA_PLUGINS);
  g_settings_bind (settings,
                   EMERILLON_CONF_PLUGINS_ACTIVE_PLUGINS,
                   engine,
                   "loaded-plugins",
                   G_SETTINGS_BIND_DEFAULT);
  gtk_main ();

  g_free (user_data);
  return 0;
}
