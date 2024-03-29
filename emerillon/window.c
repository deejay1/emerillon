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

#include "window.h"
#include "preferences.h"

#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-position.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libpeas/peas.h>

#include "config-keys.h"
#include "sidebar.h"

static GtkWidget *default_window = NULL;

#define EMERILLON_WINDOW_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        EMERILLON_TYPE_WINDOW, \
        EmerillonWindowPrivate))

G_DEFINE_TYPE (EmerillonWindow, emerillon_window, GTK_TYPE_WINDOW);

struct _EmerillonWindowPrivate
{
  GtkUIManager *ui_manager;

  PeasExtensionSet* ext_set;
  
  GtkWidget *toolbar;
  GtkWidget *statusbar;
  GtkWidget *sidebar;
  GtkWidget *throbber;

  ChamplainView *view;

  GtkActionGroup *main_actions;

  GSettings *settings_ui;


  guint tooltip_message_context_id;

  /* Defines whether the view position should be updated based on geoclue information */
  gboolean position_auto_update;

  GeoclueMasterClient *geoclue_client;
  GeocluePosition *geoclue_position;
};

enum
{
  PROP_0,

  PROP_AUTO_UPDATE
};

static void     build_ui        (EmerillonWindow *self);
static void
position_changed_cb (GeocluePosition *position,
                     GeocluePositionFields fields,
                     int timestamp,
                     double latitude,
                     double longitude,
                     double altitude,
                     GeoclueAccuracy *accuracy,
                     GError *error,
                     EmerillonWindow *self);

