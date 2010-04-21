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
#include <ethos/ethos.h>

#include <stdlib.h>

#include "manager.h"
#include "window.h"

static void
display_version()
{
  g_print (_("%s - Version %s\n"), g_get_application_name(), PACKAGE_VERSION);
  exit (0);
}

static GOptionEntry entries[]  =
{
  { "version", 'V', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, display_version,
      N_("Show version information and exit"), NULL},
  {NULL}
};

void
parse_options(int *argc,
      char ***argv)
{
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new (_("- map viewer"));
  g_option_context_add_group(context, gtk_get_option_group (TRUE));
  g_option_context_add_main_entries(context, entries, GETTEXT_PACKAGE);
  if (!g_option_context_parse (context, argc, argv, &error))
    {
      g_print ("%s\n", error->message);
      g_print (_("Run '%s --help' to see a list of available command line options.\n"), *argv[0]);
      g_error_free(error);
      exit (1);
    }
}

int
main (int argc,
      char **argv)
{
  EthosManager *manager;
  GtkWidget *window;
  gchar *user_data;
  gchar *plugin_dirs[3] = {EMERILLON_PLUGINDIR,
                           NULL,
                           NULL};

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
  gtk_init (&argc, &argv);
  gtk_clutter_init (&argc, &argv);

  g_set_application_name (_("Emerillon Map Viewer"));

  parse_options(&argc, &argv);

  window = emerillon_window_dup_default ();
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_widget_show (window);

  /* Setup the plugin infrastructure */
  manager = emerillon_manager_dup_default ();
  ethos_manager_set_app_name (manager, "Emerillon");
  ethos_manager_set_plugin_dirs (manager, (gchar **)plugin_dirs);

  ethos_manager_initialize (manager);

  gtk_main ();

  g_free (user_data);
  return 0;
}
