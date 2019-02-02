/*
 * audio/engine_jack.c - Jack engine
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

#include "audio/channel.h"
#include "audio/engine.h"
#include "audio/engine_jack.h"
#include "audio/midi.h"
#include "audio/mixer.h"
#include "audio/port.h"
#include "audio/transport.h"
#include "plugins/plugin.h"
#include "plugins/lv2_plugin.h"
#include "project.h"

#include <gtk/gtk.h>

#define JACK_PORT_T(exp) ((jack_port_t *) exp)
#define MIDI_IN_EVENT(i) (AUDIO_ENGINE->midi_in->midi_events.jack_midi_events[i])
#define MIDI_IN_NUM_EVENTS AUDIO_ENGINE->midi_in->midi_events.num_events

/** Jack sample rate callback. */
int
jack_sample_rate_cb(uint32_t nframes, void * data)
{
  AudioEngine * engine = (AudioEngine *) data;
  engine->sample_rate = nframes;

  if (PROJECT)
    engine_update_frames_per_tick (
      engine,
      engine->transport->beats_per_bar,
      engine->transport->bpm,
      engine->sample_rate);
  else
    engine_update_frames_per_tick (engine, 4, 120, 44000);

  g_message ("JACK: Sample rate changed to %d", nframes);
  return 0;
}

/** Jack buffer size callback. */
int
jack_buffer_size_cb (uint32_t nframes,
                     void *   data)
{
  AUDIO_ENGINE->block_length = nframes;
  AUDIO_ENGINE->buf_size_set = true;
#ifdef HAVE_JACK_PORT_TYPE_GET_BUFFER_SIZE
  AUDIO_ENGINE->midi_buf_size =
    jack_port_type_get_buffer_size(
      AUDIO_ENGINE->client, JACK_DEFAULT_MIDI_TYPE);
#endif
  g_message (
    "JACK: Block length changed to %d, midi buf size to %lu",
    AUDIO_ENGINE->block_length,
    AUDIO_ENGINE->midi_buf_size);

  /** reallocate port buffers to new size */
  g_message ("Reallocating port buffers to %d", nframes);
  for (int i = 0; i < AUDIO_ENGINE->num_ports; i++)
    {
      Port * port = AUDIO_ENGINE->ports[i];
      port->nframes = nframes;
      port->buf = realloc (port->buf, nframes * sizeof (float));
      /* TODO memset */
    }
  for (int i = 0; i < MIXER->num_channels; i++)
    {
      Channel * channel = MIXER->channels[i];
      for (int j = 0; j < STRIP_SIZE; j++)
        {
          if (channel->strip[j])
            {
              Plugin * plugin = channel->strip[j];
              if (plugin->descr->protocol == PROT_LV2)
                {
                  lv2_allocate_port_buffers (
                                (Lv2Plugin *)plugin->original_plugin);
                }
            }
        }
    }
  /* FIXME this is the same as block_length */
  AUDIO_ENGINE->nframes = nframes;
  return 0;
}

/**
 * The process callback for this JACK application is
 * called in a special realtime thread once for each audio
 * cycle.
 */
