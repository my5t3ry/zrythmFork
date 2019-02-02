/*
 * inc/gui/widgets/snap_grid.h - Snap and grid selection widget
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __GUI_WIDGETS_SNAP_GRID_H__
#define __GUI_WIDGETS_SNAP_GRID_H__

#include <gtk/gtk.h>

#define SNAP_GRID_WIDGET_TYPE \
  (snap_grid_widget_get_type ())
G_DECLARE_FINAL_TYPE (SnapGridWidget,
                      snap_grid_widget,
                      Z,
                      SNAP_GRID_WIDGET,
                      GtkMenuButton)

typedef struct _SnapGridPopoverWidget SnapGridPopoverWidget;
typedef struct SnapGrid SnapGrid;

typedef struct _SnapGridWidget
{
  GtkMenuButton           parent_instance;
  GtkBox *                box; ///< the box
  GtkImage *              img; ///< img to show next to the label
  GtkLabel                * label; ///< label to show
  SnapGridPopoverWidget   * popover; ///< the popover to show
  GtkBox                  * content; ///< popover content holder
  SnapGrid                * snap_grid;
} SnapGridWidget;

void
snap_grid_widget_setup (SnapGridWidget * self,
                        SnapGrid * snap_grid);

#endif
