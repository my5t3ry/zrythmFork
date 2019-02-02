/*
 * plugins/plugin.h - a base plugin for plugins (lv2/vst/ladspa/etc.)
 *                         to inherit from
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

#ifndef __PLUGINS_BASE_PLUGIN_H__
#define __PLUGINS_BASE_PLUGIN_H__

#include "audio/automatable.h"
#include "audio/port.h"

#include <jack/jack.h>

/* FIXME allocate dynamically */
#define MAX_IN_PORTS 400000
#define MAX_OUT_PORTS 14000
#define MAX_UNKNOWN_PORTS 4000

#define DUMMY_PLUGIN "Dummy Plugin"

typedef struct Channel Channel;

enum PluginCategory
{
  PC_UTILITY,
  PC_GENERATOR,
  PC_DELAY_REVERB,
  PC_MODULATION,
  PC_FILTER,
  PC_DISTORTION,
  PC_DYNAMICS,
  PC_HARDWARE,
  PC_AUDIO
};

typedef enum PluginProtocol
{
 PROT_LV2,
 PROT_DSSI,
 PROT_LADSPA,
 PROT_VST,
 PROT_VST3
} PluginProtocol;

typedef enum PluginArchitecture
{
  ARCH_32,
  ARCH_64
} PluginArchitecture;

/***
 * A descriptor to be implemented by all plugins
 * This will be used throughout the UI
 */
typedef struct PluginDescriptor
{
  char                 * author;
  char                 * name;
  char                 * website;
  char                 * category;           ///< one of the above
  uint8_t              num_audio_ins;             ///< # of input ports
  uint8_t              num_midi_ins;             ///< # of input ports
  uint8_t              num_audio_outs;            ///< # of output ports
  uint8_t              num_midi_outs;            ///< # of output ports
  uint8_t              num_ctrl_ins;            ///< # of input ctrls
  uint8_t              num_ctrl_outs;            ///< # of output ctrls
  PluginArchitecture   arch;               ///< architecture 32/64bit
  PluginProtocol       protocol;           ///< VST/LV2/DSSI/LADSPA...
  char                 * path;
  char                 * uri;            ///< for LV2 plugins
} PluginDescriptor;

/**
 * The base plugin
 * Inheriting plugins must have this as a child
 */
typedef struct Plugin
{
  int                  id;
  void                 * original_plugin;     ///< pointer to original plugin inheriting this base plugin
  PluginDescriptor    * descr;                 ///< descriptor
  Port                 * in_ports[MAX_IN_PORTS];           ///< ports coming in as input
  int                  num_in_ports;    ///< counter
  Port                 * out_ports[MAX_OUT_PORTS];           ///< ports going out as output
  int                  num_out_ports;    ///< counter
  Port                 * unknown_ports[MAX_UNKNOWN_PORTS];           ///< ports with unknown direction
  int                  num_unknown_ports;    ///< counter
  Channel              * channel;        ///< pointer to channel it belongs to
  int                  processed;  ///< processed or not yet
  int                  enabled;     ///< enabled or not
  Automatable *        automatables[900]; ///< automatables
  int                  num_automatables;
} Plugin;

/**
 * Generates automatables for the plugin.
 *
 *
 * Plugin must be instantiated already.
 */
void
plugin_generate_automatables (Plugin * plugin);

/**
 * Used when loading projects.
 */
Plugin *
plugin_get_or_create_blank (int id);

/**
 * Creates/initializes a plugin and its internal plugin (LV2, etc.)
 * using the given descriptor.
 */
Plugin *
plugin_create_from_descr (PluginDescriptor * descr);

/**
 * Loads the plugin from its state file.
 */
void
plugin_load (Plugin * plugin);

/**
 * Instantiates the plugin (e.g. when adding to a channel)
 */
int
plugin_instantiate (Plugin * plugin);

/**
 * Process plugin
 */
void
plugin_process (Plugin * plugin, nframes_t nframes);

/**
 * (re)Generates automatables for the plugin.
 */
void
plugin_update_automatables (Plugin * plugin);

/**
 * Frees given plugin, breaks all its port connections, and frees its ports
 * and other internal pointers
 */
void
plugin_free (Plugin *plugin);

#endif
