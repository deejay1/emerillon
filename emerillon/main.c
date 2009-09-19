/*
 * Copyright (C) 2008 Marco Barisione <marco@barisione.org>
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

#include <glib.h>
#include <glib/gi18n.h>
#include <clutter-gtk/clutter-gtk.h>

#include "window.h"

int
main (int argc,
      char **argv)
{
  GtkWidget *window;

  bindtextdomain (PACKAGE, EMERILLON_LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);

  g_thread_init (NULL);
  gtk_init (&argc, &argv);
  gtk_clutter_init (&argc, &argv);

  g_set_application_name (_("Emerillon Map Viewer"));

  window = emerillon_window_new ();
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_widget_show (window);

  gtk_main ();

  return 0;
}
