/*
 * Copyright (C) 2008 Marco Barisione <marco@barisione.org>
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

#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <gconf/gconf-client.h>
#include <geoclue/geoclue-master.h>
#include <geoclue/geoclue-position.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "config-keys.h"
#include "sidebar.h"

#define EMERILLION_WINDOW_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), \
        EMERILLION_TYPE_WINDOW, \
        EmerillionWindowPrivate))

G_DEFINE_TYPE (EmerillionWindow, emerillion_window, GTK_TYPE_WINDOW);

struct _EmerillionWindowPrivate
{
  GtkUIManager *ui_manager;

  GtkWidget *toolbar;
  GtkWidget *statusbar;
  GtkWidget *sidebar;

  ChamplainView *view;

  GtkActionGroup *main_actions;

  GConfClient *client;

  guint tooltip_messase_context_id;
};

static void     build_ui        (EmerillionWindow *self);

static void
position_changed_cb (GeocluePosition *position,
                     GeocluePositionFields fields,
                     int timestamp,
                     double latitude,
                     double longitude,
                     double altitude,
                     GeoclueAccuracy *accuracy,
                     GError *error,
                     EmerillionWindow *self)
{
  if (error)
    {
      g_printerr ("Error retrieving the current position: %s\n", error->message);
      g_error_free (error);
      g_object_unref (position);
      return;
    }
  else if (fields & GEOCLUE_POSITION_FIELDS_LATITUDE &&
           fields & GEOCLUE_POSITION_FIELDS_LONGITUDE)
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
            zoom_level = -1;
        }

      if (zoom_level >= 0)
        {
          champlain_view_center_on (self->priv->view, latitude, longitude);
          g_object_set (self->priv->view, "zoom-level", zoom_level, NULL);
        }
    }

  g_object_unref (g_object_get_data (G_OBJECT (position), "client"));
  g_object_unref (position);
}

static void
emerillion_window_init (EmerillionWindow *self)
{
  GdkGeometry geometry;
  GeoclueMaster *master;
  GeoclueMasterClient *client;
  GeocluePosition *position;

  self->priv = EMERILLION_WINDOW_GET_PRIVATE (self);

  /* GConf. */
  self->priv->client = gconf_client_get_default ();

  gconf_client_add_dir (self->priv->client, EMERILLION_CONF_DIR,
      GCONF_CLIENT_PRELOAD_RECURSIVE, NULL);

  /* Window setup. */
  geometry.min_width = 400;
  geometry.min_height = 350;
  gtk_window_set_geometry_hints (GTK_WINDOW (self), GTK_WIDGET (self),
      &geometry,GDK_HINT_MIN_SIZE);

  gtk_window_set_default_size (GTK_WINDOW (self), 640, 450);

  /* Current position. */
  master = geoclue_master_get_default ();
  client = geoclue_master_create_client (master, NULL, NULL);
  g_object_unref (master);

  geoclue_master_client_set_requirements (client,
      GEOCLUE_ACCURACY_LEVEL_COUNTRY, 0, FALSE, GEOCLUE_RESOURCE_ALL, NULL);
  position = geoclue_master_client_create_position (client, NULL);
  if (position)
    {
      g_object_set_data (G_OBJECT (position), "client", client);
      geoclue_position_get_position_async (position,
          (GeocluePositionCallback)position_changed_cb, self);
    }
  else
    {
      g_object_unref (client);
      g_object_unref (position);
    }
}

static void
emerillion_window_dispose (GObject *object)
{
  EmerillionWindow *self = EMERILLION_WINDOW (object);

  if (self->priv->main_actions != NULL)
    {
      g_object_unref (self->priv->main_actions);
      self->priv->main_actions = NULL;
    }

  if (self->priv->client)
    {
      gconf_client_remove_dir (self->priv->client, EMERILLION_CONF_DIR, NULL);
      g_object_unref (self->priv->client);
      self->priv->client = NULL;
    }

  G_OBJECT_CLASS (emerillion_window_parent_class)->dispose (object);
}

static void
emerillion_window_finalize (GObject *object)
{
  G_OBJECT_CLASS (emerillion_window_parent_class)->finalize (object);
}

static GObject *
emerillion_window_constructor (GType type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (emerillion_window_parent_class)->constructor (
      type, n_construct_properties, construct_params);

  build_ui (EMERILLION_WINDOW (object));

  return object;
}

static void
emerillion_window_class_init (EmerillionWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = emerillion_window_constructor;
  object_class->dispose = emerillion_window_dispose;
  object_class->finalize = emerillion_window_finalize;

  g_type_class_add_private (object_class, sizeof (EmerillionWindowPrivate));
}

GtkWidget *
emerillion_window_new (void)
{
  return g_object_new (EMERILLION_TYPE_WINDOW, 
      "type", GTK_WINDOW_TOPLEVEL,
      NULL);
}

