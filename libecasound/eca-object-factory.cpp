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

#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>
#include <kvutils/message_item.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "audioio.h"
#include "audioio-loop.h"
#include "midiio.h"
#include "audiofx_ladspa.h"
#include "generic-controller.h"
#include "eca-static-object-maps.h"
#include "eca-object-map.h"
#include "eca-object-factory.h"

// FIXME: add checks for uninitialized object maps (map == 0)

/**
 * Return the first effect that matches with 'keyword'
 */
EFFECT_LADSPA* ECA_OBJECT_FACTORY::ladspa_map_object(const std::string& keyword, bool use_regex = true) { 
  return(dynamic_cast<EFFECT_LADSPA*>(eca_ladspa_plugin_map->object(keyword, use_regex))); 
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
GENERIC_CONTROLLER* ECA_OBJECT_FACTORY::controller_map_object(const std::string& keyword, bool use_regex = true) {
  return(dynamic_cast<GENERIC_CONTROLLER*>(eca_controller_map->object(keyword,use_regex)));
}

/**
 * Return the first effect that matches with 'keyword'
 */
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::chain_operator_map_object(const std::string& keyword, bool use_regex = true) {
  return(dynamic_cast<CHAIN_OPERATOR*>(eca_chain_operator_map->object(keyword,use_regex)));
}

/**
 * Return the first audio object that matches with 'keyword'. If 
 * 'use_regex', regular expression matching is used.
 */
AUDIO_IO* ECA_OBJECT_FACTORY::audio_io_map_object(const std::string& keyword, bool use_regex ) {
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
 *  @pre arg.empty() != true
 */
AUDIO_IO* ECA_OBJECT_FACTORY::create_audio_object(const std::string& arg) {
  // --
  DBC_REQUIRE(arg.empty() != true);
  // --
 
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
MIDI_IO* ECA_OBJECT_FACTORY::create_midi_device(const std::string& arg) {
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

/**
 * Create a new loop input object.
 *
 * @param arg a formatted string describing an loop object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * @pre argu.empty() != true
 */
AUDIO_IO* ECA_OBJECT_FACTORY::create_loop_input(const std::string& argu,
						 std::map<int,LOOP_DEVICE*>* loop_map) {
  // --------
  DBC_REQUIRE(argu.empty() != true);
  // --------

  LOOP_DEVICE* p = 0;
  string tname = get_argument_number(1, argu);
  if (tname.find("loop") != string::npos) {
    int id = atoi(get_argument_number(2, argu).c_str());
    p = new LOOP_DEVICE(id);
    if (loop_map->find(id) == loop_map->end()) { 
      (*loop_map)[id] = p;
    }
    else
      p = (*loop_map)[id];

    p->register_input();
  }
  
  return(p);
}

/**
 * Create a new loop output object.
 *
 * @param arg a formatted string describing an loop object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * @pre argu.empty() != true
 */
AUDIO_IO* ECA_OBJECT_FACTORY::create_loop_output(const std::string& argu,
						 std::map<int,LOOP_DEVICE*>* loop_map) {
  // --------
  DBC_REQUIRE(argu.empty() != true);
  // --------

  LOOP_DEVICE* p = 0;
  string tname = get_argument_number(1, argu);
  if (tname.find("loop") != string::npos) {
    int id = atoi(get_argument_number(2, argu).c_str());
    p = new LOOP_DEVICE(id);
    if (loop_map->find(id) == loop_map->end()) { 
      (*loop_map)[id] = p;
    }
    else
      p = (*loop_map)[id];

    p->register_output();
  }
  
  return(p);
}

/**
 * Creates a new LADSPA plugin.
 *
 * @param arg a formatted string describing an LADSPA object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 */
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_ladspa_plugin (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

#ifdef HAVE_LADSPA_H
  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = 0;
 std::string prefix = get_argument_prefix(argu);
  if (prefix == "el" || prefix == "eli") {
 std::string unique = get_argument_number(1, argu);
    if (prefix == "el") 
      cop = ECA_OBJECT_FACTORY::ladspa_map_object(unique);
    else 
      cop = ECA_OBJECT_FACTORY::ladspa_map_object(atol(unique.c_str()));

    if (cop != 0) {
      cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());

      ecadebug->msg("(eca-object-factory) Creating LADSPA-plugin \"" +
		    cop->name() + "\"");
      otemp << "(eca-object-factory) Setting parameters: ";
      for(int n = 0; n < cop->number_of_params(); n++) {
	cop->set_parameter(n + 1, atof(get_argument_number(n + 2, argu).c_str()));
	otemp << cop->get_parameter_name(n + 1) << " = ";
	otemp << cop->get_parameter(n + 1);
	if (n + 1 < cop->number_of_params()) otemp << ", ";
      }
      ecadebug->msg(otemp.to_string());
    }
    return(cop);
  }
#endif /* HAVE_LADSPA_H */
  return(0);
}

/**
 * Creates a new VST1.0/2.0 plugin.
 *
 * Notes: VST support is currently not used 
 *        because of licensing problems 
 *        (distribution of VST-headers is not
 *        allowed).
 */
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_vst_plugin (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = 0;
  std::string prefix = get_argument_prefix(argu);

#ifdef FEELING_EXPERIMENTAL
  cop = dynamic_cast<CHAIN_OPERATOR*>(eca_vst_plugin_map.object(prefix));
#endif
  if (cop != 0) {
    
    ecadebug->msg("(eca-object-factory) Creating VST-plugin \"" + cop->name() + "\"");
    otemp << "(eca-object-factory) Setting parameters: ";
    for(int n = 0; n < cop->number_of_params(); n++) {
      cop->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << cop->get_parameter_name(n + 1) << " = ";
      otemp << cop->get_parameter(n + 1);
      if (n + 1 < cop->number_of_params()) otemp << ", ";
    }
    ecadebug->msg(otemp.to_string());
  }
  return(cop);
}

/**
 * Creates a new chain operator object.
 *
 * @param arg a formatted string describing an chain operator object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 */
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_chain_operator (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  string prefix = get_argument_prefix(argu);

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = ECA_OBJECT_FACTORY::chain_operator_map_object(prefix);
  if (cop != 0) {
    cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());

    ecadebug->msg("(eca-object-factory) Creating chain operator \"" +
			   cop->name() + "\"");
    //    otemp << "(eca-chainsetup) Adding effect " << cop->name();
    otemp << "(eca-object-factory) Setting parameters: ";
    for(int n = 0; n < cop->number_of_params(); n++) {
      cop->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << cop->get_parameter_name(n + 1) << " = ";
      otemp << cop->get_parameter(n +1);
      if (n + 1 < cop->number_of_params()) otemp << ", ";
    }
    ecadebug->msg(otemp.to_string());
    return(cop);
  }
  return(0);
}

/**
 * Creates a new generic controller object.
 *
 * @param arg a formatted string describing an generic controller object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 */
GENERIC_CONTROLLER* ECA_OBJECT_FACTORY::create_controller (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() > 0 && argu[0] != '-') return(0);
  string prefix = get_argument_prefix(argu);

  GENERIC_CONTROLLER* gcontroller = ECA_OBJECT_FACTORY::controller_map_object(prefix);
  if (gcontroller != 0) {
    gcontroller = gcontroller->clone();

    ecadebug->msg("(eca-object-factory) Creating controller source \"" +  gcontroller->name() + "\"");

    MESSAGE_ITEM otemp;
    otemp << "(eca-object-factory) Setting parameters: ";
    int numparams = gcontroller->number_of_params();
    for(int n = 0; n < numparams; n++) {
      gcontroller->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << gcontroller->get_parameter_name(n + 1) << " = ";
      otemp << gcontroller->get_parameter(n +1);
      numparams = gcontroller->number_of_params(); // in case 'n_o_p()' varies
      if (n + 1 < numparams) otemp << ", ";
    }
    ecadebug->msg(otemp.to_string());
    return(gcontroller);
  }
  return(0);
}