static void
emerillon_window_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  EmerillonWindow *self = EMERILLON_WINDOW (object);
  switch (property_id)
  {
    case PROP_AUTO_UPDATE:
      self->priv->position_auto_update = g_value_get_boolean(value);
      if (self->priv->geoclue_position != NULL)
        geoclue_position_get_position_async (self->priv->geoclue_position,
                                             (GeocluePositionCallback)position_changed_cb, self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
emerillon_window_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  EmerillonWindow *self = EMERILLON_WINDOW (object);
  switch (property_id)
  {
    case PROP_AUTO_UPDATE:
      g_value_set_boolean (value, self->priv->position_auto_update);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static gboolean
set_zoom_for_accuracy (EmerillonWindow *self,
                       GeoclueAccuracy *accuracy)
{
  GeoclueAccuracyLevel accuracy_level;
  gint zoom_level;

  geoclue_accuracy_get_details (accuracy, &accuracy_level, NULL, NULL);

  switch (accuracy_level)
    {
      case GEOCLUE_ACCURACY_LEVEL_COUNTRY:
        zoom_level = 4;
        break;
      case GEOCLUE_ACCURACY_LEVEL_REGION:
        zoom_level = 7;
        break;
      case GEOCLUE_ACCURACY_LEVEL_LOCALITY:
        zoom_level = 9;
        break;
      case GEOCLUE_ACCURACY_LEVEL_POSTALCODE:
        zoom_level = 10;
        break;
      case GEOCLUE_ACCURACY_LEVEL_STREET:
        zoom_level = 14;
        break;
      case GEOCLUE_ACCURACY_LEVEL_DETAILED:
        zoom_level = 16;
        break;
      default:
        return FALSE;
    }

  g_object_set (self->priv->view, "zoom-level", zoom_level, NULL);

  return TRUE;
}

static void
position_changed_cb (GeocluePosition *position,
                     GeocluePositionFields fields,
                     int timestamp,
                     double latitude,
                     double longitude,
                     double altitude,
                     GeoclueAccuracy *accuracy,
                     GError *error,
                     EmerillonWindow *self)
{
  if (error)
    {
      g_printerr ("Error retrieving the current position: %s\n", error->message);
      g_error_free (error);
      return;
    }
  else if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE &&
           fields & GEOCLUE_POSITION_FIELDS_LONGITUDE && self->priv->position_auto_update)
    {
      /* FIXME: if the next calls are inverted then the wrong position is
       * shown (libchamplain bug). */
      set_zoom_for_accuracy (self, accuracy);
      champlain_view_center_on (self->priv->view, latitude, longitude);
    }
}

static void
on_extension_added (PeasExtensionSet *set,
                    PeasPluginInfo   *info,
                    PeasExtension    *exten,
                    EmerillonWindow  *self)
{
  peas_activatable_activate (PEAS_ACTIVATABLE (exten));
}

static void
on_extension_removed (PeasExtensionSet *set,
                      PeasPluginInfo   *info,
                      PeasExtension    *exten,
                      EmerillonWindow  *self)
{
  peas_activatable_deactivate (PEAS_ACTIVATABLE (exten));
}

static void
emerillon_window_init (EmerillonWindow *self)
{
  GdkGeometry geometry;
  GeoclueMaster *master = NULL;
  GError *error = NULL;

  gint width, height;

  self->priv = EMERILLON_WINDOW_GET_PRIVATE (self);


  self->priv->position_auto_update = FALSE;

  /* GSettings. */
  self->priv->settings_ui = g_settings_new (EMERILLON_SCHEMA_UI);

  /* Extension setup */
  self->priv->ext_set = peas_extension_set_new (peas_engine_get_default (),
                                                PEAS_TYPE_ACTIVATABLE,
                                                NULL);

  peas_extension_set_call (self->priv->ext_set, "activate");

  g_signal_connect (self->priv->ext_set, "extension-added", G_CALLBACK (on_extension_added), self);
  g_signal_connect (self->priv->ext_set, "extension-removed", G_CALLBACK (on_extension_removed), self);
  
  /* Window setup. */
  geometry.min_width = 400;
  geometry.min_height = 350;
  gtk_window_set_geometry_hints (GTK_WINDOW (self), GTK_WIDGET (self),
      &geometry,GDK_HINT_MIN_SIZE);

  /* Set the window size */
  width = g_settings_get_int (self->priv->settings_ui,
      EMERILLON_CONF_UI_WINDOW_WIDTH);
  height = g_settings_get_int (self->priv->settings_ui,
      EMERILLON_CONF_UI_WINDOW_HEIGHT);

  if (width > 0 && height > 0)
    gtk_window_set_default_size (GTK_WINDOW (self), width, height);
  else
    gtk_window_set_default_size (GTK_WINDOW (self), 640, 450);

  /* Current position. */
  master = geoclue_master_get_default ();
  self->priv->geoclue_client = geoclue_master_create_client (master, NULL, &error);
  if (!self->priv->geoclue_client)
    {
       g_warning ("Creating Geoclue Master client failed: %s", error->message);
       g_error_free (error);
       g_object_unref (master);
       return;
    }
  g_object_unref (master);

  geoclue_master_client_set_requirements (self->priv->geoclue_client,
                                          GEOCLUE_ACCURACY_LEVEL_COUNTRY, 0, FALSE, GEOCLUE_RESOURCE_ALL, NULL);
  self->priv->geoclue_position = geoclue_master_client_create_position (self->priv->geoclue_client, &error);
  if (self->priv->geoclue_position)
  {
    g_object_set_data (G_OBJECT (self->priv->geoclue_position), "client", self->priv->geoclue_client);
    g_signal_connect(self->priv->geoclue_position, "position-changed",
                     G_CALLBACK(position_changed_cb), NULL);
  }
  else
  {
    g_warning ("Getting Geoclue Position Failed: %s", error->message);
    g_error_free (error);
    if (self->priv->geoclue_client)
      g_object_unref (self->priv->geoclue_client);
  }

}

static void
emerillon_window_dispose (GObject *object)
{
  EmerillonWindow *self = EMERILLON_WINDOW (object);
  gint width, height;

  if (self->priv->main_actions != NULL)
    {
      g_object_unref (self->priv->main_actions);
      self->priv->main_actions = NULL;
    }

  if (self->priv->settings_ui)
    {
      /* Save the window size */
      gtk_window_get_size (GTK_WINDOW (self), &width, &height);
      width = g_settings_set_int (self->priv->settings_ui,
                                  EMERILLON_CONF_UI_WINDOW_WIDTH, width);
      height = g_settings_set_int (self->priv->settings_ui,
                                   EMERILLON_CONF_UI_WINDOW_HEIGHT, height);
      g_object_unref (self->priv->settings_ui);
      self->priv->settings_ui = NULL;
    }

  if (self->priv->geoclue_client)
    {
       g_object_unref (self->priv->geoclue_client);
       self->priv->geoclue_client = NULL;
    }
   if (self->priv->geoclue_position)
    {
       g_object_unref (self->priv->geoclue_position);
       self->priv->geoclue_client = NULL;
    }
   if (self->priv->ext_set)
    {
       g_object_unref (self->priv->ext_set);
       self->priv->ext_set = NULL;
    }
  
  G_OBJECT_CLASS (emerillon_window_parent_class)->dispose (object);
}

static void
emerillon_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (emerillon_window_parent_class)->finalize (object);
}

static GObject *
emerillon_window_constructor (GType type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  if (default_window == NULL)
    {
      object = G_OBJECT_CLASS (emerillon_window_parent_class)->constructor (
      type, n_construct_properties, construct_params);

      build_ui (EMERILLON_WINDOW (object));

      default_window = GTK_WIDGET (object);
      g_object_add_weak_pointer (object, (gpointer) &default_window);
    }
  else
    {
      object = g_object_ref (default_window);
    }

  return object;
}

static void
emerillon_window_class_init (EmerillonWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = emerillon_window_constructor;
  object_class->dispose = emerillon_window_dispose;
  object_class->finalize = emerillon_window_finalize;

  /* Initialize object properties */
  object_class->get_property = emerillon_window_get_property;
  object_class->set_property = emerillon_window_set_property;

  /**
   * EmerillonWindow:auto-update:
   *
   * Toggle automatic update of the map position based on geoclue data.
   * When enabled also gets the current position.
   *
   * Since 0.1.2
   */
  g_object_class_install_property(object_class, PROP_AUTO_UPDATE,
                                  g_param_spec_boolean ("auto-update", "Position auto update",
                                "Toggle automatic update of the map position based on geoclue data",
                                FALSE, G_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (EmerillonWindowPrivate));
}

/**
 * emerillon_window_dup_default:
 *
 * Retrieves Emerillon's main window
 *
 * Return value: (transfer none): A #GtkWidget
 */
GtkWidget *
emerillon_window_dup_default (void)
{
  return g_object_new (EMERILLON_TYPE_WINDOW,
      "type", GTK_WINDOW_TOPLEVEL,
      NULL);
}

static void
state_changed_cb (GtkWidget *widget,
                  GParamSpec *pspec,
                  EmerillonWindow *self)
{
  ChamplainState state;

  g_object_get (self->priv->view, "state", &state, NULL);
  if (state == CHAMPLAIN_STATE_LOADING)
    gtk_spinner_start (GTK_SPINNER (self->priv->throbber));
  else
    gtk_spinner_stop (GTK_SPINNER (self->priv->throbber));
}

static void
zoom_changed_cb (GtkWidget *widget,
                 GParamSpec *pspec,
                 EmerillonWindow *self)
{
  GtkAction *zoom_in_action;
  GtkAction *zoom_out_action;
  gint zoom_level;
  guint min_zoom_level = 0;
  guint max_zoom_level = 18;
  ChamplainMapSource *source = NULL;

  source = champlain_view_get_map_source (self->priv->view);
  min_zoom_level = champlain_map_source_get_min_zoom_level (source);
  max_zoom_level = champlain_map_source_get_max_zoom_level (source);

  zoom_in_action = gtk_action_group_get_action (self->priv->main_actions,
        "ViewZoomIn");
  zoom_out_action = gtk_action_group_get_action (self->priv->main_actions,
        "ViewZoomOut");

  g_object_get (self->priv->view, "zoom-level", &zoom_level, NULL);

  gtk_action_set_sensitive (zoom_in_action, zoom_level < max_zoom_level);
  gtk_action_set_sensitive (zoom_out_action, zoom_level > min_zoom_level);
}

/* TODO: use g_settings_bind instead of manual visibility switching
 * see also cmd_show_hide_bar()
*/
static void
update_ui_visibility (EmerillonWindow *self)
{
  gboolean visible;
  GtkAction *action;

  /* Toolbar. */
  visible = g_settings_get_boolean (self->priv->settings_ui,
      EMERILLON_CONF_UI_TOOLBAR);
  action = gtk_ui_manager_get_action (self->priv->ui_manager,
      "/MainMenu/View/ToolbarToggle");
  g_assert (action != NULL);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);
  g_object_set (G_OBJECT (self->priv->toolbar), "visible", visible, NULL);

  /* Statusbar. */
  visible = g_settings_get_boolean (self->priv->settings_ui,
      EMERILLON_CONF_UI_STATUSBAR);
  action = gtk_ui_manager_get_action (self->priv->ui_manager,
      "/MainMenu/View/StatusbarToggle");
  g_assert (action != NULL);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);
  g_object_set (G_OBJECT (self->priv->statusbar), "visible", visible, NULL);

  /* Sidebar. */
  visible = g_settings_get_boolean (self->priv->settings_ui,
      EMERILLON_CONF_UI_SIDEBAR);
  action = gtk_ui_manager_get_action (self->priv->ui_manager,
      "/MainMenu/View/SidebarToggle");
  g_assert (action != NULL);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);
  if (visible)
      gtk_widget_show (self->priv->sidebar);
  else
      gtk_widget_hide (self->priv->sidebar);
}

