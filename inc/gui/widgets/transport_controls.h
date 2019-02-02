/*
 * gui/widgets/transport_controls.h - transport controls
 *   (play/pause/stop...)
 *
 * Copyright (C) 2018 Alexandros Theodotou
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

#ifndef __GUI_WIDGETS_TRANSPORT_CONTROLS_H__
#define __GUI_WIDGETS_TRANSPORT_CONTROLS_H__

#include <gtk/gtk.h>

#define TRANSPORT_CONTROLS_WIDGET_TYPE \
  (transport_controls_widget_get_type ())
G_DECLARE_FINAL_TYPE (TransportControlsWidget,
                      transport_controls_widget,
                      Z,
                      TRANSPORT_CONTROLS_WIDGET,
                      GtkBox)

typedef struct _TransportControlsWidget
{
  GtkBox                   parent_instance;
  GtkButton                * play;
  GtkButton                * stop;
  GtkButton                * backward;
  GtkButton                * forward;
  GtkToggleButton          * trans_record;
  GtkToggleButton          * loop;
} TransportControlsWidget;

#endif