int
jack_process_cb (nframes_t nframes, ///< the number of frames to fill
                 void *    data) ///< user data
{
  AudioEngine * engine = (AudioEngine *) data;
  if (!engine->run)
    return 0;

  engine_process_prepare (engine,
                          nframes);

  float * stereo_out_l, * stereo_out_r;

  /* get MIDI events from JACK and store to engine MIDI
   * in port
   */
  void* port_buf = jack_port_get_buffer (
        JACK_PORT_T (engine->midi_in->data), nframes);
  MIDI_IN_NUM_EVENTS = jack_midi_get_event_count(port_buf);
  if(MIDI_IN_NUM_EVENTS > 0)
    {
      g_message ("JACK: have %d events", MIDI_IN_NUM_EVENTS);
      for(int i=0; i < MIDI_IN_NUM_EVENTS; i++)
        {
          jack_midi_event_t * event = &MIDI_IN_EVENT(i);
          jack_midi_event_get(event, port_buf, i);
          uint8_t type = event->buffer[0] & 0xf0;
          uint8_t channel = event->buffer[0] & 0xf;
          switch (type)
            {
              case MIDI_CH1_NOTE_ON:
                assert (event->size == 3);
                g_message (" note on  (channel %2d): pitch %3d, velocity %3d", channel, event->buffer[1], event->buffer[2]);
                break;
              case MIDI_CH1_NOTE_OFF:
                assert (event->size == 3);
                g_message (" note off (channel %2d): pitch %3d, velocity %3d", channel, event->buffer[1], event->buffer[2]);
                break;
              case MIDI_CH1_CTRL_CHANGE:
                assert (event->size == 3);
                g_message (" control change (channel %2d): controller %3d, value %3d", channel, event->buffer[1], event->buffer[2]);
                break;
              default:
                      break;
            }
          /*g_message ("    event %d time is %d. 1st byte is 0x%x", i,*/
                     /*MIDI_IN_EVENT(i).time, *(MIDI_IN_EVENT(i).buffer));*/
        }
    }
  /* get MIDI events from other sources */
  /*if (engine->panic)*/
    /*{*/
      /*midi_panic (&engine->midi_editor_manual_press->midi_events);*/
    /*}*/
  midi_events_dequeue (&engine->midi_editor_manual_press->midi_events);

  /*
   * process
   */
  mixer_process (MIXER, nframes);

  /**
   * get jack's buffers with nframes frames for left & right
   */
  stereo_out_l = (float *)
    jack_port_get_buffer (JACK_PORT_T (engine->stereo_out->l->data),
                          nframes);
  stereo_out_r = (float *)
    jack_port_get_buffer (JACK_PORT_T (engine->stereo_out->r->data),
                           nframes);

  /* by this time, the Master channel should have its Stereo Out ports filled.
   * pass their buffers to jack's buffers */
  for (int i = 0; i < nframes; i++)
    {
      stereo_out_l[i] = MIXER->master->stereo_out->l->buf[i];
      stereo_out_r[i] = MIXER->master->stereo_out->r->buf[i];
    }
  (void) stereo_out_l; /* avoid unused warnings */
  (void) stereo_out_r;

  engine_post_process (engine);

  /*
   * processing finished, return 0
   */
  return 0;
}

/**
 * JACK calls this shutdown_callback if the server ever
 * shuts down or decides to disconnect the client.
 */
void
jack_shutdown_cb (void *arg)
{
  // TODO
  g_warning ("Jack shutting down...");
}

/**
 * Sets up the audio engine to use jack.
 */
