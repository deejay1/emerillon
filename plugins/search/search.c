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

enum {
  COL_ORDER,
  COL_SYMBOL,
  COL_NAME,
  COL_DISPLAY_NAME,
  COL_MARKER,
  COL_LAT,
  COL_LON,
  COL_COUNT
};

struct _SearchPluginPrivate
{
  GtkWidget *search_entry;
  GtkWidget *search_page;
  GtkWidget *treeview;
  GtkTreeModel *model;
  GtkToolItem *search_item;

  RestProxy *proxy;
  RestProxyCall *call;

  ChamplainView *map_view;
  ChamplainLayer *layer;
};

static void
result_cb (RestProxyCall *call,
           GError *error,
           GObject *weak_object,
           SearchPlugin *plugin)
{
  const gchar *answer;
  gint len;
  gint count = 0;
  guint i;
  RestXmlParser *parser;
  RestXmlNode *root, *n;
  gfloat min_lat, max_lat, min_lon, max_lon;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  answer = rest_proxy_call_get_payload (call);
  len = rest_proxy_call_get_payload_length (call);
  parser = rest_xml_parser_new ();

  root = rest_xml_parser_parse_from_data (parser, answer, len);

  /* Extract the result count */
  n = rest_xml_node_find (root, "totalResultsCount");
  if (n)
    count = g_strtod (n->content, NULL);

  if (count == 0)
    {
      GtkTreeIter iter;

      gtk_list_store_append (GTK_LIST_STORE (priv->model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (priv->model), &iter,
                          COL_ORDER, 0,
                          COL_SYMBOL, "",
                          COL_NAME, _("No result found"),
                          COL_DISPLAY_NAME, _("No result found"),
                          COL_MARKER, NULL,
                          -1);

      rest_xml_node_unref (root);
      return;
    }

  n = rest_xml_node_find (root, "geoname");
  i = 1;

  min_lat = CHAMPLAIN_MAX_LAT;
  max_lat = CHAMPLAIN_MIN_LAT;
  min_lon = CHAMPLAIN_MAX_LONG;
  max_lon = CHAMPLAIN_MIN_LONG;

  while (n)
    {
      RestXmlNode *name, *country, *lon, *lat;
      GtkTreeIter iter;
      ChamplainMarker *marker;
      gchar *symbol, *display_name, *escaped_name;
      gfloat flon, flat;

      name = rest_xml_node_find (n, "name");
      if (!name)
        {
          n = n->next;
          continue;
        }

      country = rest_xml_node_find (n, "countryName");
      if (!country)
        {
          n = n->next;
          continue;
        }

      lon = rest_xml_node_find (n, "lng");
      if (!lon)
        {
          n = n->next;
          continue;
        }

      lat = rest_xml_node_find (n, "lat");
      if (!lon)
        {
          n = n->next;
          continue;
        }

      symbol = g_strdup_printf ("%d", i);
      escaped_name = g_markup_escape_text (name->content, -1);
      if (country->content)
        display_name = g_strdup_printf ("%s\n<small>%s</small>", escaped_name, country->content);
      else
        display_name = g_strdup_printf ("%s\n", escaped_name);

      flon = g_strtod (lon->content, NULL);
      flat = g_strtod (lat->content, NULL);
      if (flat > max_lat)
        max_lat = flat;
      if (flat < min_lat)
        min_lat = flat;
      if (flon > max_lon)
        max_lon = flon;
      if (flon < min_lon)
        min_lon = flon;

      /* Create the marker */
      marker = CHAMPLAIN_MARKER (champlain_marker_new ());
      champlain_marker_set_text (marker, symbol);
      champlain_base_marker_set_position (CHAMPLAIN_BASE_MARKER (marker),
          flat,
          flon);
      clutter_container_add_actor (CLUTTER_CONTAINER (priv->layer),
          CLUTTER_ACTOR (marker));
      clutter_actor_show (CLUTTER_ACTOR (marker));

      /* Create the row item */
      gtk_list_store_append (GTK_LIST_STORE (priv->model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (priv->model), &iter,
                          COL_ORDER, i,
                          COL_SYMBOL, symbol,
                          COL_NAME, name->content,
                          COL_DISPLAY_NAME, display_name,
                          COL_MARKER, marker,
                          COL_LAT, flat,
                          COL_LON, flon,
                          -1);

      g_free (symbol);
      g_free (display_name);

      n = n->next;
      i++;
    }

  champlain_view_ensure_visible (priv->map_view,
      min_lat, min_lon,
      max_lat, max_lon,
      FALSE);

  rest_xml_node_unref (root);
}

static void
search_address (SearchPlugin *plugin)
{
  const gchar *query;
  gchar *locale;
  gchar lang[2];
  GError *error = NULL;
  GList *children, *l;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  query = gtk_entry_get_text (GTK_ENTRY (plugin->priv->search_entry));
  locale = setlocale (LC_MESSAGES, NULL);
  g_utf8_strncpy (lang, locale, 2);

  gtk_list_store_clear (GTK_LIST_STORE (priv->model));

  /* Remove markers */
  children = clutter_container_get_children (CLUTTER_CONTAINER (priv->layer));
  for (l = children; l != NULL; l = l->next)
    {
      champlain_layer_remove_marker (priv->layer, CHAMPLAIN_BASE_MARKER (l->data));
    }
  g_list_free (children);

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
row_selected_cb (GtkTreeSelection *selection,
                 SearchPlugin *plugin)
{
  GtkTreeIter iter;
  GValue value = {0};
  ChamplainBaseMarker *marker;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  if (!gtk_tree_selection_get_selected (selection, &priv->model, &iter))
    return;

  gtk_tree_model_get_value (priv->model, &iter, COL_MARKER, &value);
  marker = g_value_get_object (&value);
  g_value_unset (&value);

  if (!marker)
    return;

  champlain_selection_layer_select (CHAMPLAIN_SELECTION_LAYER (priv->layer),
        marker);
}

static void
row_activated_cb (GtkTreeView *tree_view,
                  GtkTreePath *path,
                  GtkTreeViewColumn *column,
                  SearchPlugin *plugin)
{
  GtkTreeIter iter;
  GValue value = {0};
  gfloat lat, lon;
  ChamplainMarker *marker;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  if (!gtk_tree_model_get_iter (priv->model, &iter, path))
    return;

  gtk_tree_model_get_value (priv->model, &iter, COL_MARKER, &value);
  marker = g_value_get_object (&value);
  g_value_unset (&value);

  if (!marker)
    return;

  gtk_tree_model_get_value (priv->model, &iter, COL_LAT, &value);
  lat = g_value_get_float (&value);
  g_value_unset (&value);

  gtk_tree_model_get_value (priv->model, &iter, COL_LON, &value);
  lon = g_value_get_float (&value);
  g_value_unset (&value);

  if (champlain_view_get_zoom_level (priv->map_view) < 12)
    champlain_view_set_zoom_level (priv->map_view, 12);

  champlain_view_center_on (priv->map_view, lat, lon);
}

static gboolean
select_function_cb (GtkTreeSelection *selection,
                    GtkTreeModel *model,
                    GtkTreePath *path,
                    gboolean path_currently_selected,
                    SearchPlugin *plugin)
{
  GtkTreeIter iter;
  GValue value = {0};
  ChamplainBaseMarker *marker;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  if (path_currently_selected)
    return TRUE;

  if (!gtk_tree_model_get_iter (priv->model, &iter, path))
    return FALSE;

  gtk_tree_model_get_value (priv->model, &iter, COL_MARKER, &value);
  marker = g_value_get_object (&value);
  g_value_unset (&value);

  return marker != NULL;
}

static void
activated (EthosPlugin *plugin)
{
  GtkWidget *window, *toolbar, *sidebar, *scrolled;
  gint count = 0;
  GtkListStore *store;
  GtkCellRenderer *cell;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;
  SearchPluginPrivate *priv = SEARCH_PLUGIN (plugin)->priv;

  priv->proxy = NULL;
  priv->call = NULL;
  window = emerillon_window_dup_default ();
  toolbar = emerillon_window_get_toolbar (EMERILLON_WINDOW (window));
  sidebar = emerillon_window_get_sidebar (EMERILLON_WINDOW (window));
  priv->map_view = emerillon_window_get_map_view (EMERILLON_WINDOW (window));

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
  /*priv->search_page = gtk_label_new (_("Type an address and press the search button."));
  gtk_misc_set_padding (GTK_MISC (priv->search_page), 10, 10);
  gtk_label_set_line_wrap (GTK_LABEL (priv->search_page), TRUE);
  gtk_label_set_single_line_mode (GTK_LABEL (priv->search_page), FALSE);
  */
  priv->search_page = gtk_vbox_new (FALSE, 10);

  /* FIXME: set this based on the sidebar size. */
  gtk_widget_set_size_request (priv->search_page, 200, -1);

  /* Setup result treeview */
  store = gtk_list_store_new (COL_COUNT,
                              G_TYPE_INT,          /* Order */
                              G_TYPE_STRING,       /* Symbol */
                              G_TYPE_STRING,       /* Name */
                              G_TYPE_STRING,       /* Display name */
                              G_TYPE_OBJECT,       /* Marker pointer */
                              G_TYPE_FLOAT,        /* Latitude */
                              G_TYPE_FLOAT);       /* Longitude */
  priv->model = GTK_TREE_MODEL (store);

  priv->treeview = gtk_tree_view_new ();
  g_signal_connect (priv->treeview, "row-activated",
      G_CALLBACK (row_activated_cb),
      plugin);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview));
  gtk_tree_selection_set_select_function (selection,
      (GtkTreeSelectionFunc) select_function_cb, plugin, NULL);
  g_signal_connect (selection, "changed",
      G_CALLBACK (row_selected_cb),
      plugin);
  gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview), priv->model);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (priv->treeview), COL_NAME);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
      GTK_POLICY_AUTOMATIC,
      GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
      GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (scrolled), priv->treeview);
  gtk_box_pack_start_defaults (GTK_BOX (priv->search_page), scrolled);
  gtk_widget_show_all (scrolled);

  cell = gtk_cell_renderer_text_new ();
  g_object_set (cell,
                "ellipsize", PANGO_ELLIPSIZE_END,
                NULL);

  column = gtk_tree_view_column_new_with_attributes (_("No"),
                                                     cell,
                                                     "text", COL_SYMBOL,
                                                     NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_ORDER);
  gtk_tree_view_column_set_expand (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);

  column = gtk_tree_view_column_new_with_attributes (_("Name"),
                                                     cell,
                                                     "markup", COL_DISPLAY_NAME,
                                                     NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_NAME);
  gtk_tree_view_column_set_expand (column, FALSE);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview), column);

  emerillon_sidebar_add_page (EMERILLON_SIDEBAR (sidebar),
      _("Search results"), priv->search_page);
  gtk_widget_show (priv->search_page);

  /* Setup result layer */
  priv->layer = champlain_selection_layer_new();
  champlain_view_add_layer (priv->map_view,
      priv->layer);
  /* FIXME: when ChamplainSelectionLayer grows a selection-changed
   * signal, connect to it */
  clutter_actor_show (CLUTTER_ACTOR (priv->layer));

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