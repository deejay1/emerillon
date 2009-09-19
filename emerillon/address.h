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

#ifndef __EMERILLON_ADDRESS_H__
#define __EMERILLON_ADDRESS_H__

#include <glib.h>
#include <geoclue/geoclue-types.h>
#include <geoclue/geoclue-accuracy.h>

G_BEGIN_DECLS 

typedef void (*EmerillonAddressFunc) (gdouble latitude,
                                       gdouble longitude,
                                       GHashTable *details,
                                       GeoclueAccuracy *accuracy,
                                       gpointer userdata);

void   emerillon_address_get    (const gchar *text,
                                  EmerillonAddressFunc callback,
                                  gpointer userdata);

G_END_DECLS

#endif
