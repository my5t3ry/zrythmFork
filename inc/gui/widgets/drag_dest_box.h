/*
 * gui/widgets/drag_dest_box.h - A dnd destination box used
 * by mixer and tracklist widgets
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

#ifndef __GUI_WIDGETS_DRAG_DEST_BOX_H__
#define __GUI_WIDGETS_DRAG_DEST_BOX_H__

#include <gtk/gtk.h>

#define DRAG_DEST_BOX_WIDGET_TYPE \
  (drag_dest_box_widget_get_type ())
G_DECLARE_FINAL_TYPE (DragDestBoxWidget,
                      drag_dest_box_widget,
                      Z,
                      DRAG_DEST_BOX_WIDGET,
                      GtkBox)

typedef struct Channel Channel;

typedef enum DragDestBoxType
{
  DRAG_DEST_BOX_TYPE_MIXER,
  DRAG_DEST_BOX_TYPE_TRACKLIST
} DragDestBoxType;

typedef struct _DragDestBoxWidget
{
  GtkBox                  parent_instance;
  DragDestBoxType         type;
} DragDestBoxWidget;

/**
 * Creates a drag destination box widget.
 */
DragDestBoxWidget *
drag_dest_box_widget_new (GtkOrientation  orientation,
                          int             spacing,
                          DragDestBoxType type);

#endif
