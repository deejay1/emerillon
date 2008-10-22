/*
 * Copyright (C) 2004 Red Hat, Inc.
 * Copyright (C) 2007 The Free Software Foundation
 * Copyright (C) 2008 Marco Barisione <marco@barisione.org>
 *
 * Based on evince code (shell/ev-sidebar.h) by: 
 * 	- Jonathan Blandford <jrb@alum.mit.edu>
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

#ifndef __EMERILLION_SIDEBAR_H__
#define __EMERILLION_SIDEBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _EmerillionSidebar EmerillionSidebar;
typedef struct _EmerillionSidebarClass EmerillionSidebarClass;
typedef struct _EmerillionSidebarPrivate EmerillionSidebarPrivate;

#define EMERILLION_TYPE_SIDEBAR            (emerillion_sidebar_get_type())
#define EMERILLION_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), EMERILLION_TYPE_SIDEBAR, EmerillionSidebar))
#define EMERILLION_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  EMERILLION_TYPE_SIDEBAR, EmerillionSidebarClass))
#define EMERILLION_IS_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), EMERILLION_TYPE_SIDEBAR))
#define EMERILLION_IS_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  EMERILLION_TYPE_SIDEBAR))
#define EMERILLION_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  EMERILLION_TYPE_SIDEBAR, EmerillionSidebarClass))

struct _EmerillionSidebar
{
  GtkVBox base_instance;

  EmerillionSidebarPrivate *priv;
};

struct _EmerillionSidebarClass
{
  GtkVBoxClass base_class;

  void (* page_added)   (EmerillionSidebar *sidebar, 
                         GtkWidget *main_widget);

  void (* page_removed) (EmerillionSidebar *sidebar, 
                         GtkWidget *main_widget);
};

GType      emerillion_sidebar_get_type     (void);

GtkWidget *emerillion_sidebar_new          (void);

void       emerillion_sidebar_add_page     (EmerillionSidebar *sidebar,
                                            const gchar *title,
                                            GtkWidget *main_widget);

void       emerillion_sidebar_remove_page  (EmerillionSidebar *sidebar,
                                            GtkWidget *main_widget);

void       emerillion_sidebar_set_page     (EmerillionSidebar *sidebar,
                                            GtkWidget *main_widget);

gint       emerillion_sidebar_get_n_pages  (EmerillionSidebar *sidebar);

gboolean   emerillion_sidebar_is_empty     (EmerillionSidebar *sidebar);

G_END_DECLS

#endif /* __EMERILLION_SIDEBAR_H__ */


