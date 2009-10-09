/*
 * Copyright (C) 2004 Red Hat, Inc.
 * Copyright (C) 2007 The Free Software Foundation
 * Copyright (C) 2008 Marco Barisione <marco@barisione.org>
 *
 * Based on evince code (shell/ev-sidebar.h) by:
 * - Jonathan Blandford <jrb@alum.mit.edu>
 *
 * Base on eog code (src/eog-sidebar.c) by:
 *      - Lucas Rocha <lucasr@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#if !defined (__EMERILLON_H_INSIDE__) && !defined (EMERILLON_COMPILATION)
#error "Only <emerillon/emerillon.h> can be included directly."
#endif

#ifndef __EMERILLON_SIDEBAR_H__
#define __EMERILLON_SIDEBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _EmerillonSidebar EmerillonSidebar;
typedef struct _EmerillonSidebarClass EmerillonSidebarClass;
typedef struct _EmerillonSidebarPrivate EmerillonSidebarPrivate;

#define EMERILLON_TYPE_SIDEBAR            (emerillon_sidebar_get_type())
#define EMERILLON_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), EMERILLON_TYPE_SIDEBAR, EmerillonSidebar))
#define EMERILLON_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  EMERILLON_TYPE_SIDEBAR, EmerillonSidebarClass))
#define EMERILLON_IS_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), EMERILLON_TYPE_SIDEBAR))
#define EMERILLON_IS_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  EMERILLON_TYPE_SIDEBAR))
#define EMERILLON_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  EMERILLON_TYPE_SIDEBAR, EmerillonSidebarClass))

struct _EmerillonSidebar
{
  GtkVBox base_instance;

  EmerillonSidebarPrivate *priv;
};

struct _EmerillonSidebarClass
{
  GtkVBoxClass base_class;

  void (* page_added)   (EmerillonSidebar *sidebar,
                         GtkWidget *main_widget);

  void (* page_removed) (EmerillonSidebar *sidebar,
                         GtkWidget *main_widget);
};

GType      emerillon_sidebar_get_type     (void);

GtkWidget *emerillon_sidebar_new          (void);

void       emerillon_sidebar_add_page     (EmerillonSidebar *sidebar,
                                           const gchar *title,
                                           GtkWidget *main_widget);

void       emerillon_sidebar_remove_page  (EmerillonSidebar *sidebar,
                                           GtkWidget *main_widget);

void       emerillon_sidebar_set_page     (EmerillonSidebar *sidebar,
                                           GtkWidget *main_widget);

gint       emerillon_sidebar_get_n_pages  (EmerillonSidebar *sidebar);

gboolean   emerillon_sidebar_is_empty     (EmerillonSidebar *sidebar);

G_END_DECLS

#endif /* __EMERILLON_SIDEBAR_H__ */
