// ------------------------------------------------------------------------
// eca-object-factory.cpp: Abstract factory for creating libecasound 
//                         objects.
// Copyright (C) 2000-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <list>
#include <map>
#include <string>
#include <pthread.h>

#include <kvu_dbc.h>
#include "kvu_locks.h"
#include <kvu_numtostr.h>
#include <kvu_message_item.h>

#include "audioio.h"
#include "audioio-loop.h"
#include "midiio.h"
#include "audiofx_ladspa.h"
#include "generic-controller.h"
#include "eca-static-object-maps.h"
#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-object-factory.h"
#include "eca-logger.h"

/**
 * Import std namespace.
 */
using std::list;
using std::map;
using std::string;

/**
 * Initialize static member variables.
 */

ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::audio_io_rt_map_repp = 0;
ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::audio_io_nonrt_map_repp = 0;
ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::chain_operator_map_repp = 0;
ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::ladspa_plugin_map_repp = 0;
ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::ladspa_plugin_id_map_repp = 0;
ECA_PRESET_MAP* ECA_OBJECT_FACTORY::preset_map_repp = 0;
ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::controller_map_repp = 0;
ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::midi_device_map_repp = 0;

pthread_mutex_t ECA_OBJECT_FACTORY::lock_rep = PTHREAD_MUTEX_INITIALIZER;

/**
 * Definitions for static member functions.
 */

/**
 * Returns an object map containing all registered
 * realtime audio i/o object types.
 *
 * All stored objects are derived from AUDIO_IO_DEVICE.
 */
ECA_OBJECT_MAP& ECA_OBJECT_FACTORY::audio_io_rt_map(void) 
{
  //
  // Note! Below we use the Double-Checked Locking Pattern
  //       to protect against concurrent access

  if (audio_io_rt_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (audio_io_rt_map_repp == 0) {
      audio_io_rt_map_repp = new ECA_OBJECT_MAP();
      ECA_STATIC_OBJECT_MAPS::register_audio_io_rt_objects(audio_io_rt_map_repp);
    }
  }
  return(*audio_io_rt_map_repp);
}

/**
 * Returns an object map containing all registered
 * non-realtime audio i/o object types.
 *
 * All stored objects are derived from AUDIO_IO.
 */
ECA_OBJECT_MAP& ECA_OBJECT_FACTORY::audio_io_nonrt_map(void) 
{
  if (audio_io_nonrt_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (audio_io_nonrt_map_repp == 0) {
      audio_io_nonrt_map_repp = new ECA_OBJECT_MAP();
      ECA_STATIC_OBJECT_MAPS::register_audio_io_nonrt_objects(audio_io_nonrt_map_repp);
    }
  }
  return(*audio_io_nonrt_map_repp);
}

/**
 * Returns an object map containing all registered
 * chain operator object types.
 *
 * All stored objects are derived from CHAIN_OPERATOR.
 */
ECA_OBJECT_MAP& ECA_OBJECT_FACTORY::chain_operator_map(void) 
{
  if (chain_operator_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (chain_operator_map_repp == 0) {
      chain_operator_map_repp = new ECA_OBJECT_MAP();
      ECA_STATIC_OBJECT_MAPS::register_chain_operator_objects(chain_operator_map_repp);
    }
  }
  return(*chain_operator_map_repp);
}

/**
 * Returns an object map containing all registered
 * LADSPA plugin types.
 * 
 *
 * All stored objects are derived from EFFECT_LADSPA.
 *
 * @see ladspa_plugin_id_map()
 */
ECA_OBJECT_MAP& ECA_OBJECT_FACTORY::ladspa_plugin_map(void) 
{
  if (ladspa_plugin_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (ladspa_plugin_map_repp == 0) {
      ladspa_plugin_map_repp = new ECA_OBJECT_MAP();
      ECA_STATIC_OBJECT_MAPS::register_ladspa_plugin_objects(ladspa_plugin_map_repp);
    }
  }
  return(*ladspa_plugin_map_repp);
}

