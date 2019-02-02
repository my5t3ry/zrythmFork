/*
 * gui/widgets/audio_unit_label.h - AudioUnitLabel widget
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

/** \file
 */

#ifndef __GUI_WIDGETS_AUDIO_UNIT_LABEL_H__
#define __GUI_WIDGETS_AUDIO_UNIT_LABEL_H__

#include <gtk/gtk.h>

#define AUDIO_UNIT_LABEL_WIDGET_TYPE \
  (audio_unit_label_widget_get_type ())
G_DECLARE_FINAL_TYPE (AudioUnitLabelWidget,
                      audio_unit_label_widget,
                      Z,
                      AUDIO_UNIT_LABEL_WIDGET,
                      GtkBox)

typedef enum AULType
{
  AUL_TYPE_LEFT,
  AUL_TYPE_RIGHT
} AULType;

typedef struct Port Port;
typedef struct _AudioUnitWidget AudioUnitWidget;

typedef struct _AudioUnitLabelWidget
{
  GtkBox                 parent_instance;
  GtkDrawingArea *       da;
  GtkBox *               drag_box;
  AudioUnitWidget *      parent; ///< the parent audio unit widget
  Port *                 port; ///< the port it corresponds to
  AULType                type;
  int                    hover; ///< hovered or not
} AudioUnitLabelWidget;

/**
 * Creates a new AudioUnitLabel widget and binds it to the given value.
 */
AudioUnitLabelWidget *
audio_unit_label_widget_new (AULType type,
                             Port *  port,
                             AudioUnitWidget * parent);

#endif
