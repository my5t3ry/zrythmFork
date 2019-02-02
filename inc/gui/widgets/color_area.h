/*
 * gui/widgets/color_area.h - channel color picker
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

#ifndef __GUI_WIDGETS_COLOR_AREA_H__
#define __GUI_WIDGETS_COLOR_AREA_H__

/**
 * \file
 *
 * Color picker for a channel strip.
 */

#include <gtk/gtk.h>

#define COLOR_AREA_WIDGET_TYPE \
  (color_area_widget_get_type ())
G_DECLARE_FINAL_TYPE (ColorAreaWidget,
                      color_area_widget,
                      Z,
                      COLOR_AREA_WIDGET,
                      GtkDrawingArea)

typedef struct _ColorAreaWidget
{
  GtkDrawingArea         parent_instance;
  GdkRGBA                * color;          ///< color pointer to set/read value
} ColorAreaWidget;

/**
 * Creates a channel color widget using the given color pointer.
 */
ColorAreaWidget *
color_area_widget_setup (ColorAreaWidget * self,
                         GdkRGBA * color);

/**
 * Changes the color.
 */
void
color_area_widget_set_color (ColorAreaWidget * widget,
                             GdkRGBA * color);

#endif
