/*
 * gui/widgets/audio_unit.h - AudioUnit widget
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

#ifndef __GUI_WIDGETS_AUDIO_UNIT_H__
#define __GUI_WIDGETS_AUDIO_UNIT_H__

#include <gtk/gtk.h>

#define AUDIO_UNIT_WIDGET_TYPE                  (audio_unit_widget_get_type ())
G_DECLARE_FINAL_TYPE (AudioUnitWidget,
                      audio_unit_widget,
                      Z,
                      AUDIO_UNIT_WIDGET,
                      GtkGrid)

typedef enum AUWidgetType
{
  AUWT_NONE,
  AUWT_CHANNEL,
  AUWT_MASTER,
  AUWT_PLUGIN,
  AUWT_RACK_CONTROLLER,
  AUWT_ENGINE
} AUWidgetType;

typedef struct Plugin Plugin;
typedef struct Channel Channel;
typedef struct RackController RackController;
typedef struct _AudioUnitLabelWidget AudioUnitLabelWidget;
typedef enum AULType AULType;

typedef struct _AudioUnitWidget
{
  GtkGrid                  parent_instance;
  AudioUnitLabelWidget *   l_labels[400];
  int                      num_l_labels;
  AudioUnitLabelWidget *   r_labels[400];
  int                      num_r_labels;
  int                      is_right; ///< 0 for left, 1 for rigth
  GtkLabel *               label;
  GtkBox *                 l_box;
  GtkBox *                 r_box;
  Channel *                channel; ///< if channel;
  Plugin *                 plugin; ///< if plugin;
  RackController *         rc; ///< if rack controller;
  AUWidgetType             type;
} AudioUnitWidget;

AudioUnitWidget *
audio_unit_widget_new (int is_right);

void
audio_unit_widget_set_from_channel (AudioUnitWidget * self,
                                    Channel * chan);

void
audio_unit_widget_set_from_plugin (AudioUnitWidget *self,
                                   Plugin * plugin);

void
audio_unit_widget_set_audio_engine (AudioUnitWidget * self);

void
audio_unit_widget_set_rack_controller (AudioUnitWidget * self,
                                       RackController * rc);

/**
 * Clears the boxes.
 */
void
audio_unit_widget_clear_ports (AudioUnitWidget * self);

/**
 * Adds a port to the corresponding box.
 */
void
audio_unit_widget_add_port (AudioUnitWidget * self,
                            AULType           type,
                            Port *            port);

#endif