static void
cmd_quit (GtkAction *action,
          EmerillonWindow *self)
{
  gtk_widget_destroy (GTK_WIDGET (self));
  gtk_main_quit ();
}

static void
cmd_help (GtkAction *action,
          EmerillonWindow *self)
{
  GError *error = NULL;

  gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (self)), "ghelp:emerillon",
                gtk_get_current_event_time (), &error);

  if (error != NULL)
  {
    g_printerr ("Error opening help: %s\n", error->message);
    g_error_free (error);
  }
}

static void
cmd_about (GtkAction *action,
           EmerillonWindow *self)
{
  static const char *authors[] = {
      "Marco Barisione <marco@barisione.org>",
      "Pierre-Luc Beaudoin <pierre-luc.beaudoin@novopia.com>",
      "Simon Wenner <simon@wenner.ch>",
      NULL
  };
  static const char *documenters[] = {
      NULL
  };
  const char *license[] = {
      N_("This program is free software; you can redistribute it and/or "
          "modify it under the terms of the GNU General Public License as "
          "published by the Free Software Foundation; either version 2 of "
          "the License, or (at your option) any later version.\n"),
      N_("This program is distributed in the hope that it will be useful, "
          "but WITHOUT ANY WARRANTY; without even the implied warranty of "
          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
          "GNU General Public License for more details.\n"),
      N_("You should have received a copy of the GNU General Public License "
          "along with this program; if not, write to the Free Software "
          "Foundation, Inc., 59 Temple Place - Suite 330, Boston, "
          "MA 02111-1307, USA.")
  };
  const char *translators = _("translator-credits");
  char *license_translated;

  license_translated = g_strconcat (_(license[0]), "\n", _(license[1]), "\n",
      _(license[2]), "\n", NULL);

  gtk_show_about_dialog (GTK_WINDOW (self),
      "program-name", _("Emerillon"),
      "version", VERSION,
      "copyright", "Copyright \xc2\xa9 2008 Marco Barisione\n"
                   "Copyright \xc2\xa9 2009 Novopia Inc.\n"
                   "Copyright \xc2\xa9 2010 Emerillon Contributors.",
      "comments",_("A map viewer for the GNOME desktop"),
      "authors", authors,
      "documenters", documenters,
      "translator-credits", translators,
      "website", "http://projects.gnome.org/emerillon",
      //"logo-icon-name", "emerillon",
      "license", license_translated,
      "wrap-license", TRUE,
      NULL);

  g_free (license_translated);
}

