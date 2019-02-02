/*
 * gui/widgets/midi_ruler.c - Ruler
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

#include "gui/widgets/ruler.h"
#include "gui/widgets/midi_ruler.h"

#include <gtk/gtk.h>

G_DEFINE_TYPE (MidiRulerWidget,
               midi_ruler_widget,
               RULER_WIDGET_TYPE)

static void
midi_ruler_widget_class_init (
  MidiRulerWidgetClass * klass)
{
}

static void
midi_ruler_widget_init (
  MidiRulerWidget * self)
{
  g_message ("initing midi ruler");
}

