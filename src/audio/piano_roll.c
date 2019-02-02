/*
 * audio/piano_roll.c - piano roll back end
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

#include <stdlib.h>

#include "audio/piano_roll.h"

PianoRoll *
piano_roll_new ()
{
  PianoRoll * self = calloc (1, sizeof (PianoRoll));

  self->notes_zoom = 1; /* FIXME */

  self->midi_modifier = MIDI_MODIFIER_VELOCITY;

  return self;
}
