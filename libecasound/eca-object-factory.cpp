// ------------------------------------------------------------------------
// eca-object-factory.cpp: Class for creating various ecasound objects.
// Copyright (C) 2000,2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <list>
#include <map>
#include <string>

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
#include "eca-preset-map.h"
#include "eca-object-factory.h"
#include "eca-debug.h"

using std::list;
using std::map;
using std::string;

/**
 * Reserves object factory for use. Must be called before 
 * using any other object factory services. When 
 * the first factory client calls reserve_factory(), 
 * all required resources are allocated. Reference 
 * counting is used to track down individual uses of 
 * the factory.
 * 
 * @see free_factory()
 */
void ECA_OBJECT_FACTORY::reserve_factory(void) {
  ECA_STATIC_OBJECT_MAPS::register_default_objects();
}

/**
 * Free the object factory. Must be called when 
 * factory is not used anymore. Once the last 
 * independent factory client issues free_factory(), 
 * all resources are freed.
 * 
 * @see reserve_factory()
 */
void ECA_OBJECT_FACTORY::free_factory(void) {
  ECA_STATIC_OBJECT_MAPS::unregister_default_objects();
}

string ECA_OBJECT_FACTORY::object_identifier(const ECA_OBJECT* obj) {
  string result;

  if (ECA_STATIC_OBJECT_MAPS::audio_object_map()->has_object(obj) == true) {
    result = ECA_STATIC_OBJECT_MAPS::audio_object_map()->object_identifier(obj);
  }
  else if (ECA_STATIC_OBJECT_MAPS::chain_operator_map()->has_object(obj) == true) {
    result = ECA_STATIC_OBJECT_MAPS::chain_operator_map()->object_identifier(obj);
  }
  else if (ECA_STATIC_OBJECT_MAPS::ladspa_plugin_map()->has_object(obj) == true) {
    result = ECA_STATIC_OBJECT_MAPS::ladspa_plugin_map()->object_identifier(obj);
  }
  else if (ECA_STATIC_OBJECT_MAPS::ladspa_plugin_id_map()->has_object(obj) == true) {
    result = ECA_STATIC_OBJECT_MAPS::ladspa_plugin_id_map()->object_identifier(obj);
  }
  else if (ECA_STATIC_OBJECT_MAPS::controller_map()->has_object(obj) == true) {
    result = ECA_STATIC_OBJECT_MAPS::controller_map()->object_identifier(obj);
  }
  else if (ECA_STATIC_OBJECT_MAPS::midi_device_map()->has_object(obj) == true) {
    result = ECA_STATIC_OBJECT_MAPS::midi_device_map()->object_identifier(obj);
  }
  else if (ECA_STATIC_OBJECT_MAPS::preset_map()->has_object(obj) == true) {
    result = ECA_STATIC_OBJECT_MAPS::preset_map()->object_identifier(obj);
  }

  return(result);
}

const list<string>& ECA_OBJECT_FACTORY::audio_io_list(void)
{
  return(ECA_STATIC_OBJECT_MAPS::audio_object_map()->registered_objects());
}

const list<string>& ECA_OBJECT_FACTORY::chain_operator_list(void) 
{
  return(ECA_STATIC_OBJECT_MAPS::chain_operator_map()->registered_objects());
}

const list<string>& ECA_OBJECT_FACTORY::preset_list(void)
{
  return(ECA_STATIC_OBJECT_MAPS::preset_map()->registered_objects());
}

const list<string>& ECA_OBJECT_FACTORY::ladspa_list(void)
{
  return(ECA_STATIC_OBJECT_MAPS::ladspa_plugin_map()->registered_objects());
}

const list<string>& ECA_OBJECT_FACTORY::controller_list(void)
{
  return(ECA_STATIC_OBJECT_MAPS::controller_map()->registered_objects());
}

/**
 * Returns the first audio object that matches with 'keyword'.
 */
const AUDIO_IO* ECA_OBJECT_FACTORY::audio_io_map_object(const string& keyword) 
{
  return(dynamic_cast<const AUDIO_IO*>(ECA_STATIC_OBJECT_MAPS::audio_object_map()->object_expr(keyword)));
}

/**
 * Returns the first effect that matches with 'keyword'
 */
const CHAIN_OPERATOR* ECA_OBJECT_FACTORY::chain_operator_map_object(const string& keyword) {
  return(dynamic_cast<const CHAIN_OPERATOR*>(ECA_STATIC_OBJECT_MAPS::chain_operator_map()->object_expr(keyword)));
}

/**
 * Returns the first effect preset that matches with 'keyword'
 */
