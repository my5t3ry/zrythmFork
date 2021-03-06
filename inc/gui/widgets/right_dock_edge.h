/*
 * gui/widgets/right_dock_edge.h - Main window widget
 *
 * Copyright (C) 2019 Alexandros Theodotou
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __GUI_WIDGETS_RIGHT_DOCK_EDGE_H__
#define __GUI_WIDGETS_RIGHT_DOCK_EDGE_H__

#include <gtk/gtk.h>
#include <dazzle.h>

#define RIGHT_DOCK_EDGE_WIDGET_TYPE \
  (right_dock_edge_widget_get_type ())
G_DECLARE_FINAL_TYPE (RightDockEdgeWidget,
                      right_dock_edge_widget,
                      Z,
                      RIGHT_DOCK_EDGE_WIDGET,
                      GtkBox)

#define MW_RIGHT_DOCK_EDGE MW_CENTER_DOCK->right_dock_edge

typedef struct _BrowserWidget BrowserWidget;

typedef struct _RightDockEdgeWidget
{
  GtkBox                   parent_instance;
  GtkNotebook              * right_notebook;
  BrowserWidget            * browser;
} RightDockEdgeWidget;

#endif