static void
zoom_changed_cb (GtkWidget *widget,
                 GParamSpec *pspec,
                 EmerillionWindow *self)
{
  GtkAction *zoom_in_action;
  GtkAction *zoom_out_action;
  gint zoom_level;

  zoom_in_action = gtk_action_group_get_action (self->priv->main_actions,
        "ViewZoomIn");
  zoom_out_action = gtk_action_group_get_action (self->priv->main_actions,
        "ViewZoomOut");

  g_object_get (self->priv->view, "zoom-level", &zoom_level, NULL);

  gtk_action_set_sensitive (zoom_in_action, zoom_level < 17);
  gtk_action_set_sensitive (zoom_out_action, zoom_level > 0);
}

static void
update_ui_visibility (EmerillionWindow *self)
{
  gboolean visible;
  GtkAction *action;

  /* Toolbar. */
  visible = gconf_client_get_bool (self->priv->client,
      EMERILLION_CONF_UI_TOOLBAR, NULL);
  action = gtk_ui_manager_get_action (self->priv->ui_manager,
      "/MainMenu/View/ToolbarToggle");
  g_assert (action != NULL);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);
  g_object_set (G_OBJECT (self->priv->toolbar), "visible", visible, NULL);

  /* Statusbar. */
  visible = gconf_client_get_bool (self->priv->client,
      EMERILLION_CONF_UI_STATUSBAR, NULL);
  action = gtk_ui_manager_get_action (self->priv->ui_manager,
      "/MainMenu/View/StatusbarToggle");
  g_assert (action != NULL);
  gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);
  g_object_set (G_OBJECT (self->priv->statusbar), "visible", visible, NULL);

  /* Sidebar. */
  visible = gconf_client_get_bool (self->priv->client,
      EMERILLION_CONF_UI_SIDEBAR, NULL);
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
          EmerillionWindow *self)
{
  gtk_widget_destroy (GTK_WIDGET (self));
  gtk_main_quit ();
}

static void
cmd_help (GtkAction *action,
          EmerillionWindow *self)
{
  g_printerr ("Sorry, help not available\n");
}

static void
cmd_about (GtkAction *action,
           EmerillionWindow *self)
{
  static const char *authors[] = {
      "Marco Barisione <marco@barisione.org>",
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
      "program-name", _("Emerillion"),
      "version", VERSION,
      "copyright", "Copyright \xc2\xa9 2008 Marco Barisione",
      "comments",_("A map viewer for the GNOME desktop"),
      "authors", authors,
      "documenters", documenters,
      "translator-credits", translators,
      //"website", "http://www.example.com/",
      //"logo-icon-name", "emerillion",
      "license", license_translated,
      "wrap-license", TRUE,
      NULL);

  g_free (license_translated);
}

static void
cmd_show_hide_bar (GtkAction *action,
                   EmerillionWindow *self)
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
      target_conf_key = EMERILLION_CONF_UI_TOOLBAR;
    }
  else if (g_ascii_strcasecmp (action_name, "ViewStatusbar") == 0)
    {
      target_widget = self->priv->statusbar;
      target_conf_key = EMERILLION_CONF_UI_STATUSBAR;
    }
  else if (g_ascii_strcasecmp (action_name, "ViewSidebar") == 0)
    {
      target_widget = self->priv->sidebar;
      target_conf_key = EMERILLION_CONF_UI_SIDEBAR;
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

  gconf_client_set_bool (self->priv->client, target_conf_key, visible, NULL);
}

static void
cmd_zoom_in (GtkAction *action,
             EmerillionWindow *self)
{
  champlain_view_zoom_in (self->priv->view);
}

static void
cmd_zoom_out (GtkAction *action,
              EmerillionWindow *self)
{
  champlain_view_zoom_out (self->priv->view);
}

static void
menu_item_select_cb (GtkMenuItem *proxy,
                     EmerillionWindow *self)
{
  GtkAction *action;
  char *tooltip;

  action = g_object_get_data (G_OBJECT (proxy), "gtk-action");
  g_return_if_fail (action);

  g_object_get (G_OBJECT (action), "tooltip", &tooltip, NULL);
  if (tooltip)
    {
      gtk_statusbar_push (GTK_STATUSBAR (self->priv->statusbar),
          self->priv->tooltip_messase_context_id, tooltip);
      g_free (tooltip);
    }
}

static void
menu_item_deselect_cb (GtkMenuItem *proxy,
                       EmerillionWindow *self)
{
  gtk_statusbar_pop (GTK_STATUSBAR (self->priv->statusbar),
      self->priv->tooltip_messase_context_id);
}

static void
connect_proxy_cb (GtkUIManager *manager,
                  GtkAction *action,
                  GtkWidget *proxy,
                  EmerillionWindow *self)
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
                     EmerillionWindow *self)
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
                               EmerillionWindow *self)
{
  GtkAction *action;
  gboolean visible;

  visible = GTK_WIDGET_VISIBLE (self->priv->sidebar);

  gconf_client_set_bool (self->priv->client, EMERILLION_CONF_UI_SIDEBAR,
      visible, NULL);

  action = gtk_action_group_get_action (self->priv->main_actions,
      "ViewSidebar");
  if (gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action)) != visible)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), visible);
}