/**
 * Returns an object map containing all registered
 * LADSPA plugin types. Plugins are identified using
 * their unique LADSPA id number.
 * 
 * All stored objects are derived from EFFECT_LADSPA.
 *
 * @see ladspa_plugin_map()
 */
ECA_OBJECT_MAP& ECA_OBJECT_FACTORY::ladspa_plugin_id_map(void) 
{
  if (ladspa_plugin_id_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (ladspa_plugin_id_map_repp == 0) {
      ladspa_plugin_id_map_repp = new ECA_OBJECT_MAP();
      ECA_STATIC_OBJECT_MAPS::register_ladspa_plugin_id_objects(ladspa_plugin_id_map_repp);
    }
  }
  return(*ladspa_plugin_id_map_repp);
}

/**
 * Returns an object map containing all registered
 * chain operator preset object types.
 *
 * All stored objects are derived from PRESET.
 */
ECA_PRESET_MAP& ECA_OBJECT_FACTORY::preset_map(void) 
{
  if (preset_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (preset_map_repp == 0) {
      preset_map_repp = new ECA_PRESET_MAP();
      ECA_STATIC_OBJECT_MAPS::register_preset_objects(preset_map_repp);
    }
  }
  return(*preset_map_repp);
}

/**
 * Returns an object map containing all registered
 * controller object types.
 *
 * All stored objects are derived from GENERIC_CONTROLLER.
 */
ECA_OBJECT_MAP& ECA_OBJECT_FACTORY::controller_map(void) 
{
  if (controller_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (controller_map_repp == 0) {
      controller_map_repp = new ECA_OBJECT_MAP();
      ECA_STATIC_OBJECT_MAPS::register_controller_objects(controller_map_repp);
    }
  }
  return(*controller_map_repp);
}

/**
 * Returns an object map containing all registered
 * MIDI-device types.
 *
 *
 * All stored objects are derived from MIDI_IO.
 */
ECA_OBJECT_MAP& ECA_OBJECT_FACTORY::midi_device_map(void) 
{
  if (midi_device_map_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_OBJECT_FACTORY::lock_rep);
    if (midi_device_map_repp == 0) {
      midi_device_map_repp = new ECA_OBJECT_MAP();
      ECA_STATIC_OBJECT_MAPS::register_midi_device_objects(midi_device_map_repp);
    }
  }
  return(*midi_device_map_repp);
}

/**
 * Create a new audio object based on the formatted argument string.
 *
 * @param arg a formatted string describing an audio object, see ecasound 
 *            manuals for detailed info
 * @return the created object or 0 if an invalid format string was given 
 *         as the argument
 *
 * @pre arg.empty() != true
 */
