/*
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

/** \file */

#ifndef __GUI_WIDGETS_AUTOMATION_POINT_H__
#define __GUI_WIDGETS_AUTOMATION_POINT_H__

#include <audio/automation_point.h>

#include <gtk/gtk.h>

#define AUTOMATION_POINT_WIDGET_TYPE \
  (automation_point_widget_get_type ())
G_DECLARE_FINAL_TYPE (AutomationPointWidget,
                      automation_point_widget,
                      Z,
                      AUTOMATION_POINT_WIDGET,
                      GtkBox)

#define AP_WIDGET_POINT_SIZE 6
#define AP_WIDGET_CURVE_H 2
#define AP_WIDGET_CURVE_W 8
#define AP_WIDGET_PADDING 1

typedef enum AutomationPointWidgetState
{
  APW_STATE_NONE,
  APW_STATE_SELECTED,
  APW_STATE_HOVER
} AutomationPointWidgetState;

typedef struct _AutomationPointWidget
{
  GtkBox                      parent_instance;
  AutomationPoint *           ap;   ///< the automation_point associated with this
  AutomationPointWidgetState  state;
} AutomationPointWidget;

/**
 * Creates a automation_point.
 */
AutomationPointWidget *
automation_point_widget_new (AutomationPoint * automation_point);

/**
 * Sets hover state and queues draw.
 */
void
automation_point_widget_set_state_and_queue_draw (AutomationPointWidget *    self,
                                                  AutomationPointWidgetState state);

GType automation_point_widget_get_type(void);

#endif


