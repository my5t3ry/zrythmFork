/*
 * gui/widgets/drag_dest_box.c - A dnd destination box used by mixer and tracklist
 *                                widgets
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

/** \file
 */

#include "audio/channel.h"
#include "audio/mixer.h"
#include "audio/track.h"
#include "audio/tracklist.h"
#include "gui/widgets/bot_dock_edge.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/channel.h"
#include "gui/widgets/drag_dest_box.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/mixer.h"
#include "gui/widgets/track.h"
#include "gui/widgets/tracklist.h"
#include "utils/gtk.h"
#include "utils/ui.h"

G_DEFINE_TYPE (DragDestBoxWidget,
               drag_dest_box_widget,
               GTK_TYPE_BOX)

static void
on_drag_data_received (GtkWidget        *widget,
               GdkDragContext   *context,
               gint              x,
               gint              y,
               GtkSelectionData *data,
               guint             info,
               guint             time,
               gpointer          user_data)
{
  DragDestBoxWidget * self = Z_DRAG_DEST_BOX_WIDGET (widget);
  if (self->type == DRAG_DEST_BOX_TYPE_MIXER ||
      self->type == DRAG_DEST_BOX_TYPE_TRACKLIST)
    {
      PluginDescriptor * descr =
        *(gpointer *) gtk_selection_data_get_data (data);
      mixer_add_channel_from_plugin_descr (MIXER,
                                           descr);
    }
}

/**
 * Creates a drag destination box widget.
 */
DragDestBoxWidget *
drag_dest_box_widget_new (GtkOrientation  orientation,
                          int             spacing,
                          DragDestBoxType type)
{
  /* create */
  DragDestBoxWidget * self = g_object_new (
                            DRAG_DEST_BOX_WIDGET_TYPE,
                            "orientation",
                            orientation,
                            "spacing",
                            spacing,
                            NULL);
  self->type = type;

  if (type == DRAG_DEST_BOX_TYPE_MIXER)
    {
      gtk_widget_set_size_request (
        GTK_WIDGET (self),
        20,
        -1);
    }

  /* make expandable */
  gtk_widget_set_vexpand (GTK_WIDGET (self),
                          1);
  gtk_widget_set_hexpand (GTK_WIDGET (self),
                          1);

  /* set as drag dest */
  GtkTargetEntry entries[1];
  entries[0].target = TARGET_ENTRY_PLUGIN_DESCR;
  entries[0].flags = GTK_TARGET_SAME_APP;
  entries[0].info = 0;
  gtk_drag_dest_set (GTK_WIDGET (self),
                     GTK_DEST_DEFAULT_ALL,
                     entries,
                     1,
                     GDK_ACTION_COPY);

  /* connect signal */
  g_signal_connect (GTK_WIDGET (self),
                    "drag-data-received",
                    G_CALLBACK(on_drag_data_received), NULL);

  /* show */
  gtk_widget_set_visible (GTK_WIDGET (self),
                          1);

  return self;
}

/**
 * GTK boilerplate.
 */
static void
drag_dest_box_widget_init (DragDestBoxWidget * self)
{
}
static void
drag_dest_box_widget_class_init (DragDestBoxWidgetClass * klass)
{
}


