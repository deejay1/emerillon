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

#if !defined (__EMERILLON_H_INSIDE__) && !defined (EMERILLON_COMPILATION)
#error "Only <emerillon/emerillon.h> can be included directly."
#endif

#ifndef __EMERILLON_PREFERENCES_H__
#define __EMERILLON_PREFERENCES_H__

#include <gtk/gtk.h>
#include <champlain/champlain.h>

G_BEGIN_DECLS

typedef struct _EmerillonPreferences EmerillonPreferences;
typedef struct _EmerillonPreferencesClass EmerillonPreferencesClass;
typedef struct _EmerillonPreferencesPrivate EmerillonPreferencesPrivate;

#define EMERILLON_TYPE_PREFERENCES            (emerillon_preferences_get_type ())
#define EMERILLON_PREFERENCES(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EMERILLON_TYPE_PREFERENCES, EmerillonPreferences))
#define EMERILLON_PREFERENCES_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  EMERILLON_TYPE_PREFERENCES, EmerillonPreferencesClass))
#define EMERILLON_IS_PREFERENCES(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EMERILLON_TYPE_PREFERENCES))
#define EMERILLON_IS_PREFERENCES_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  EMERILLON_TYPE_PREFERENCES))
#define EMERILLON_PREFERENCES_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  EMERILLON_TYPE_PREFERENCES, EmerillonPreferencesClass))

#define EMERILLON_PREFERENCES_ERROR           (emerillon_preferences_error_quark ())

struct _EmerillonPreferences
{
  GtkDialog parent;
  EmerillonPreferencesPrivate *priv;
};

struct _EmerillonPreferencesClass
{
  GtkDialogClass parent_class;
};

GType         emerillon_preferences_get_type        (void) G_GNUC_CONST;

GtkWidget *   emerillon_preferences_dup_default     (void);

G_END_DECLS

#endif