static void
cmd_map_change_map (GtkRadioAction *action,
                    GtkRadioAction *current,
                    EmerillonWindow *self)
{
  gint value;
  ChamplainMapSourceFactory *factory;
  ChamplainMapSource *source;

  factory = champlain_map_source_factory_dup_default ();
  value = gtk_radio_action_get_current_value (current);

  switch (value)
    {
      default:
      case 0:
        source = champlain_map_source_factory_create_cached_source (factory,
            CHAMPLAIN_MAP_SOURCE_OSM_MAPNIK);
        break;
      case 1:
        source = champlain_map_source_factory_create_cached_source (factory,
            CHAMPLAIN_MAP_SOURCE_OSM_CYCLE_MAP);
        break;
      case 2:
        source = champlain_map_source_factory_create_cached_source (factory,
            CHAMPLAIN_MAP_SOURCE_OSM_TRANSPORT_MAP);
        break;
      case 3:
        source = champlain_map_source_factory_create_cached_source (factory,
            CHAMPLAIN_MAP_SOURCE_MFF_RELIEF);
        break;

    }
  champlain_view_set_map_source (self->priv->view, source);
  g_object_unref (factory);
}

static void
cmd_show_hide_bar (GtkAction *action,
                   EmerillonWindow *self)
{
  gboolean visible;
  const gchar *action_name;
  GtkWidget *target_widget;
  const gchar *target_conf_key;

  visible = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  action_name = gtk_action_get_name (action);

  if (g_ascii_strcasecmp (action_name, "ViewToolbar") == 0)
    {
      target_widget = self->priv->toolbar;
      target_conf_key = EMERILLON_CONF_UI_TOOLBAR;
    }
  else if (g_ascii_strcasecmp (action_name, "ViewStatusbar") == 0)
    {
      target_widget = self->priv->statusbar;
      target_conf_key = EMERILLON_CONF_UI_STATUSBAR;
    }
  else if (g_ascii_strcasecmp (action_name, "ViewSidebar") == 0)
    {
      target_widget = self->priv->sidebar;
      target_conf_key = EMERILLON_CONF_UI_SIDEBAR;
    }
  else
    {
      g_warning ("Unknown action '%s'\n", action_name);
      return;
    }

  if (visible)
      gtk_widget_show (target_widget);
  else
      gtk_widget_hide (target_widget);

  g_settings_set_boolean (self->priv->settings_ui, target_conf_key, visible);
}

