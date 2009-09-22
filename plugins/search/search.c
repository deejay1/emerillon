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

#include "config.h"
#include "search.h"
#include "emerillon/emerillon.h"


#include <glib/gi18n.h>
#include <rest/rest-proxy.h>
#include <rest/rest-proxy-call.h>
#include <rest/rest-xml-parser.h>

G_DEFINE_TYPE (SearchPlugin, search_plugin, ETHOS_TYPE_PLUGIN)

struct _SearchPluginPrivate
{
  GtkWidget *search_entry;
  GtkWidget *search_page;
  GtkToolItem *search_item;
  RestProxy *proxy;
  RestProxyCall *call;
};

static void
result_cb (RestProxyCall *call,
           GError *error,
           GObject *weak_object,
           SearchPlugin *plugin)
{
  const gchar *answer;
  gint len;
  RestXmlParser *parser;
  RestXmlNode *root, *n, *name;

  answer = rest_proxy_call_get_payload (call);
  len = rest_proxy_call_get_payload_length (call);
  parser = rest_xml_parser_new ();

  root = rest_xml_parser_parse_from_data (parser, answer, len);
  n = rest_xml_node_find (root, "geoname");
  while (n)
    {
      name = rest_xml_node_find (n, "name");
      if (name)
        g_print ("City: %s\n", name->content);
      n = n->next;
    }

  rest_xml_node_unref (root);
}

static void
search_address (SearchPlugin *plugin)
{
  const gchar *query;
  gchar *locale;
  gchar lang[2];
  GError *error = NULL;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  query = gtk_entry_get_text (GTK_ENTRY (plugin->priv->search_entry));
  locale = setlocale (LC_MESSAGES, NULL);
  g_utf8_strncpy (lang, locale, 2);

  g_print("Searching for %s in %s\n", query, lang);

  if (priv->proxy == NULL)
    priv->proxy = rest_proxy_new ("http://ws.geonames.org/", FALSE);

  /* Cancel previous call */
  if (priv->call)
    g_object_unref (priv->call);
  priv->call = rest_proxy_new_call (priv->proxy);

  rest_proxy_set_user_agent (priv->proxy, "Emerillon/"VERSION);

  rest_proxy_call_set_function (priv->call, "search");
  rest_proxy_call_set_method (priv->call, "GET");
  rest_proxy_call_add_params (priv->call,
      "q", query,
      "maxRows", "10",
      "lang", lang,
      NULL);


  if (!rest_proxy_call_async (priv->call,
        (RestProxyCallAsyncCallback) result_cb,
        G_OBJECT (priv->proxy),
        plugin,
        &error))
    {
      g_error ("Cannot make call: %s", error->message);
      g_error_free (error);
    }
}

static void
search_activate_cb (GtkEntry *entry,
                    SearchPlugin *plugin)
{
  search_address (plugin);
}

static void
search_icon_activate_cb (GtkEntry *entry,
                         GtkEntryIconPosition position,
                         GdkEvent *event,
                         SearchPlugin *plugin)
{
  search_address (plugin);
}

static void
activated (EthosPlugin *plugin)
{
  GtkWidget *window, *toolbar, *sidebar;
  gint count = 0;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  priv->proxy = NULL;
  priv->call = NULL;
  window = emerillon_window_dup_default ();
  toolbar = emerillon_window_get_toolbar (EMERILLON_WINDOW (window));
  sidebar = emerillon_window_get_sidebar (EMERILLON_WINDOW (window));

  /* Setup toolbar */
  priv->search_entry = gtk_entry_new ();
  g_signal_connect (priv->search_entry, "activate",
      G_CALLBACK (search_activate_cb), plugin);
  gtk_entry_set_icon_from_stock (GTK_ENTRY (priv->search_entry),
      GTK_ENTRY_ICON_SECONDARY, "gtk-find");
  gtk_entry_set_icon_activatable (GTK_ENTRY (priv->search_entry),
      GTK_ENTRY_ICON_SECONDARY, TRUE);
  g_signal_connect (priv->search_entry, "icon-press",
      G_CALLBACK (search_icon_activate_cb), plugin);

  priv->search_item = gtk_tool_item_new ();
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (priv->search_item), TRUE);
  gtk_container_add (GTK_CONTAINER (priv->search_item), priv->search_entry);
  gtk_widget_show (GTK_WIDGET (priv->search_entry));
  gtk_widget_show (GTK_WIDGET (priv->search_item));

  count = gtk_toolbar_get_n_items (GTK_TOOLBAR (toolbar));
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), priv->search_item,
      count - 1);

  /* Search result sidebar page. */
  priv->search_page = gtk_label_new (_("Type an address and press the search button."));
  gtk_misc_set_padding (GTK_MISC (priv->search_page), 10, 10);
  gtk_label_set_line_wrap (GTK_LABEL (priv->search_page), TRUE);
  gtk_label_set_single_line_mode (GTK_LABEL (priv->search_page), FALSE);

  /* FIXME: set this based on the sidebar size. */
  gtk_widget_set_size_request (priv->search_page, 200, -1);

  emerillon_sidebar_add_page (EMERILLON_SIDEBAR (sidebar),
      _("Search results"), priv->search_page);
  gtk_widget_show (priv->search_page);

  g_object_unref (window);
}

static void
deactivated (EthosPlugin *plugin)
{
  GtkWidget *window, *toolbar, *sidebar;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  if (priv->proxy)
    {
      g_object_unref (priv->proxy);
      priv->proxy = NULL;
    }

  if (priv->call)
    {
      g_object_unref (priv->call);
      priv->call = NULL;
    }

  window = emerillon_window_dup_default ();
  toolbar = emerillon_window_get_toolbar (EMERILLON_WINDOW (window));
  sidebar = emerillon_window_get_sidebar (EMERILLON_WINDOW (window));

  gtk_container_remove (GTK_CONTAINER (toolbar), GTK_WIDGET (priv->search_item));
  emerillon_sidebar_remove_page (EMERILLON_SIDEBAR (sidebar), priv->search_page);
  g_object_unref (window);
}

static void
search_plugin_class_init (SearchPluginClass *klass)
{
  EthosPluginClass *plugin_class;

  g_type_class_add_private (klass, sizeof (SearchPluginPrivate));

  plugin_class = ETHOS_PLUGIN_CLASS (klass);
  plugin_class->activated = activated;
  plugin_class->deactivated = deactivated;
}

static void
search_plugin_init (SearchPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin,
                                              SEARCH_TYPE_PLUGIN,
                                              SearchPluginPrivate);
}

EthosPlugin*
search_plugin_new (void)
{
  return g_object_new (SEARCH_TYPE_PLUGIN, NULL);
}

G_MODULE_EXPORT EthosPlugin*
ethos_plugin_register (void)
{
  return search_plugin_new ();
}
