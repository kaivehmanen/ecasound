// ------------------------------------------------------------------------
// eca-object-factory.cpp: Class for creating various ecasound objects.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <kvutils/kvu_numtostr.h>

#include "audioio.h"
#include "midiio.h"
#include "audiofx_ladspa.h"
#include "generic-controller.h"
#include "eca-static-object-maps.h"
#include "eca-object-factory.h"

/**
 * Return the first effect that matches with 'keyword'
 */
EFFECT_LADSPA* ECA_OBJECT_FACTORY::ladspa_map_object(const string& keyword) { 
  return(dynamic_cast<EFFECT_LADSPA*>(eca_ladspa_plugin_map->object(keyword))); 
}

/**
 * Return the first effect that matches with 'number'
 */
EFFECT_LADSPA* ECA_OBJECT_FACTORY::ladspa_map_object(long int number) { 
  return(dynamic_cast<EFFECT_LADSPA*>(eca_ladspa_plugin_id_map->object(kvu_numtostr(number)))); 
}

/**
 * Return the first controller object that matches with 'keyword'
 */
GENERIC_CONTROLLER* ECA_OBJECT_FACTORY::controller_map_object(const string& keyword) {
  return(dynamic_cast<GENERIC_CONTROLLER*>(eca_controller_map->object(keyword)));
}

/**
 * Return the first effect that matches with 'keyword'
 */
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::chain_operator_map_object(const string& keyword) {
  return(dynamic_cast<CHAIN_OPERATOR*>(eca_chain_operator_map->object(keyword)));
}

/**
 * Return the first audio object that matches with 'keyword'. If 
 * 'use_regex', regular expression matching is used.
 */
AUDIO_IO* ECA_OBJECT_FACTORY::audio_io_map_object(const string& keyword, bool use_regex ) {
  return(dynamic_cast<AUDIO_IO*>(eca_audio_object_map->object(keyword, use_regex)));
}

/**
 * Create a new audio object based on the formatted argument string.
 *
 * @param arg a formatted string describing an audio object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * require:
 *  arg.empty() != true
 */
AUDIO_IO* ECA_OBJECT_FACTORY::create_audio_object(const string& arg) {
  assert(arg.empty() != true);
 
  string fname = get_argument_number(1, arg);
  if (fname.find(".") != string::npos) {
    fname = string(fname, fname.find_last_of("."), string::npos);
  }

  AUDIO_IO* main_file = 0;
  main_file = audio_io_map_object(fname);

  if (main_file != 0) {
    main_file = main_file->new_expr();
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-object-factory) Object \"" + arg + "\" created, type \"" + main_file->name() + "\". Has " + kvu_numtostr(main_file->number_of_params()) + " parameter(s).");
    for(int n = 0; n < main_file->number_of_params(); n++) {
      main_file->set_parameter(n + 1, get_argument_number(n + 1, arg));
    }
  }
  return(main_file);
}

/**
 * Create a new MIDI-device object based on the formatted argument string.
 *
 * @param arg a formatted string describing a MIDI-device object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * require:
 *  arg.empty() != true
 */
MIDI_IO* ECA_OBJECT_FACTORY::create_midi_device(const string& arg) {
  assert(arg.empty() != true);
 
  ::register_default_objects();
  string fname = get_argument_number(1, arg);

  MIDI_IO* device = 0;
  device = dynamic_cast<MIDI_IO*>(eca_midi_device_map->object(fname));

  if (device != 0) {
    device = device->new_expr();
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-object-factory) Object \"" + arg + "\" created, type \"" + device->name() + "\". Has " + kvu_numtostr(device->number_of_params()) + " parameter(s).");
    for(int n = 0; n < device->number_of_params(); n++) {
      device->set_parameter(n + 1, get_argument_number(n + 1, arg));
    }
  }
  return(device);
}
