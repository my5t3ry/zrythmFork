/*
 * gui/widgets/automation_tracklist.h - Automation tracklist
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

#ifndef __GUI_WIDGETS_AUTOMATION_TRACKLIST_H__
#define __GUI_WIDGETS_AUTOMATION_TRACKLIST_H__

#include <dazzle.h>
#include <gtk/gtk.h>

#define AUTOMATION_TRACKLIST_WIDGET_TYPE                  (automation_tracklist_widget_get_type ())
G_DECLARE_FINAL_TYPE (AutomationTracklistWidget,
                      automation_tracklist_widget,
                      Z,
                      AUTOMATION_TRACKLIST_WIDGET,
                      DzlMultiPaned)

typedef struct _AutomationTrackWidget AutomationTrackWidget;
typedef struct _TrackWidget TrackWidget;
typedef struct AutomationTracklist AutomationTracklist;

typedef struct _AutomationTracklistWidget
{
  DzlMultiPaned           parent_instance;

  /**
   * The backend.
   */
  AutomationTracklist *   automation_tracklist;
} AutomationTracklistWidget;

/**
 * Creates a new tracks widget and sets it to main window.
 */
AutomationTracklistWidget *
automation_tracklist_widget_new (
  AutomationTracklist * automation_tracklist);

/**
 * Show or hide all automation track widgets.
 */
void
automation_tracklist_widget_refresh (
  AutomationTracklistWidget * self);

#endif

