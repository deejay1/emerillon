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

#include "address.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <geoclue/gc-web-service.h>
#include <geoclue/geoclue-error.h>

#define YAHOO_GEOCLUE_APP_ID "zznSbDjV34HRU5CXQc4D3qE1DzCsJTaKvWTLhNJxbvI_JTp1hIncJ4xTSJFRgjE-"
#define YAHOO_BASE_URL "http://api.local.yahoo.com/MapsService/V1/geocode"

typedef struct
{
  gchar *text;
  EmerillonAddressFunc callback;
  gpointer userdata;
} SearchData;

typedef struct
{
  SearchData *search;
  gdouble latitude;
  gdouble longitude;
  GeoclueAccuracy *accuracy;
  GHashTable *details;
} ResultData;

static gboolean
call_callback (gpointer data)
{
  ResultData *result = data;

  result->search->callback (result->latitude, result->longitude,
      result->details, result->accuracy, result->search->userdata);

  g_hash_table_unref (result->details);
  if (result->accuracy != NULL)
    geoclue_accuracy_free (result->accuracy);
  g_free (result->search);
  g_free (result);

  return FALSE;
}

static void
async_call_callback (ResultData *result)
{
  g_idle_add (call_callback, result);
}

static void
insert_detail_string (GcWebService *web_service,
                      const gchar *xpath,
                      GHashTable *details,
                      const gchar *key)
{
  gchar *tmp;

  if (gc_web_service_get_string (web_service, &tmp, xpath))
    {
      g_strstrip (tmp);
      if (tmp[0] != '\0')
        g_hash_table_insert (details, g_strdup (key), tmp);
      else
        g_free (tmp);
    }
}

static gpointer
address_get_thread (gpointer data)
{
  static GcWebService *web_service = NULL;

  SearchData *search = data;
  ResultData *result;
  gchar *precision;
  GeoclueAccuracyLevel level;
  GError *error = NULL;

  if (!web_service)
    {
      web_service = g_object_new (GC_TYPE_WEB_SERVICE, NULL);
      gc_web_service_set_base_url (web_service, YAHOO_BASE_URL);
      gc_web_service_add_namespace (web_service, "yahoo", "urn:yahoo:maps");
    }

  result = g_new0 (ResultData, 1);
  result->search = search;
  result->details = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
      g_free);
  result->accuracy = geoclue_accuracy_new (GEOCLUE_ACCURACY_LEVEL_NONE, 0, 0);

  if (!gc_web_service_query (web_service, &error,
      "appid", YAHOO_GEOCLUE_APP_ID,
      "location", search->text,
      NULL))
    {
      async_call_callback (result);
      return NULL;
    }

  if (!gc_web_service_get_double (web_service, &result->latitude,
        "//yahoo:Latitude") ||
      !gc_web_service_get_double (web_service, &result->longitude,
        "//yahoo:Longitude"))
    {
      async_call_callback (result);
      return NULL;
    }

  level = GEOCLUE_ACCURACY_LEVEL_NONE;
  if (gc_web_service_get_string (web_service, &precision,
        "//yahoo:Result/attribute::precision"))
    {
      if ((strcmp (precision, "street") == 0) ||
          (strcmp (precision, "address") == 0))
        {
          level = GEOCLUE_ACCURACY_LEVEL_STREET;
        }
      else if ((strcmp (precision, "zip") == 0) ||
               (strcmp (precision, "city") == 0))
        {
          level = GEOCLUE_ACCURACY_LEVEL_LOCALITY;
        }
      else if ((strcmp (precision, "zip+2") == 0) ||
               (strcmp (precision, "zip+4") == 0))
        {
          level = GEOCLUE_ACCURACY_LEVEL_POSTALCODE;
        }
      else if (strcmp (precision, "state") == 0)
        {
          level = GEOCLUE_ACCURACY_LEVEL_REGION;
        }
      else if (strcmp (precision, "country") == 0)
        {
          level = GEOCLUE_ACCURACY_LEVEL_COUNTRY;
        }
      g_free (precision);
    }
  g_value_set_int (g_value_array_get_nth (result->accuracy, 0), level);

  insert_detail_string (web_service, "//yahoo:Address", result->details, "address");
  insert_detail_string (web_service, "//yahoo:City", result->details, "city");
  insert_detail_string (web_service, "//yahoo:State", result->details, "state");
  insert_detail_string (web_service, "//yahoo:Zip", result->details, "zip");
  insert_detail_string (web_service, "//yahoo:Country", result->details, "country");

  async_call_callback (result);

  return NULL;
}

void
emerillon_address_get (const gchar *text,
                        EmerillonAddressFunc callback,
                        gpointer userdata)
{
  SearchData *data;

  data = g_new0 (SearchData, 1);
  data->text = g_strdup (text);
  data->callback = callback;
  data->userdata = userdata;

  g_thread_create (address_get_thread, data, FALSE, NULL);
}