static void
cmd_toggle_fullscreen (GtkAction *action,
              EmerillonWindow *self)
{
  gboolean fullscreen;
  GtkWidget *window;

  fullscreen = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
  window = emerillon_window_dup_default ();

  if (fullscreen)
    {
      gtk_window_fullscreen (GTK_WINDOW (window));
    }
  else
    {
      gtk_window_unfullscreen (GTK_WINDOW (window));
    }

  g_object_unref (window);
}

static void
cmd_zoom_in (GtkAction *action,
             EmerillonWindow *self)
{
  champlain_view_zoom_in (self->priv->view);
}

static void
cmd_zoom_out (GtkAction *action,
              EmerillonWindow *self)
{
  champlain_view_zoom_out (self->priv->view);
}

static void
cmd_preferences (GtkAction *action,
                 EmerillonWindow *self)
{
  GtkWidget *dialog;

  dialog = emerillon_preferences_dup_default ();

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (self));

  g_signal_connect_swapped (dialog,
      "response",
      G_CALLBACK (gtk_widget_destroy),
      dialog);

  gtk_widget_show_all (dialog);
}

static void
menu_item_select_cb (GtkMenuItem *proxy,
                     EmerillonWindow *self)
{
  GtkAction *action;
  char *tooltip;

  action = g_object_get_data (G_OBJECT (proxy), "gtk-action");
  g_return_if_fail (action);

  g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);
  if (tooltip)
    {
      gtk_statusbar_push (GTK_STATUSBAR (self->priv->statusbar),
          self->priv->tooltip_message_context_id, tooltip);
      g_free (tooltip);
    }
}

static void
menu_item_deselect_cb (GtkMenuItem *proxy,
                       EmerillonWindow *self)
{
  gtk_statusbar_pop (GTK_STATUSBAR (self->priv->statusbar),
      self->priv->tooltip_message_context_id);
}