static const GtkActionEntry action_entries[] = {
      { "File",  NULL, N_("_File") },
      { "Edit",  NULL, N_("_Edit") },
      { "View",  NULL, N_("_View") },
      { "Tools", NULL, N_("_Tools") },
      { "Help",  NULL, N_("_Help") },

      { "FileQuit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
        N_("Quit the program"),
        G_CALLBACK (cmd_quit) },
      { "ViewZoomIn", GTK_STOCK_ZOOM_IN, N_("_Zoom In"), "<control>plus", 
        N_("Enlarge the image"), 
        G_CALLBACK (cmd_zoom_in) },
      { "ViewZoomOut", GTK_STOCK_ZOOM_OUT, N_("Zoom _Out"), "<control>minus", 
        N_("Shrink the image"), 
        G_CALLBACK (cmd_zoom_out) },
      { "HelpManual", GTK_STOCK_HELP, N_("_Contents"), "F1", 
        N_("Help on this application"),
        G_CALLBACK (cmd_help) },
      { "HelpAbout", GTK_STOCK_ABOUT, N_("_About"), NULL, 
        N_("About this application"),
        G_CALLBACK (cmd_about) }
};

static const GtkToggleActionEntry toggle_entries[] = {
      { "ViewToolbar", NULL, N_("_Toolbar"), NULL, 
        N_("Show or hide the toolbar in the current self"),
        G_CALLBACK (cmd_show_hide_bar), TRUE },
      { "ViewStatusbar", NULL, N_("_Statusbar"), NULL, 
        N_("Show or hide the statusbar in the current self"), 
        G_CALLBACK (cmd_show_hide_bar), TRUE },
      { "ViewSidebar", NULL, N_("Side _Pane"), "F9",
        N_("Show or hide the side pane in the current self"), 
        G_CALLBACK (cmd_show_hide_bar), TRUE },
};

static void 
build_ui (EmerillionWindow *self)
{
  GtkAction *action;
  GtkWidget *vbox;
  GtkWidget *menubar;
  GtkWidget *viewport;
  GtkWidget *hpaned;
  GtkWidget *sidebar_content;
  GtkWidget *embed_view;
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
        EMERILLION_DATADIR "/emerillion-ui.xml", &error))
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
  vbox = gtk_vbox_new (FALSE, 0);
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
  gtk_box_pack_start (GTK_BOX (vbox), self->priv->toolbar,
      FALSE, FALSE, 0);
  gtk_widget_show (self->priv->toolbar);

  /* Statusbar. */
  self->priv->statusbar = gtk_statusbar_new ();
  gtk_box_pack_end (GTK_BOX (vbox),
      GTK_WIDGET (self->priv->statusbar), FALSE, FALSE, 0);
  gtk_widget_show (self->priv->statusbar);

  self->priv->tooltip_messase_context_id = gtk_statusbar_get_context_id (
      GTK_STATUSBAR (self->priv->statusbar), "tooltip-message");

  /* Viewport. */
  viewport = gtk_viewport_new (NULL, NULL);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewport), GTK_SHADOW_IN);

  /* Map. */
  self->priv->view = CHAMPLAIN_VIEW (
      champlain_view_new (CHAMPLAIN_VIEW_MODE_KINETIC));
  g_signal_connect (self->priv->view, "notify::zoom-level",
      G_CALLBACK (zoom_changed_cb), self);
  g_object_set (self->priv->view, "zoom-level", 1, NULL);
  champlain_view_center_on (self->priv->view, 40, 0);

  embed_view = champlain_view_embed_new (self->priv->view);
  gtk_container_add (GTK_CONTAINER (viewport), embed_view);
  /* FIXME: workaround for a champlain-gtk bug, replace with _show(). */
  gtk_widget_show_all (embed_view);

  /* Sidebar. */
  self->priv->sidebar = emerillion_sidebar_new ();
  gtk_widget_set_size_request (self->priv->sidebar, 200, -1);

  g_signal_connect_after (self->priv->sidebar, "show",
      G_CALLBACK (sidebar_visibility_changed_cb), self);
  g_signal_connect_after (self->priv->sidebar, "hide",
      G_CALLBACK (sidebar_visibility_changed_cb), self);

  sidebar_content = gtk_label_new ("Sidebar test");
  emerillion_sidebar_add_page (EMERILLION_SIDEBAR (self->priv->sidebar),
      "Test page", sidebar_content);
  gtk_widget_show (sidebar_content);

  /* Horizontal pane. */
  hpaned = gtk_hpaned_new ();
  gtk_paned_pack1 (GTK_PANED (hpaned), self->priv->sidebar, FALSE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), viewport, TRUE, FALSE);
  gtk_widget_show (self->priv->sidebar);
  gtk_widget_show (viewport);

  gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
  gtk_widget_show (hpaned);

  update_ui_visibility (self);
}

