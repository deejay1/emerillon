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

#ifndef __EMERILLON_WINDOW_H__
#define __EMERILLON_WINDOW_H__

#include <gtk/gtk.h>
#include <champlain/champlain.h>

G_BEGIN_DECLS 

typedef struct _EmerillonWindow EmerillonWindow;
typedef struct _EmerillonWindowClass EmerillonWindowClass;
typedef struct _EmerillonWindowPrivate EmerillonWindowPrivate;

#define EMERILLON_TYPE_WINDOW            (emerillon_window_get_type ())
#define EMERILLON_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EMERILLON_TYPE_WINDOW, EmerillonWindow))
#define EMERILLON_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  EMERILLON_TYPE_WINDOW, EmerillonWindowClass))
#define EMERILLON_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EMERILLON_TYPE_WINDOW))
#define EMERILLON_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  EMERILLON_TYPE_WINDOW))
#define EMERILLON_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  EMERILLON_TYPE_WINDOW, EmerillonWindowClass))

#define EMERILLON_WINDOW_ERROR           (emerillon_window_error_quark ())

struct _EmerillonWindow
{
  GtkWindow parent;
  EmerillonWindowPrivate *priv;
};

struct _EmerillonWindowClass
{
  GtkWindowClass parent_class;
};

GType         emerillon_window_get_type        (void) G_GNUC_CONST;

GtkWidget *   emerillon_window_new             (void);

ChamplainView * emerillon_window_get_map_view  (EmerillonWindow *window);

GtkUIManager * emerillon_window_get_ui_manager (EmerillonWindow *window);

G_END_DECLS

#endif
