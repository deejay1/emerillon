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

#ifndef __EMERILLION_WINDOW_H__
#define __EMERILLION_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS 

typedef struct _EmerillionWindow EmerillionWindow;
typedef struct _EmerillionWindowClass EmerillionWindowClass;
typedef struct _EmerillionWindowPrivate EmerillionWindowPrivate;

#define EMERILLION_TYPE_WINDOW            (emerillion_window_get_type ())
#define EMERILLION_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EMERILLION_TYPE_WINDOW, EmerillionWindow))
#define EMERILLION_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  EMERILLION_TYPE_WINDOW, EmerillionWindowClass))
#define EMERILLION_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EMERILLION_TYPE_WINDOW))
#define EMERILLION_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  EMERILLION_TYPE_WINDOW))
#define EMERILLION_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  EMERILLION_TYPE_WINDOW, EmerillionWindowClass))

#define EMERILLION_WINDOW_ERROR           (emerillion_window_error_quark ())

struct _EmerillionWindow
{
  GtkWindow parent;
  EmerillionWindowPrivate *priv;
};

struct _EmerillionWindowClass
{
  GtkWindowClass parent_class;
};

GType         emerillion_window_get_type        (void) G_GNUC_CONST;

GtkWidget *   emerillion_window_new             (void);

G_END_DECLS

#endif