static void
connect_proxy_cb (GtkUIManager *manager,
                  GtkAction *action,
                  GtkWidget *proxy,
                  EmerillonWindow *self)
{
  if (GTK_IS_MENU_ITEM (proxy))
    {
      g_signal_connect (proxy, "select",
          G_CALLBACK (menu_item_select_cb), self);
      g_signal_connect (proxy, "deselect",
          G_CALLBACK (menu_item_deselect_cb), self);
    }
}

static void
disconnect_proxy_cb (GtkUIManager *manager,
                     GtkAction *action,
                     GtkWidget *proxy,
                     EmerillonWindow *self)
{
  if (GTK_IS_MENU_ITEM (proxy))
    {
      g_signal_handlers_disconnect_by_func (proxy,
          G_CALLBACK (menu_item_select_cb), self);
      g_signal_handlers_disconnect_by_func (proxy,
          G_CALLBACK (menu_item_deselect_cb), self);
  }
}

static void
sidebar_visibility_changed_cb (GtkWidget *widget,
                               EmerillonWindow *self)
{
  GtkAction *action;
  gboolean visible;

  visible = gtk_widget_get_visible (self->priv->sidebar);

  g_settings_set_boolean (self->priv->settings_ui, EMERILLON_CONF_UI_SIDEBAR,
      visible);

  action = gtk_action_group_get_action (self->priv->main_actions,
      "ViewSidebar");
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);
}

static const GtkActionEntry action_entries[] = {
      { "Map",   NULL, N_("_Map") },
      { "Edit",  NULL, N_("_Edit") },
      { "View",  NULL, N_("_View") },
      { "Tools", NULL, N_("_Tools") },
      { "Help",  NULL, N_("_Help") },

      { "MapQuit", GTK_STOCK_QUIT, NULL, NULL,
        N_("Quit the program"),
        G_CALLBACK (cmd_quit) },
      { "ViewZoomIn", GTK_STOCK_ZOOM_IN, NULL, "<control>plus",
        N_("Enlarge the image"),
        G_CALLBACK (cmd_zoom_in) },
      { "ViewZoomOut", GTK_STOCK_ZOOM_OUT, NULL, "<control>minus",
        N_("Shrink the image"),
        G_CALLBACK (cmd_zoom_out) },
      { "EditPreferences", GTK_STOCK_PREFERENCES, NULL, "<control>p",
        N_("Edit the preferences"),
        G_CALLBACK (cmd_preferences) },
      { "HelpManual", GTK_STOCK_HELP, N_("_Contents"), "F1",
        N_("Help on this application"),
        G_CALLBACK (cmd_help) },
      { "HelpAbout", GTK_STOCK_ABOUT, NULL, NULL,
        N_("About this application"),
        G_CALLBACK (cmd_about) },
};

static const GtkToggleActionEntry toggle_entries[] = {
      { "ViewToolbar", NULL, N_("_Toolbar"), NULL,
        N_("Show or hide the toolbar in the current window"),
        G_CALLBACK (cmd_show_hide_bar), TRUE },
      { "ViewStatusbar", NULL, N_("_Statusbar"), NULL,
        N_("Show or hide the statusbar in the current window"),
        G_CALLBACK (cmd_show_hide_bar), TRUE },
      { "ViewSidebar", NULL, N_("Side _Pane"), "F9",
        N_("Show or hide the side pane in the current window"),
        G_CALLBACK (cmd_show_hide_bar), TRUE },
      { "ViewFullscreen", GTK_STOCK_FULLSCREEN, NULL, "F11",
        N_("Enable or disable full screen mode"),
        G_CALLBACK (cmd_toggle_fullscreen), FALSE },
};

static const GtkRadioActionEntry radio_entries[] = {
      { "MapMapnik", NULL, N_("_Street"), "<alt>1",
        N_("View the street map based on OpenStreetMap"), 0 },
      { "MapCycle", NULL, N_("_Cycling"), "<alt>2",
        N_("View the cycling map based on OpenCycleMap"), 1 },
      { "MapTransport", NULL, N_("_Public Transportation"), "<alt>3",
        N_("View the public transportation map based on Öpnvkarte"), 2 },
      { "MapTerrain", NULL, N_("_Terrain"), "<alt>4",
        N_("View the terrain map based on mapsforfree.com Relief"), 3 },
};