const PRESET* ECA_OBJECT_FACTORY::preset_object(const string& keyword)
{
  return(dynamic_cast<const PRESET*>(ECA_STATIC_OBJECT_MAPS::preset_map()->object_expr(keyword)));
}

/**
 * Return the first LADSPA effect that matches with 'keyword'
 */
const EFFECT_LADSPA* ECA_OBJECT_FACTORY::ladspa_map_object(const string& keyword) { 
  return(dynamic_cast<const EFFECT_LADSPA*>(ECA_STATIC_OBJECT_MAPS::ladspa_plugin_map()->object_expr(keyword))); 
}

/**
 * Return the first LADSPA effect that matches with 'number'
 */
const EFFECT_LADSPA* ECA_OBJECT_FACTORY::ladspa_map_object(long int number) { 
  return(dynamic_cast<const EFFECT_LADSPA*>(ECA_STATIC_OBJECT_MAPS::ladspa_plugin_id_map()->object_expr(kvu_numtostr(number)))); 
}

/**
 * Return the first controller object that matches with 'keyword'
 */
const GENERIC_CONTROLLER* ECA_OBJECT_FACTORY::controller_map_object(const string& keyword) {
  return(dynamic_cast<const GENERIC_CONTROLLER*>(ECA_STATIC_OBJECT_MAPS::controller_map()->object_expr(keyword)));
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
AUDIO_IO* ECA_OBJECT_FACTORY::create_audio_object(const string& arg) {
  // --
  DBC_REQUIRE(arg.empty() != true);
  // --
 
  string fname = get_argument_number(1, arg);
  if (fname.find(".") != string::npos) {
    fname = string(fname, fname.find_last_of("."), string::npos);
  }

  const AUDIO_IO* main_file = 0;
  main_file = ECA_OBJECT_FACTORY::audio_io_map_object(fname);

  AUDIO_IO* new_file = 0;
  if (main_file != 0) {
    new_file = main_file->new_expr();
    ecadebug->msg(ECA_DEBUG::user_objects, "(eca-object-factory) Object \"" + arg + "\" created, type \"" + new_file->name() + "\". Has " + kvu_numtostr(new_file->number_of_params()) + " parameter(s).");
    for(int n = 0; n < new_file->number_of_params(); n++) {
      new_file->set_parameter(n + 1, get_argument_number(n + 1, arg));
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
MIDI_IO* ECA_OBJECT_FACTORY::create_midi_device(const string& arg) {
  // --------
  DBC_REQUIRE(arg.empty() != true);
  // --------
 
  ECA_STATIC_OBJECT_MAPS::register_default_objects();
  string fname = get_argument_number(1, arg);

  const MIDI_IO* device = 0;
  device = dynamic_cast<const MIDI_IO*>(ECA_STATIC_OBJECT_MAPS::midi_device_map()->object_expr(fname));

  MIDI_IO* new_device = 0;
  if (device != 0) {
    new_device = device->new_expr();
    ecadebug->msg(ECA_DEBUG::user_objects, "(eca-object-factory) Object \"" + arg + "\" created, type \"" + new_device->name() + "\". Has " + kvu_numtostr(new_device->number_of_params()) + " parameter(s).");
    for(int n = 0; n < new_device->number_of_params(); n++) {
      new_device->set_parameter(n + 1, get_argument_number(n + 1, arg));
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
AUDIO_IO* ECA_OBJECT_FACTORY::create_loop_output(const string& argu,
						 map<int,LOOP_DEVICE*>* loop_map) {
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
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_ladspa_plugin (const string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

#ifdef HAVE_LADSPA_H
  MESSAGE_ITEM otemp;
  const CHAIN_OPERATOR* cop = 0;
  string prefix = get_argument_prefix(argu);
  if (prefix == "el" || prefix == "eli") {
    string unique = get_argument_number(1, argu);
    if (prefix == "el") 
      cop = ECA_OBJECT_FACTORY::ladspa_map_object(unique);
    else 
      cop = ECA_OBJECT_FACTORY::ladspa_map_object(atol(unique.c_str()));

    CHAIN_OPERATOR* new_cop = 0;
    if (cop != 0) {
      new_cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());

      ecadebug->msg(ECA_DEBUG::user_objects, "(eca-object-factory) Creating LADSPA-plugin \"" +
		    new_cop->name() + "\"");
      otemp << "(eca-object-factory) Setting parameters: ";
      for(int n = 0; n < new_cop->number_of_params(); n++) {
	new_cop->set_parameter(n + 1, atof(get_argument_number(n + 2, argu).c_str()));
	otemp << new_cop->get_parameter_name(n + 1) << " = ";
	otemp << new_cop->get_parameter(n + 1);
	if (n + 1 < new_cop->number_of_params()) otemp << ", ";
      }
      ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
    }
    return(new_cop);
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
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_vst_plugin (const string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  MESSAGE_ITEM otemp;
  const CHAIN_OPERATOR* cop = 0;
  string prefix = get_argument_prefix(argu);

#ifdef FEELING_EXPERIMENTAL
  cop = dynamic_cast<const CHAIN_OPERATOR*>(ECA_STATIC_OBJECT_MAPS::vst_plugin_map().object(prefix));
#endif
  CHAIN_OPERATOR* new_cop = 0;
  if (cop != 0) {
    
    ecadebug->msg(ECA_DEBUG::user_objects, "(eca-object-factory) Creating VST-plugin \"" + new_cop->name() + "\"");
    otemp << "(eca-object-factory) Setting parameters: ";
    for(int n = 0; n < new_cop->number_of_params(); n++) {
      new_cop->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << new_cop->get_parameter_name(n + 1) << " = ";
      otemp << new_cop->get_parameter(n + 1);
      if (n + 1 < new_cop->number_of_params()) otemp << ", ";
    }
    ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
  }
  return(new_cop);
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
CHAIN_OPERATOR* ECA_OBJECT_FACTORY::create_chain_operator (const string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  string prefix = get_argument_prefix(argu);

  MESSAGE_ITEM otemp;
  const CHAIN_OPERATOR* cop = ECA_OBJECT_FACTORY::chain_operator_map_object(prefix);
  CHAIN_OPERATOR* new_cop = 0;
  if (cop != 0) {
    new_cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());

    ecadebug->msg(ECA_DEBUG::user_objects, "(eca-object-factory) Creating chain operator \"" +
		  new_cop->name() + "\"");
    //    otemp << "(eca-chainsetup) Adding effect " << new_cop->name();
    otemp << "(eca-object-factory) Setting parameters: ";
    for(int n = 0; n < new_cop->number_of_params(); n++) {
      new_cop->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << new_cop->get_parameter_name(n + 1) << " = ";
      otemp << new_cop->get_parameter(n +1);
      if (n + 1 < new_cop->number_of_params()) otemp << ", ";
    }
    ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
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
  string prefix = get_argument_prefix(argu);

  const GENERIC_CONTROLLER* gcontroller = ECA_OBJECT_FACTORY::controller_map_object(prefix);
  GENERIC_CONTROLLER* new_gcontroller = 0;
  if (gcontroller != 0) {
    new_gcontroller = gcontroller->clone();

    ecadebug->msg(ECA_DEBUG::user_objects, "(eca-object-factory) Creating controller source \"" +  new_gcontroller->name() + "\"");

    MESSAGE_ITEM otemp;
    otemp << "(eca-object-factory) Setting parameters: ";
    int numparams = new_gcontroller->number_of_params();
    for(int n = 0; n < numparams; n++) {
      new_gcontroller->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << new_gcontroller->get_parameter_name(n + 1) << " = ";
      otemp << new_gcontroller->get_parameter(n +1);
      numparams = new_gcontroller->number_of_params(); // in case 'n_o_p()' varies
      if (n + 1 < numparams) otemp << ", ";
    }
    ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
    return(new_gcontroller);
  }
  return(0);
}

const ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::audio_io_map(void) 
{
  return(ECA_STATIC_OBJECT_MAPS::audio_object_map());
}

const ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::chain_operator_map(void)
{
  return(ECA_STATIC_OBJECT_MAPS::chain_operator_map());
}

const ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::ladspa_map(void)
{
  return(ECA_STATIC_OBJECT_MAPS::ladspa_plugin_map());
}

const ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::preset_map(void)
{
  return(ECA_STATIC_OBJECT_MAPS::preset_map());
}

const ECA_OBJECT_MAP* ECA_OBJECT_FACTORY::controller_map(void)
{
  return(ECA_STATIC_OBJECT_MAPS::controller_map());
}

void ECA_OBJECT_FACTORY::register_chain_operator(const string& keyword, const string& expr, CHAIN_OPERATOR* object)
{
  ECA_STATIC_OBJECT_MAPS::chain_operator_map()->register_object(keyword, expr, static_cast<ECA_OBJECT*>(object));
}

void ECA_OBJECT_FACTORY::register_controller(const string& keyword, const string& expr, GENERIC_CONTROLLER* object)
{
  ECA_STATIC_OBJECT_MAPS::controller_map()->register_object(keyword, expr, static_cast<ECA_OBJECT*>(object));
}
