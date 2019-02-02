/*
 * audio/region.h - A region in the timeline having a start
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

#ifndef __AUDIO_REGION_H__
#define __AUDIO_REGION_H__

#include "audio/position.h"

#include <cyaml/cyaml.h>

#define REGION_PRINTF_FILENAME "%d_%s_%s.mid"

typedef struct _RegionWidget RegionWidget;
typedef struct Channel Channel;
typedef struct Track Track;
typedef struct MidiNote MidiNote;

typedef enum RegionType
{
  REGION_TYPE_MIDI,
  REGION_TYPE_AUDIO
} RegionType;


typedef struct Region
{
  /**
   * ID for saving/loading projects
   */
  int          id;

  /**
   * Region name to be shown on the region.
   */
  char         * name;

  RegionType   type;
  Position     start_pos; ///< start position
  Position     end_pos; ///< end position

  /**
   * Position where the first unit of repeation ends.
   *
   * If end pos > unit_end_pos, then region is repeating.
   */
  Position        unit_end_pos;

  /**
   * Region widget.
   */
  RegionWidget * widget;

  /**
   * Owner track.
   */
  Track        * track;

  /**
   * Linked parent region.
   *
   * Either the midi notes from this region, or the midi
   * notes from the linked region are used
   */
  struct Region       * linked_region;

  int                      selected;
} Region;

static const cyaml_strval_t region_type_strings[] = {
	{ "Midi",          REGION_TYPE_MIDI    },
	{ "Audio",         REGION_TYPE_AUDIO   },
};

static const cyaml_schema_field_t
  region_fields_schema[] =
{
	CYAML_FIELD_INT(
			"id", CYAML_FLAG_DEFAULT,
			Region, id),

  CYAML_FIELD_ENUM(
			"type", CYAML_FLAG_DEFAULT,
			Region, type, region_type_strings,
CYAML_ARRAY_LEN(region_type_strings)),

	CYAML_FIELD_END
};

static const cyaml_schema_value_t
region_schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
			Region, region_fields_schema),
};

static const cyaml_config_t config = {
	.log_level = CYAML_LOG_DEBUG, /* Logging errors and warnings only. */
	.log_fn = cyaml_log,            /* Use the default logging function. */
	.mem_fn = cyaml_mem,            /* Use the default memory allocator. */
};

/**
 * Only to be used by implementing structs.
 */
void
region_init (Region *   region,
             RegionType type,
             Track *    track,
             Position * start_pos,
             Position * end_pos);

/**
 * Clamps position then sets it.
 */
void
region_set_start_pos (Region * region,
                      Position * pos,
                      int      moved); ///< region moved or not (to move notes as
                                          ///< well)

/**
 * Checks if position is valid then sets it.
 */
void
region_set_end_pos (Region * region,
                    Position * end_pos);

/**
 * Returns the region at the given position in the given channel
 */
Region *
region_at_position (Track    * track, ///< the track to look in
                    Position * pos); ///< the position

/**
 * Generates the filename for this region.
 *
 * MUST be free'd.
 */
char *
region_generate_filename (Region * region);

/**
 * Serializes the region.
 *
 * MUST be free'd.
 */
char *
region_serialize (Region * region);

#endif // __AUDIO_REGION_H__