static void
build_ui (EmerillonWindow *self)
{
  GtkAction *action;
  GtkWidget *vbox;
  GtkWidget *menubar;
  GtkToolItem *throbber;
  GtkWidget *viewport;
  GtkWidget *hpaned;
  GtkWidget *embed_view;
  ClutterActor *scale;
  GError *error = NULL;

  /* Action entries. */
  self->priv->main_actions = gtk_action_group_new ("MenuActionsWindow");
  gtk_action_group_set_translation_domain (self->priv->main_actions,
      GETTEXT_PACKAGE);

  gtk_action_group_add_actions (self->priv->main_actions, action_entries,
      G_N_ELEMENTS (action_entries), self);

  /* Toggle entries. */
  gtk_action_group_add_toggle_actions (self->priv->main_actions,
      toggle_entries, G_N_ELEMENTS (toggle_entries), self);

  /* Radio entries. */
  gtk_action_group_add_radio_actions (self->priv->main_actions,
      radio_entries, G_N_ELEMENTS (radio_entries), 0,
      G_CALLBACK (cmd_map_change_map), self);

  /* Short labels. */
  action = gtk_action_group_get_action (self->priv->main_actions,
      "ViewZoomIn");
  g_object_set (action, "short_label", _("In"), NULL);

  action = gtk_action_group_get_action (self->priv->main_actions,
      "ViewZoomOut");
  g_object_set (action, "short_label", _("Out"), NULL);

  /* UI manager. */
  self->priv->ui_manager = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (self->priv->ui_manager,
      self->priv->main_actions, 0);

  if (!gtk_ui_manager_add_ui_from_file (self->priv->ui_manager,
        EMERILLON_DATADIR "/emerillon-ui.xml", &error))
    {
      g_warning ("building menus failed: %s", error->message);
      g_error_free (error);
      return;
    }

  g_signal_connect (self->priv->ui_manager, "connect_proxy",
      G_CALLBACK (connect_proxy_cb), self);
  g_signal_connect (self->priv->ui_manager, "disconnect_proxy",
      G_CALLBACK (disconnect_proxy_cb), self);

  gtk_window_add_accel_group (GTK_WINDOW (self),
      gtk_ui_manager_get_accel_group (self->priv->ui_manager));

  /* Main box. */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (self), vbox);
  gtk_widget_show (vbox);

  /* Menu. */
  menubar = gtk_ui_manager_get_widget (self->priv->ui_manager, "/MainMenu");
  g_assert (GTK_IS_WIDGET (menubar));
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
  gtk_widget_show (menubar);

  /* Toolbar. */
  self->priv->toolbar = gtk_ui_manager_get_widget (self->priv->ui_manager,
      "/Toolbar");

  gtk_style_context_add_class (gtk_widget_get_style_context (self->priv->toolbar),
      "primary-toolbar");

  self->priv->throbber = gtk_spinner_new ();

  throbber = gtk_tool_item_new ();
  gtk_container_add (GTK_CONTAINER (throbber), self->priv->throbber);
  gtk_widget_show (GTK_WIDGET (self->priv->throbber));
  gtk_widget_show (GTK_WIDGET (throbber));
  gtk_toolbar_insert (GTK_TOOLBAR (self->priv->toolbar), throbber,
      -1);

  gtk_box_pack_start (GTK_BOX (vbox), self->priv->toolbar,
      FALSE, FALSE, 0);
  gtk_widget_show (self->priv->toolbar);

  /* Statusbar. */
  self->priv->statusbar = gtk_statusbar_new ();
  gtk_box_pack_end (GTK_BOX (vbox),
      GTK_WIDGET (self->priv->statusbar), FALSE, FALSE, 0);
  gtk_widget_show (self->priv->statusbar);

  self->priv->tooltip_message_context_id = gtk_statusbar_get_context_id (
      GTK_STATUSBAR (self->priv->statusbar), "tooltip-message");

  /* Viewport. */
  viewport = gtk_frame_new (NULL);

  /* Map. */

  embed_view = gtk_champlain_embed_new ();
  gtk_container_add (GTK_CONTAINER (viewport), embed_view);
  /* FIXME: workaround for a champlain-gtk bug, replace with _show(). */
  gtk_widget_show_all (embed_view);

  self->priv->view = gtk_champlain_embed_get_view (GTK_CHAMPLAIN_EMBED (embed_view));
  g_signal_connect (self->priv->view, "notify::zoom-level",
      G_CALLBACK (zoom_changed_cb), self);
  g_signal_connect (self->priv->view, "notify::map-source",
      G_CALLBACK (zoom_changed_cb), self);
  g_signal_connect (self->priv->view, "notify::state",
      G_CALLBACK (state_changed_cb), self);
  g_object_set (self->priv->view, "zoom-level", 1,
      "kinetic-mode", TRUE,
      NULL);
  champlain_view_center_on (self->priv->view, 40, 0);

  scale = champlain_scale_new ();
  champlain_scale_connect_view (CHAMPLAIN_SCALE (scale), self->priv->view);

  /* align to the bottom left */
  champlain_view_bin_layout_add (self->priv->view, scale,
      CLUTTER_BIN_ALIGNMENT_START, CLUTTER_BIN_ALIGNMENT_END);

  /* Sidebar. */
  self->priv->sidebar = emerillon_sidebar_new ();
  gtk_widget_set_size_request (self->priv->sidebar, 200, -1);

  /* Horizontal pane. */
  hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_pack1 (GTK_PANED (hpaned), self->priv->sidebar, FALSE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), viewport, TRUE, FALSE);
  gtk_widget_show (self->priv->sidebar);
  gtk_widget_show (viewport);

  g_signal_connect_after (self->priv->sidebar, "show",
      G_CALLBACK (sidebar_visibility_changed_cb), self);
  g_signal_connect_after (self->priv->sidebar, "hide",
      G_CALLBACK (sidebar_visibility_changed_cb), self);

  gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
  gtk_widget_show (hpaned);

  update_ui_visibility (self);
}