void
jack_setup (AudioEngine * self)
{
  g_message ("Setting up JACK...");

  const char **ports;
  const char *client_name = "Zrythm";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  // open a client connection to the JACK server
  self->client = jack_client_open (client_name, options, &status, server_name);
  if (self->client == NULL) {
          g_error ("jack_client_open() failed, "
                   "status = 0x%2.0x", status);
          if (status & JackServerFailed) {
                  g_error ("Unable to connect to JACK server");
          }
          exit (1);
  }
  if (status & JackServerStarted) {
  // FIXME g_info
          g_message ("JACK server started\n");
  }
  if (status & JackNameNotUnique) {
          client_name = jack_get_client_name(self->client);
          g_error ("unique name `%s' assigned\n", client_name);
  }

  /* Set audio engine properties */
  self->sample_rate   = jack_get_sample_rate (self->client);
  self->block_length  = jack_get_buffer_size (self->client);
  self->midi_buf_size = 4096;
#ifdef HAVE_JACK_PORT_TYPE_GET_BUFFER_SIZE
  self->midi_buf_size = jack_port_type_get_buffer_size (
        self->client, JACK_DEFAULT_MIDI_TYPE);
#endif


  /* set jack callbacks */
  jack_set_process_callback (self->client, &jack_process_cb, self);
  jack_set_buffer_size_callback(self->client, &jack_buffer_size_cb, self);
  jack_set_sample_rate_callback(self->client, &jack_sample_rate_cb, self);
  jack_on_shutdown(self->client, &jack_shutdown_cb, self);
  /*jack_set_latency_callback(client, &jack_latency_cb, arg);*/
#ifdef JALV_JACK_SESSION
  /*jack_set_session_callback(client, &jack_session_cb, arg);*/
#endif

  /* create ports */
  Port * stereo_out_l = port_new_with_data (
        self->block_length,
        INTERNAL_JACK_PORT,
        TYPE_AUDIO,
        FLOW_OUTPUT,
        "JACK Stereo Out / L",
        (void *) jack_port_register (self->client, "Stereo_out_L",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0));
  Port * stereo_out_r = port_new_with_data (
        self->block_length,
        INTERNAL_JACK_PORT,
        TYPE_AUDIO,
        FLOW_OUTPUT,
        "JACK Stereo Out / R",
        (void *) jack_port_register (self->client, "Stereo_out_R",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0));
  Port * stereo_in_l = port_new_with_data (
        self->block_length,
        INTERNAL_JACK_PORT,
        TYPE_AUDIO,
        FLOW_INPUT,
        "JACK Stereo In / L",
        (void *) jack_port_register (self->client, "Stereo_in_L",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsInput, 0));
  Port * stereo_in_r = port_new_with_data (
        self->block_length,
        INTERNAL_JACK_PORT,
        TYPE_AUDIO,
        FLOW_INPUT,
        "JACK Stereo In / R",
        (void *) jack_port_register (self->client, "Stereo_in_R",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsInput, 0));
  Port * midi_in = port_new_with_data (
        self->block_length,
        INTERNAL_JACK_PORT,
        TYPE_EVENT,
        FLOW_INPUT,
        "JACK MIDI In",
        (void *) jack_port_register (self->client, "MIDI_in",
                                     JACK_DEFAULT_MIDI_TYPE,
                                     JackPortIsInput, 0));

  Port * midi_editor_manual_press = port_new_with_type (
        self->block_length,
        TYPE_EVENT,
        FLOW_INPUT,
        "MIDI Editor Manual Press");


  stereo_in_l->owner_jack = 1;
  stereo_in_r->owner_jack = 1;
  stereo_out_l->owner_jack = 1;
  stereo_out_r->owner_jack = 1;
  midi_in->owner_jack = 1;
  midi_editor_manual_press->owner_jack = 0;

  self->stereo_in  = stereo_ports_new (stereo_in_l, stereo_in_r);
  self->stereo_out = stereo_ports_new (stereo_out_l, stereo_out_r);
  self->midi_in    = midi_in;
  self->midi_editor_manual_press = midi_editor_manual_press;

  /* init MIDI queues for manual presse/piano roll */
  self->midi_editor_manual_press->midi_events.queue = calloc (1, sizeof (MidiEvents));

  if (!self->stereo_in->l->data || !self->stereo_in->r->data ||
      !self->stereo_out->l->data || !self->stereo_out->r->data ||
      !self->midi_in->data)
    {
      g_error ("no more JACK ports available");
    }

  /* init semaphore */
  zix_sem_init (&self->port_operation_lock, 1);

  /* Tell the JACK server that we are ready to roll.  Our
   * process() callback will start running now. */
  if (jack_activate (self->client))
    {
      g_error ("cannot activate client");
      return;
    }
  g_message ("Jack activated");

  /* Connect the ports.  You can't do this before the client is
   * activated, because we can't make connections to clients
   * that aren't running.  Note the confusing (but necessary)
   * orientation of the driver backend ports: playback ports are
   * "input" to the backend, and capture ports are "output" from
   * it.
   */

  ports = jack_get_ports (self->client, NULL, NULL,
                          JackPortIsPhysical|JackPortIsInput);
  if (ports == NULL) {
          g_error ("no physical playback ports\n");
          exit (1);
  }

  if (jack_connect (self->client, jack_port_name (
              JACK_PORT_T(self->stereo_out->l->data)), ports[0])) {
          g_error ("cannot connect output ports\n");
  }

  if (jack_connect (self->client, jack_port_name (
              JACK_PORT_T (self->stereo_out->r->data)), ports[1])) {
          g_error ("cannot connect output ports\n");
  }

  jack_free (ports);

  g_message ("JACK set up");
}
