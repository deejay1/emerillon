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

#include "preferences.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "manager.h"

static GtkWidget *default_preferences = NULL;

#define EMERILLON_PREFERENCES_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        EMERILLON_TYPE_PREFERENCES, \
        EmerillonPreferencesPrivate))

G_DEFINE_TYPE (EmerillonPreferences, emerillon_preferences, GTK_TYPE_DIALOG);

struct _EmerillonPreferencesPrivate
{
  gpointer filler;
};

static void     build_ui        (EmerillonPreferences *self);

static void
emerillon_preferences_init (EmerillonPreferences *self)
{
  self->priv = EMERILLON_PREFERENCES_GET_PRIVATE (self);
}

static void
emerillon_preferences_dispose (GObject *object)
{
  //EmerillonPreferences *self = EMERILLON_PREFERENCES (object);

  G_OBJECT_CLASS (emerillon_preferences_parent_class)->dispose (object);
}

static void
emerillon_preferences_finalize (GObject *object)
{
  G_OBJECT_CLASS (emerillon_preferences_parent_class)->finalize (object);
}

static GObject *
emerillon_preferences_constructor (GType type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  if (default_preferences == NULL)
    {
      object = G_OBJECT_CLASS (emerillon_preferences_parent_class)->constructor (
      type, n_construct_properties, construct_params);

      build_ui (EMERILLON_PREFERENCES (object));

      default_preferences = GTK_WIDGET (object);
      g_object_add_weak_pointer (object, (gpointer) &default_preferences);
    }
  else
    {
      object = g_object_ref (default_preferences);
    }

  return object;
}

static void
emerillon_preferences_class_init (EmerillonPreferencesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = emerillon_preferences_constructor;
  object_class->dispose = emerillon_preferences_dispose;
  object_class->finalize = emerillon_preferences_finalize;

  g_type_class_add_private (object_class, sizeof (EmerillonPreferencesPrivate));
}

GtkWidget *
emerillon_preferences_dup_default (void)
{
  return g_object_new (EMERILLON_TYPE_PREFERENCES, NULL);
}

static void
build_plugin_tab (GtkNotebook *notebook)
{
  GtkWidget *tab;
  GtkWidget *label;
  PeasEngine *manager;

  label = gtk_label_new (_("Plugins"));
  manager = peas_engine_get_default();
  tab = peas_gtk_plugin_manager_new (manager);
  gtk_widget_show (tab);
  gtk_container_set_border_width (GTK_CONTAINER (tab), 10);

  gtk_notebook_append_page (notebook, tab, label);
}

static void
build_ui (EmerillonPreferences *self)
{
  GtkWidget *area, *notebook;
  GtkDialog *dialog = GTK_DIALOG (self);

  gtk_window_set_title (GTK_WINDOW (self), _("Emerillon Preferences"));

  /* gtk_dialog_set_has_separator (dialog, FALSE); */
  gtk_dialog_add_button (dialog, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
  gtk_widget_set_size_request (GTK_WIDGET (dialog), 400, 400);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

  area = gtk_dialog_get_content_area (dialog);

  notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (area), notebook);
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);

  build_plugin_tab (GTK_NOTEBOOK (notebook));
}