/**
 * emerillon_window_get_map_view:
 * @window: (in): An #EmerillonWindow instance
 *
 * Retrieves Emerillon's map view
 *
 * Return value: (transfer none): A #ChamplainView
 */
ChamplainView *
emerillon_window_get_map_view  (EmerillonWindow *window)
{
  g_return_val_if_fail (EMERILLON_IS_WINDOW (window), NULL);

  return window->priv->view;
}

/**
 * emerillon_window_get_ui_manager:
 * @window: (in): An #EmerillonWindow instance
 *
 * Retrieves Emerillon's UI manager
 *
 * Return value: (transfer none): A #GtkUIManager
 */
GtkUIManager *
emerillon_window_get_ui_manager (EmerillonWindow *window)
{
  g_return_val_if_fail (EMERILLON_IS_WINDOW (window), NULL);

  return window->priv->ui_manager;
}

/**
 * emerillon_window_get_toolbar:
 * @window: (in): An #EmerillonWindow instance
 *
 * Retrieves Emerillon's toolbar
 *
 * Return value: (transfer none): A #GtkWidget containing Emerillon's toolbar
 */
GtkWidget *
emerillon_window_get_toolbar (EmerillonWindow *window)
{
  g_return_val_if_fail (EMERILLON_IS_WINDOW (window), NULL);

  return window->priv->toolbar;
}

/**
 * emerillon_window_get_sidebar:
 * @window: (in): An #EmerillonWindow instance
 *
 * Retrieves Emerillon's sidebar
 *
 * Return value: (transfer none): A #GtkWidget containing the current sidebar
 */
GtkWidget *
emerillon_window_get_sidebar (EmerillonWindow *window)
{
  g_return_val_if_fail (EMERILLON_IS_WINDOW (window), NULL);

  return window->priv->sidebar;
}

/**
 * emerillon_window_get_statusbar:
 * @window: (in): An #EmerillonWindow instance
 *
 * Retrieves Emerillon's statusbar
 *
 * Return value: (transfer none): A #GtkWidget containing the current statusbar
 */
GtkWidget *
emerillon_window_get_statusbar (EmerillonWindow *window)
{
  g_return_val_if_fail (EMERILLON_IS_WINDOW (window), NULL);

  return window->priv->statusbar;
}
