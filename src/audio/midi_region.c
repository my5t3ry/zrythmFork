/*
 * audio/midi_region.c - A region in the timeline having a start
 *   and an end
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

#include "audio/channel.h"
#include "audio/midi_note.h"
#include "audio/midi_region.h"
#include "audio/track.h"
#include "gui/widgets/main_window.h"
#include "gui/widgets/midi_region.h"
#include "gui/widgets/region.h"
#include "project.h"

/**
 * Creates region (used when loading projects).
 */
MidiRegion *
midi_region_get_or_create_blank (int id)
{
  if (PROJECT->regions[id])
    {
      return (MidiRegion *) PROJECT->regions[id];
    }
  else
    {
      MidiRegion * midi_region = calloc (1, sizeof (MidiRegion));
      Region * region = (Region *) midi_region;

      region->id = id;
      PROJECT->regions[id] = region;
      PROJECT->num_regions++;

      g_message ("[region_new] Creating blank region %d", id);

      return midi_region;
    }
}

MidiRegion *
midi_region_new (Track *    track,
                 Position * start_pos,
                 Position * end_pos)
{
  MidiRegion * midi_region = calloc (1, sizeof (MidiRegion));

  region_init ((Region *) midi_region,
               REGION_TYPE_MIDI,
               track,
               start_pos,
               end_pos);

  return midi_region;
}

/**
 * Adds midi note to region
 */
void
midi_region_add_midi_note (MidiRegion * region,
                      MidiNote * midi_note)
{
  region->midi_notes [region->num_midi_notes++] = midi_note;
}

