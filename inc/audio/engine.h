/*
 * audio/engine.h - Audio engine
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

#ifndef __AUDIO_ENGINE_H__
#define __AUDIO_ENGINE_H__

#define BLOCK_LENGTH 4096 // should be set by backend
#define MIDI_BUF_SIZE 1024 // should be set by backend

#include "zrythm.h"
#include "utils/sem.h"

#include <jack/jack.h>
#include <jack/midiport.h>
#include <portaudio.h>

#define AUDIO_ENGINE ZRYTHM->audio_engine
#define MANUAL_PRESS_QUEUE AUDIO_ENGINE->midi_editor_manual_press->midi_events.queue
#define MANUAL_PRESS_EVENTS AUDIO_ENGINE->midi_editor_manual_press->midi_events

typedef jack_nframes_t                nframes_t;
typedef struct Mixer Mixer;

//typedef struct MIDI_Controller
//{
  //jack_midi_event_t    in_event[30];
  //int                  num_events;
//} MIDI_Controller;
typedef struct StereoPorts StereoPorts;
typedef struct Port Port;
typedef struct Channel Channel;
typedef struct Plugin Plugin;
typedef struct Transport Transport;
typedef struct Tracklist Tracklist;

typedef enum EngineBackend
{
  ENGINE_BACKEND_JACK,
  ENGINE_BACKEND_PORT_AUDIO,
  NUM_ENGINE_BACKENDS,
} EngineBackend;

typedef struct AudioEngine
{
  jack_client_t     * client;     ///< jack client
  EngineBackend      backend; ///< current backend, regardless if the selection chagned in preferences
	uint32_t           block_length;   ///< Audio buffer size (block length)
	size_t             midi_buf_size;  ///< Size of MIDI port buffers
	uint32_t           sample_rate;    ///< Sample rate
  int               frames_per_tick;  ///< number of frames per tick
	int               buf_size_set;   ///< True iff buffer size callback fired
  Mixer              * mixer;        ///< the mixer
  StereoPorts       * stereo_in;  ///< stereo in ports from JACK
  StereoPorts       * stereo_out;  ///< stereo out ports to JACK
  Port              * midi_in;     ///< MIDI in port from JACK
  Port              * midi_editor_manual_press; ///< manual note press in editor
  uint32_t          nframes;     ///< nframes for current cycle
  //MIDI_Controller    * midi_controller; ///< the midi input on JACK
  //Port_Manager      * port_manager;  ///< manages all ports created for/by plugins
  ZixSem            port_operation_lock;  ///< semaphore for blocking DSP while plugin and its ports are deleted
  int               run;    ///< ok to process or not
  int               panic; ///< send note off MIDI everywhere

  Port              * ports[600000];   ///< all ports have a reference here for easy access
  int               num_ports;

  /**
   * Port Audio output buffer.
   *
   * Unlike JACK, the audio goes directly here.
   */
  float *            pa_out_buf;

  PaStream *         pa_stream;

  /**
   * Timeline metadata like BPM, time signature, etc.
   */
  Transport         * transport;

} AudioEngine;

AudioEngine *
engine_new ();

void
engine_setup (AudioEngine * self);

/**
 * Updates frames per tick based on the time sig, the BPM,
 * and the sample rate
 */
void
engine_update_frames_per_tick (AudioEngine * self,
                               int beats_per_bar,
                               int bpm,
                               int sample_rate);

/**
 * To be called by each implementation to prepare the
 * structures before processing.
 *
 * Clears buffers, marks all as unprocessed, etc.
 */
void
engine_process_prepare (AudioEngine * self,
                        uint32_t      nframes);

/**
 * To be called after processing for common logic.
 */
void
engine_post_process (AudioEngine * self);

#endif
