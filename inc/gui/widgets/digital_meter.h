/*
 * inc/gui/widgets/digital_meter.h - DigitalMeter
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

#ifndef __GUI_WIDGETS_DIGITAL_METER_H__
#define __GUI_WIDGETS_DIGITAL_METER_H__

#include <gtk/gtk.h>

#define DIGITAL_METER_WIDGET_TYPE \
  (digital_meter_widget_get_type ())
G_DECLARE_FINAL_TYPE (DigitalMeterWidget,
                      digital_meter_widget,
                      Z,
                      DIGITAL_METER_WIDGET,
                      GtkDrawingArea)

typedef enum NoteLength NoteLength;
typedef enum NoteType NoteType;

typedef enum DigitalMeterType
{
  DIGITAL_METER_TYPE_BPM,
  DIGITAL_METER_TYPE_POSITION,
  DIGITAL_METER_TYPE_TIMESIG,
  DIGITAL_METER_TYPE_NOTE_TYPE,
  DIGITAL_METER_TYPE_NOTE_LENGTH,
} DigitalMeterType;

typedef struct SnapGrid SnapGrid;

typedef struct _DigitalMeterWidget
{
  GtkDrawingArea           parent_instance;
  DigitalMeterType         type;
  GtkGestureDrag           * drag;
  double                   last_y;
  int height_start_pos;
  int height_end_pos;

  /* for BPM */
  int                      num_part_start_pos;
  int                      num_part_end_pos;
  int                      dec_part_start_pos;
  int                      dec_part_end_pos;
  int                      update_num; ///< flag to update bpm
  int                      update_dec; ///< flag to update bpm decimal

  /* for position */
  int                      bars_start_pos;
  int                      bars_end_pos;
  int                      beats_start_pos;
  int                      beats_end_pos;
  int                      sixteenths_start_pos;
  int                      sixteenths_end_pos;
  int                      ticks_start_pos;
  int                      ticks_end_pos;
  int                      update_bars; ///< flag to update bars
  int                      update_beats; ///< flag to update beats
  int                      update_sixteenths; ///< flag to update beats
  int                      update_ticks; ///< flag to update beats

  /* for note length/type */
  NoteLength *             note_length;
  NoteType *               note_type;
  int                      update_note_length; ///< flag to update note length
  int                      start_note_length; ///< start note length
  int                      update_note_type; ///< flag to update note type
  int                      start_note_type; ///< start note type

  /* for time sig */
  int                      update_timesig_top;
  int                      start_timesig_top;
  int                      update_timesig_bot;
  int                      start_timesig_bot;
} DigitalMeterWidget;

/**
 * Creates a digital meter with the given type (bpm or position).
 */
DigitalMeterWidget *
digital_meter_widget_new (DigitalMeterType  type,
                          NoteLength *      note_length,
                          NoteType *        note_type);

#endif