AUDIO_IO* ECA_OBJECT_FACTORY::create_audio_object(const string& arg) {
  // --
  DBC_REQUIRE(arg.empty() != true);
  // --
 
  string fname = kvu_get_argument_number(1, arg);
  if (fname.find(".") != string::npos) {
    fname = string(fname, fname.find_last_of("."), string::npos);
  }

  const AUDIO_IO* main_file = 0;
  main_file = dynamic_cast<const AUDIO_IO*>(ECA_OBJECT_FACTORY::audio_io_rt_map().object_expr(fname));
  if (main_file == 0) {
    main_file = dynamic_cast<const AUDIO_IO*>(ECA_OBJECT_FACTORY::audio_io_nonrt_map().object_expr(fname));
  }

  AUDIO_IO* new_file = 0;
  if (main_file != 0) {
    new_file = main_file->new_expr();
    ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-object-factory) Object \"" + arg + "\" created, type \"" + new_file->name() + "\". Has " + kvu_numtostr(new_file->number_of_params()) + " parameter(s).");
    for(int n = 0; n < new_file->number_of_params(); n++) {
      new_file->set_parameter(n + 1, kvu_get_argument_number(n + 1, arg));
    }
  }
  return(new_file);
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
MIDI_IO* ECA_OBJECT_FACTORY::create_midi_device(const string& arg)
{
  // --------
  DBC_REQUIRE(arg.empty() != true);
  // --------
 
  string fname = kvu_get_argument_number(1, arg);

  const MIDI_IO* device = 0;
  device = dynamic_cast<const MIDI_IO*>(ECA_OBJECT_FACTORY::midi_device_map().object_expr(fname));

  MIDI_IO* new_device = 0;
  if (device != 0) {
    new_device = device->new_expr();
    ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-object-factory) Object \"" + arg + "\" created, type \"" + new_device->name() + "\". Has " + kvu_numtostr(new_device->number_of_params()) + " parameter(s).");
    for(int n = 0; n < new_device->number_of_params(); n++) {
      new_device->set_parameter(n + 1, kvu_get_argument_number(n + 1, arg));
    }
  }
  return(new_device);
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
AUDIO_IO* ECA_OBJECT_FACTORY::create_loop_input(const string& argu,
						 map<int,LOOP_DEVICE*>* loop_map) {
  // --------
  DBC_REQUIRE(argu.empty() != true);
  // --------

  LOOP_DEVICE* p = 0;
  string tname = kvu_get_argument_number(1, argu);
  if (tname.find("loop") != string::npos) {
    int id = atoi(kvu_get_argument_number(2, argu).c_str());
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
AUDIO_IO* ECA_OBJECT_FACTORY::create_loop_output(const string& argu,
						 map<int,LOOP_DEVICE*>* loop_map) {
  // --------
  DBC_REQUIRE(argu.empty() != true);
  // --------

  LOOP_DEVICE* p = 0;
  string tname = kvu_get_argument_number(1, argu);
  if (tname.find("loop") != string::npos) {
    int id = atoi(kvu_get_argument_number(2, argu).c_str());
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
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_ladspa_plugin (const string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

#ifdef HAVE_LADSPA_H
  MESSAGE_ITEM otemp;
  const CHAIN_OPERATOR* cop = 0;
  string prefix = kvu_get_argument_prefix(argu);
  if (prefix == "el" || prefix == "eli") {
    string unique = kvu_get_argument_number(1, argu);
    if (prefix == "el") 
      cop = dynamic_cast<const CHAIN_OPERATOR*>(ECA_OBJECT_FACTORY::ladspa_plugin_map().object(unique));
    else 
      cop = dynamic_cast<const CHAIN_OPERATOR*>(ECA_OBJECT_FACTORY::ladspa_plugin_id_map().object(unique));

    CHAIN_OPERATOR* new_cop = 0;
    if (cop != 0) {
      new_cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());

      ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-object-factory) Creating LADSPA-plugin \"" +
		    new_cop->name() + "\"");
      otemp << "(eca-object-factory) Setting parameters: ";
      for(int n = 0; n < new_cop->number_of_params(); n++) {
	new_cop->set_parameter(n + 1, atof(kvu_get_argument_number(n + 2, argu).c_str()));
	otemp << new_cop->get_parameter_name(n + 1) << " = ";
	otemp << new_cop->get_parameter(n + 1);
	if (n + 1 < new_cop->number_of_params()) otemp << ", ";
      }
      ECA_LOG_MSG(ECA_LOGGER::user_objects, otemp.to_string());
    }
    return(new_cop);
  }
#endif /* HAVE_LADSPA_H */
  return(0);
}

/**
 * VST not currently actively supported due to licensing
 * issues.
 */
#if 0
/**
 * Creates a new VST1.0/2.0 plugin.
 *
 * Notes: VST support is currently not used 
 *        because of licensing problems 
 *        (distribution of VST-headers is not
 *        allowed).
 */
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_vst_plugin (const string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  MESSAGE_ITEM otemp;
  const CHAIN_OPERATOR* cop = 0;
  string prefix = kvu_get_argument_prefix(argu);

  cop = dynamic_cast<const CHAIN_OPERATOR*>(ECA_STATIC_OBJECT_MAPS::vst_plugin_map().object(prefix));
  CHAIN_OPERATOR* new_cop = 0;
  if (cop != 0) {
    
    ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-object-factory) Creating VST-plugin \"" + new_cop->name() + "\"");
    otemp << "(eca-object-factory) Setting parameters: ";
    for(int n = 0; n < new_cop->number_of_params(); n++) {
      new_cop->set_parameter(n + 1, atof(kvu_get_argument_number(n + 1, argu).c_str()));
      otemp << new_cop->get_parameter_name(n + 1) << " = ";
      otemp << new_cop->get_parameter(n + 1);
      if (n + 1 < new_cop->number_of_params()) otemp << ", ";
    }
    ECA_LOG_MSG(ECA_LOGGER::user_objects, otemp.to_string());
  }
  return(new_cop);
}
#endif /* VST ifdef 0 */

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
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_chain_operator (const string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  string prefix = kvu_get_argument_prefix(argu);

  MESSAGE_ITEM otemp;
  const CHAIN_OPERATOR* cop = 
    dynamic_cast<const CHAIN_OPERATOR*>(ECA_OBJECT_FACTORY::chain_operator_map().object(prefix));
  CHAIN_OPERATOR* new_cop = 0;
  if (cop != 0) {
    new_cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());

    ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-object-factory) Creating chain operator \"" +
		  new_cop->name() + "\"");
    //    otemp << "(eca-chainsetup) Adding effect " << new_cop->name();
    otemp << "(eca-object-factory) Setting parameters: ";
    for(int n = 0; n < new_cop->number_of_params(); n++) {
      new_cop->set_parameter(n + 1, atof(kvu_get_argument_number(n + 1, argu).c_str()));
      otemp << new_cop->get_parameter_name(n + 1) << " = ";
      otemp << new_cop->get_parameter(n +1);
      if (n + 1 < new_cop->number_of_params()) otemp << ", ";
    }
    ECA_LOG_MSG(ECA_LOGGER::user_objects, otemp.to_string());
    return(new_cop);
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
GENERIC_CONTROLLER* ECA_OBJECT_FACTORY::create_controller (const string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() > 0 && argu[0] != '-') return(0);
  string prefix = kvu_get_argument_prefix(argu);

  const GENERIC_CONTROLLER* gcontroller = 
    dynamic_cast<const GENERIC_CONTROLLER*>(ECA_OBJECT_FACTORY::controller_map().object(prefix));
  GENERIC_CONTROLLER* new_gcontroller = 0;
  if (gcontroller != 0) {
    new_gcontroller = gcontroller->new_expr();
    if (new_gcontroller != 0) {
      const CONTROLLER_SOURCE* csource = gcontroller->source_pointer();
      CONTROLLER_SOURCE* new_csource = 0;
      if (csource != 0) {
	new_csource = csource->new_expr();
      }
      new_gcontroller->assign_source(new_csource);

      ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-object-factory) Creating controller source \"" +  new_gcontroller->name() + "\"");

      MESSAGE_ITEM otemp;
      otemp << "(eca-object-factory) Setting parameters: ";
      int numparams = new_gcontroller->number_of_params();
      for(int n = 0; n < numparams; n++) {
	new_gcontroller->set_parameter(n + 1, atof(kvu_get_argument_number(n + 1, argu).c_str()));
	otemp << new_gcontroller->get_parameter_name(n + 1) << " = ";
	otemp << new_gcontroller->get_parameter(n +1);
	numparams = new_gcontroller->number_of_params(); // in case 'n_o_p()' varies
	if (n + 1 < numparams) otemp << ", ";
      }
      ECA_LOG_MSG(ECA_LOGGER::user_objects, otemp.to_string());

      return(new_gcontroller);
    }
  }
  return(0);
}
