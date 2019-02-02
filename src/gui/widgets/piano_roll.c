/*
 * inc/gui/widgets/editor_notebook.c - Editor notebook (bot of arranger)
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "audio/channel.h"
#include "audio/region.h"
#include "audio/track.h"
#include "gui/widgets/arranger.h"
#include "gui/widgets/bot_dock_edge.h"
#include "gui/widgets/center_dock.h"
#include "gui/widgets/color_area.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/piano_roll.h"
#include "gui/widgets/midi_arranger.h"
#include "gui/widgets/midi_modifier_arranger.h"
#include "gui/widgets/midi_note.h"
#include "gui/widgets/midi_ruler.h"
#include "gui/widgets/piano_roll.h"
#include "gui/widgets/piano_roll_labels.h"
#include "gui/widgets/piano_roll_notes.h"
#include "gui/widgets/ruler.h"
#include "utils/resources.h"

G_DEFINE_TYPE (PianoRollWidget,
               piano_roll_widget,
               GTK_TYPE_BOX)


/**
 * Links scroll windows after all widgets have been
 * initialized.
 */
static void
link_scrolls (
  PianoRollWidget * self)
{
  /* link note labels v scroll to arranger v scroll */
  if (self->piano_roll_labels_scroll)
    {
      gtk_scrolled_window_set_vadjustment (
        self->piano_roll_labels_scroll,
        gtk_scrolled_window_get_vadjustment (
          GTK_SCROLLED_WINDOW (
            self->arranger_scroll)));
    }

  /* link notes v scroll to arranger v scroll */
  if (self->piano_roll_notes_scroll)
    {
      gtk_scrolled_window_set_vadjustment (
        self->piano_roll_notes_scroll,
        gtk_scrolled_window_get_vadjustment (
          GTK_SCROLLED_WINDOW (
            self->arranger_scroll)));
    }

  /* link arranger h scroll to timeline h scroll */
  if (self->arranger_scroll)
    {
      g_assert (MW_CENTER_DOCK);
      g_assert (MW_CENTER_DOCK->timeline_scroll);
      gtk_scrolled_window_set_hadjustment (
        self->arranger_scroll,
        gtk_scrolled_window_get_hadjustment (
          GTK_SCROLLED_WINDOW (
            MW_CENTER_DOCK->timeline_scroll)));
    }

  /* link ruler h scroll to arranger h scroll */
  if (self->midi_ruler_scroll)
    {
      gtk_scrolled_window_set_hadjustment (
        self->midi_ruler_scroll,
        gtk_scrolled_window_get_hadjustment (
          GTK_SCROLLED_WINDOW (
            self->arranger_scroll)));
    }

  /* link modifier arranger h scroll to arranger h scroll */
  if (self->modifier_arranger_scroll)
    {
      gtk_scrolled_window_set_hadjustment (
        self->modifier_arranger_scroll,
        gtk_scrolled_window_get_hadjustment (
          GTK_SCROLLED_WINDOW (
            self->arranger_scroll)));
    }

}

void
piano_roll_widget_setup (
  PianoRollWidget * self,
  PianoRoll *       pr)
{
  self->piano_roll = pr;

  if (self->arranger)
    {
      arranger_widget_setup (
        Z_ARRANGER_WIDGET (self->arranger),
        ZRYTHM->snap_grid_midi,
        ARRANGER_TYPE_MIDI);
      gtk_widget_show_all (
        GTK_WIDGET (self->arranger));
    }
  if (self->modifier_arranger)
    {
      arranger_widget_setup (
        Z_ARRANGER_WIDGET (self->modifier_arranger),
        ZRYTHM->snap_grid_midi,
        ARRANGER_TYPE_MIDI_MODIFIER);
      gtk_widget_show_all (
        GTK_WIDGET (self->modifier_arranger));
    }

  link_scrolls (self);

  GtkAdjustment * adj =
    gtk_scrolled_window_get_vadjustment (
      PIANO_ROLL->arranger_scroll);
  gtk_adjustment_set_value (
    adj,
    gtk_adjustment_get_upper (adj) / 2);
  gtk_scrolled_window_set_vadjustment (
    PIANO_ROLL->arranger_scroll,
    adj);

  piano_roll_notes_widget_refresh (
    self->piano_roll_notes);
}

static void
piano_roll_widget_init (PianoRollWidget * self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_label_set_text (self->midi_name_label,
                      "Select a region...");

  GdkRGBA * color = calloc (1, sizeof (GdkRGBA));
  gdk_rgba_parse (color, "gray");
  color_area_widget_set_color (self->color_bar,
                               color);
}

static void
piano_roll_widget_class_init (
  PianoRollWidgetClass * _klass)
{
  GtkWidgetClass * klass = GTK_WIDGET_CLASS (_klass);
  resources_set_class_template (
    klass,
    "piano_roll.ui");

  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    color_bar);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    midi_bot_toolbar);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    midi_name_label);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    midi_controls_above_notes_box);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    midi_ruler_scroll);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    midi_ruler_viewport);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    ruler);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    piano_roll_labels_scroll);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    piano_roll_labels_viewport);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    piano_roll_labels);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    piano_roll_notes_scroll);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    piano_roll_notes_viewport);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    piano_roll_notes);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    midi_arranger_velocity_paned);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    arranger_scroll);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    arranger_viewport);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    arranger);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    modifier_arranger_scroll);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    modifier_arranger_viewport);
  gtk_widget_class_bind_template_child (
    klass,
    PianoRollWidget,
    modifier_arranger);
}
