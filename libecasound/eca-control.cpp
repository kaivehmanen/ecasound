// ------------------------------------------------------------------------
// eca-control.cpp: Class for controlling the whole ecasound library
// Copyright (C) 1999-2004 Kai Vehmanen
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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <unistd.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <kvu_utils.h> /* string_to_vector(), string_to_int_vector() */
#include <kvu_value_queue.h>
#include <kvu_message_item.h>
#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#include "eca-control.h"
#include "eca-chainop.h"
#include "eca-chainsetup.h"
#include "eca-object-factory.h"
#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-session.h"

#include "generic-controller.h"
#include "eca-chainop.h"
#include "audiofx_ladspa.h"
#include "preset.h"
#include "sample-specs.h"

#include "eca-version.h"
#include "eca-error.h"
#include "eca-logger.h"
#include "eca-logger-wellformed.h"

/**
 * Import namespaces
 */
using std::string;
using std::list;
using std::vector;
using std::cerr;
using std::endl;

/**
 * Declarations for private static functions
 */
static string eca_aio_register_sub(ECA_OBJECT_MAP& objmap);

/**
 * Definitions for member functions
 */

ECA_CONTROL::ECA_CONTROL (ECA_SESSION* psession) 
  : ECA_CONTROL_OBJECTS(psession),
    wellformed_mode_rep(false),
    ctrl_dump_rep(this)
{
}

ECA_CONTROL::~ECA_CONTROL(void)
{
}

void ECA_CONTROL::command(const string& cmd)
{
  clear_last_values();
  clear_action_arguments();
  vector<string> cmds = kvu_string_to_tokens_quoted(cmd);
  vector<string>::iterator p = cmds.begin();
  if (p != cmds.end()) {

    const std::map<std::string,int>& cmdmap = ECA_IAMODE_PARSER::registered_commands();
    if (cmdmap.find(*p) == cmdmap.end()) {
      // ---
      // *p is not recognized as a iamode command
      // ---
      if (p->size() > 0 && (*p)[0] == '-') {
	//  std::cerr << "Note! Direct use of EOS-options (-prefix:arg1,...,argN)" << " as iactive-mode commands is considered deprecated. " << "\tUse the notation 'cs-option -prefix:a,b,x' instead." << std::endl;
	chainsetup_option(cmd);
      }
      else {
	set_last_error("Unknown command!");
      }
    }
    else {
      int action_id = ECA_IAMODE_PARSER::command_to_action_id(*p);
      if (action_id == ec_help) {
	show_controller_help();
      }
      else {
	string first = *p;
	++p;
	if (p != cmds.end()) {
	  set_action_argument(kvu_vector_to_string(vector<string> (p, cmds.end()), " "));
	}
	action(action_id);
      }
    }
  }
}

void ECA_CONTROL::set_action_argument(const string& s)
{
  action_args_rep = s;
  action_arg_f_set_rep = false;
}

void ECA_CONTROL::set_action_argument(double v)
{
  action_arg_f_rep = v;
  action_arg_f_set_rep = true;
}

void ECA_CONTROL::clear_action_arguments(void)
{
  action_args_rep.clear();
  action_arg_f_rep = 0.0f;
  action_arg_f_set_rep = false;
}

double ECA_CONTROL::first_action_argument_as_float(void) const
{
  if (action_arg_f_set_rep == true)
    return action_arg_f_rep;

  return atof(action_args_rep.c_str());
}

long int ECA_CONTROL::first_action_argument_as_long_int(void) const
{
  return atol(action_args_rep.c_str());
}

SAMPLE_SPECS::sample_pos_t ECA_CONTROL::first_action_argument_as_samples(void) const
{
#ifdef HAVE_ATOLL
  return atoll(action_args_rep.c_str());
#else
  return atol(action_args_rep.c_str());
#endif
}

void ECA_CONTROL::command_float_arg(const string& cmd, double arg)
{
  clear_action_arguments();
  set_action_argument(arg);
  int action_id = ec_unknown;
  action_id = ECA_IAMODE_PARSER::command_to_action_id(cmd);
  action(action_id);
}

/**
 * Interprets an EOS (ecasound optiont syntax) token  (prefixed with '-').
 */
void ECA_CONTROL::chainsetup_option(const string& cmd)
{
  string prefix = kvu_get_argument_prefix(cmd);
  if (prefix == "el" || prefix == "pn") { // --- LADSPA plugins and presets
    if (selected_chains().size() == 1) 
      add_chain_operator(cmd);
    else
      set_last_error("When adding chain operators, only one chain can be selected.");
  }
  else if (ECA_OBJECT_FACTORY::chain_operator_map().object(prefix) != 0) {
    if (selected_chains().size() == 1) 
      add_chain_operator(cmd);
    else
      set_last_error("When adding chain operators, only one chain can be selected.");
  }
  else if (ECA_OBJECT_FACTORY::controller_map().object(prefix) != 0) {
    if (selected_chains().size() == 1) 
      add_controller(cmd);
    else
      set_last_error("When adding controllers, only one chain can be selected.");
  }
  else {
    set_action_argument(cmd);
    action(ec_cs_option);
  }
}

/**
 * Checks action preconditions.
 *
 * @return Sets status of private data members 'action_ok', 
 *         'action_restart', and 'action_reconnect'.
 */
void ECA_CONTROL::check_action_preconditions(int action_id)
{
  action_ok = true;
  action_reconnect = false;
  action_restart = false;

  /* case 1: action requiring arguments, but not arguments available */
  if (action_arg_f_set_rep == false && 
      action_args_rep.size() == 0 &&
      action_requires_params(action_id)) {
    set_last_error("Can't perform requested action; argument omitted.");
    action_ok = false;
  }
  /* case 2: action requires an audio input, but no input available */
  else if (is_selected() == true &&
	   get_audio_input() == 0 &&
	   action_requires_selected_audio_input(action_id)) {
    set_last_error("Can't perform requested action; no audio input selected.");
    action_ok = false;
  }
  /* case 3: action requires an audio output, but no output available */
  else if (is_selected() == true &&
	   get_audio_output() == 0 &&
	   action_requires_selected_audio_output(action_id)) {
    set_last_error("Can't perform requested action; no audio output selected.");
    action_ok = false;
  }
  /* case 4: action requires a select chainsetup, but none selected */
  else if (is_selected() == false &&
	   action_requires_selected(action_id)) {
    if (!is_connected()) {
      set_last_error("Can't perform requested action; no chainsetup selected.");
      action_ok = false;
    }
    else {
      set_last_error("Warning! No chainsetup selected. Connected chainsetup will be selected.");
      select_chainsetup(connected_chainsetup());
    }
  }
  /* case 5: action requires a connected chainsetup, but none connected */
  else if (is_connected() == false &&
      action_requires_connected(action_id)) {
    if (!is_selected()) {
      set_last_error("Can't perform requested action; no chainsetup connected.");
      action_ok = false;
    }
    else {
      set_last_error("Warning! No chainsetup connected. Trying to connect currently selected chainsetup.");
      if (is_valid() == true) {
	connect_chainsetup();
      }
      if (is_connected() != true) {
	/* connect_chainsetup() sets last_error() so we just add to it */
	set_last_error(last_error() + "\nWarning! Selected chainsetup cannot be connected. Can't perform requested action. ");
	action_ok = false;
      }
    }
  }
  /* case 6: action can't be performed on a connected setup, 
   *         but selected chainsetup is also connected */
  else if (selected_chainsetup() == connected_chainsetup() &&
	   action_requires_selected_not_connected(action_id)) {
    set_last_error("Warning! This operation requires that chainsetup is disconnected. Temporarily disconnecting...");
    if (is_running()) action_restart = true;
    disconnect_chainsetup();
    action_reconnect = true;
  }
}

void ECA_CONTROL::action(int action_id, 
			 const vector<string>& args) 
{
  ECA_LOG_MSG(ECA_LOGGER::info, "Warning: ECA_CONTROL::action() method is obsolete.\n");
  clear_action_arguments();
  set_action_argument(kvu_vector_to_string(args, " "));
  action(action_id);
}

void ECA_CONTROL::action(int action_id)
{
  clear_last_values();
  check_action_preconditions(action_id);

  if (action_ok != true) return;

  switch(action_id) {
  case ec_unknown: { set_last_error("Unknown command!"); break; }

    // ---
    // General
    // ---
  case ec_exit: { quit(); break; }
  case ec_start: 
    { 
      if (is_running() != true) start();
      // ECA_LOG_MSG(ECA_LOGGER::info, "(eca-control) Can't perform requested action; no chainsetup connected.");
      break; 
    }
  case ec_stop: { if (is_running() == true) stop(); break; }
  case ec_run: { run(); break; }
  case ec_debug:
    {
      int level = atoi(action_args_rep.c_str());
      ECA_LOGGER::instance().set_log_level_bitmask(level);
      set_last_string("Debug level set to " + kvu_numtostr(level) + ".");
      break;
    }

    // ---
    // Chainsetups
    // ---
  case ec_cs_add:
    {
      add_chainsetup(action_args_rep);
      break;
    }
  case ec_cs_remove: { remove_chainsetup(); break; }
  case ec_cs_list: { set_last_string_list(chainsetup_names()); break; }
  case ec_cs_select: { select_chainsetup(action_args_rep); break; }
  case ec_cs_selected: { set_last_string(selected_chainsetup()); break; }
  case ec_cs_index_select: { 
    if (action_args_rep.size() > 0) {
      select_chainsetup_by_index(atoi(action_args_rep.c_str()));
    }
    break; 
  }
  case ec_cs_edit: { edit_chainsetup(); break; }
  case ec_cs_load: { load_chainsetup(action_args_rep); break; }
  case ec_cs_save: { save_chainsetup(""); break; }
  case ec_cs_save_as: { save_chainsetup(action_args_rep); break; }
  case ec_cs_is_valid: { 
    if (is_valid() == true) 
      set_last_integer(1);
    else
      set_last_integer(0);
    break;
  }
  case ec_cs_connect: 
    { 
      if (is_valid() != false) {
	connect_chainsetup(); 
      }
      else {
	set_last_error("Can't connect; chainsetup not valid!");
      }
      break; 
    }
  case ec_cs_connected: { set_last_string(connected_chainsetup()); break; }
  case ec_cs_disconnect: { disconnect_chainsetup(); break; }
  case ec_cs_set_param: { set_chainsetup_parameter(action_args_rep); break; }
  case ec_cs_set_audio_format: { set_chainsetup_sample_format(action_args_rep); break; }
  case ec_cs_status: { 
    set_last_string(chainsetup_status()); 
    break; 
  }
  case ec_cs_rewind: { change_chainsetup_position(-first_action_argument_as_float()); break; }
  case ec_cs_forward: { change_chainsetup_position(first_action_argument_as_float()); break; }
  case ec_cs_set_position: { set_chainsetup_position(first_action_argument_as_float()); break; }
  case ec_cs_set_position_samples: { set_chainsetup_position_samples(first_action_argument_as_samples()); break; }
  case ec_cs_get_position: { set_last_float(position_in_seconds_exact()); break; }
  case ec_cs_get_position_samples: { set_last_long_integer(selected_chainsetup_repp->position_in_samples()); break; }
  case ec_cs_get_length: { set_last_float(length_in_seconds_exact()); break; }
  case ec_cs_get_length_samples: { set_last_long_integer(length_in_samples()); break; }
  case ec_cs_set_length: 
    { 
      set_chainsetup_processing_length_in_seconds(first_action_argument_as_float()); 
      break; 
    }
  case ec_cs_set_length_samples:
    {
      set_chainsetup_processing_length_in_samples(first_action_argument_as_samples());
      break;
    }
  case ec_cs_toggle_loop: { toggle_chainsetup_looping(); break; } 
  case ec_cs_option: 
    {
      vector<string> temp = kvu_string_to_tokens_quoted(action_args_rep);
      selected_chainsetup_repp->interpret_options(temp);
      if (selected_chainsetup_repp->interpret_result() != true) {
	set_last_error(selected_chainsetup_repp->interpret_result_verbose());
      }
      break;
    }

  // ---
  // Chains
  // ---
  case ec_c_add: { add_chains(kvu_string_to_vector(action_args_rep, ',')); break; }
  case ec_c_remove: { remove_chains(); break; }
  case ec_c_list: { set_last_string_list(chain_names()); break; }
  case ec_c_select: { select_chains(kvu_string_to_vector(action_args_rep, ',')); break; }
  case ec_c_selected: { set_last_string_list(selected_chains()); break; }
  case ec_c_index_select: { select_chains_by_index(kvu_string_to_int_vector(action_args_rep, ',')); break; }
  case ec_c_deselect: { deselect_chains(kvu_string_to_vector(action_args_rep, ',')); break; }
  case ec_c_select_add: 
    { 
      select_chains(kvu_string_to_vector(action_args_rep + "," +
					 kvu_vector_to_string(selected_chains(), ","), ',')); 
      break; 
    }
  case ec_c_select_all: { select_all_chains(); break; }
  case ec_c_clear: { clear_chains(); break; }
  case ec_c_rename: 
    { 
      if (selected_chains().size() != 1) {
	set_last_error("When renaming chains, only one chain canbe selected.");
      }
      else {
	rename_chain(action_args_rep); 
      }
      break;
    }
  case ec_c_muting: { toggle_chain_muting(); break; }
  case ec_c_bypass: { toggle_chain_bypass(); break; }
  case ec_c_status: 
    { 
      set_last_string(chain_status()); 
      break; 
    }

    // ---
    // Actions common to audio inputs and outputs
    // ---
  case ec_aio_status:
  case ec_ai_status:
  case ec_ao_status:
    { 
      set_last_string(aio_status()); 
      break; 
    }
  case ec_aio_register: { aio_register(); break; }

    // ---
    // Audio input objects
    // ---
  case ec_ai_add: { add_audio_input(action_args_rep); break; }
  case ec_ai_remove: { remove_audio_input(); break; }
  case ec_ai_list: { set_last_string_list(audio_input_names()); break; }
  case ec_ai_select: { select_audio_input(action_args_rep); break; }
  case ec_ai_selected: { set_last_string(get_audio_input()->label()); break; }
  case ec_ai_index_select: { 
    if (action_args_rep.size() > 0) {
	select_audio_input_by_index(atoi(action_args_rep.c_str()));
    }
    break; 
  }
  case ec_ai_attach: { attach_audio_input(); break; }
  case ec_ai_forward: 
    { 
      audio_input_as_selected();
      forward_audio_object(first_action_argument_as_float()); 
      break; 
    }
  case ec_ai_rewind: 
    { 
      audio_input_as_selected();
      rewind_audio_object(first_action_argument_as_float()); 
      break; 
    }
  case ec_ai_set_position: { audio_input_as_selected(); set_audio_object_position(first_action_argument_as_float()); break; }
  case ec_ai_set_position_samples: { audio_input_as_selected(); set_audio_object_position_samples(first_action_argument_as_long_int()); break; }
  case ec_ai_get_position: { set_last_float(get_audio_input()->position().seconds()); break; }
  case ec_ai_get_position_samples: { set_last_long_integer(get_audio_input()->position().samples()); break; }
  case ec_ai_get_length: { set_last_float(get_audio_input()->length().seconds()); break; }
  case ec_ai_get_length_samples: { set_last_long_integer(get_audio_input()->length().samples()); break; }
  case ec_ai_get_format: {
    set_last_string(get_audio_input()->format_string() + "," +
		    kvu_numtostr(get_audio_input()->channels()) + "," +
		    kvu_numtostr(get_audio_input()->samples_per_second())); 

    break; 
  }

  case ec_ai_wave_edit: { audio_input_as_selected(); wave_edit_audio_object(); break; }

    // ---
    // Audio output objects
    // ---
  case ec_ao_add: { if (action_args_rep.size() == 0) add_default_output(); else add_audio_output(action_args_rep); break; }
  case ec_ao_add_default: { add_default_output(); break; }
  case ec_ao_remove: { remove_audio_output(); break; }
  case ec_ao_list: { set_last_string_list(audio_output_names()); break; }
  case ec_ao_select: { select_audio_output(action_args_rep); break; }
  case ec_ao_selected: { set_last_string(get_audio_output()->label()); break; }
  case ec_ao_index_select: { 
    if (action_args_rep.size() > 0) {
      select_audio_output_by_index(atoi(action_args_rep.c_str()));
    }
    break; 
  }
  case ec_ao_attach: { attach_audio_output(); break; }
  case ec_ao_forward: 
    { 
      audio_output_as_selected();
      forward_audio_object(first_action_argument_as_float()); 
      break; 
    }
  case ec_ao_rewind: 
    { 
      audio_output_as_selected();
      rewind_audio_object(first_action_argument_as_float()); 
      break; 
    }
  case ec_ao_set_position: { audio_output_as_selected(); set_audio_object_position(first_action_argument_as_float()); break; }
  case ec_ao_set_position_samples: { audio_output_as_selected(); set_audio_object_position_samples(first_action_argument_as_long_int()); break; }
  case ec_ao_get_position: { set_last_float(get_audio_output()->position().seconds()); break; }
  case ec_ao_get_position_samples: { set_last_long_integer(get_audio_output()->position().samples()); break; }
  case ec_ao_get_length: { set_last_float(get_audio_output()->length().seconds()); break; }
  case ec_ao_get_length_samples: { set_last_long_integer(get_audio_output()->length().samples()); break; }
  case ec_ao_get_format: { 
    set_last_string(get_audio_output()->format_string() + "," +
		    kvu_numtostr(get_audio_output()->channels()) + "," +
		    kvu_numtostr(get_audio_output()->samples_per_second())); 
    break; 
  }
  case ec_ao_wave_edit: { audio_output_as_selected(); wave_edit_audio_object(); break; }

    // ---
    // Chain operators
    // ---
  case ec_cop_add: { add_chain_operator(action_args_rep); break; }
  case ec_cop_remove: { remove_chain_operator(); break; }
  case ec_cop_list: { set_last_string_list(chain_operator_names()); break; }
  case ec_cop_select: { select_chain_operator(atoi((action_args_rep).c_str())); break; }
  case ec_cop_selected: { set_last_integer(selected_chain_operator()); break; }
  case ec_cop_set: 
    { 
      vector<string> a = kvu_string_to_vector(action_args_rep, ',');
      if (a.size() < 3) {
	set_last_error("Not enough parameters!");
	break;
      }
      int id1 = atoi(a[0].c_str());
      int id2 = atoi(a[1].c_str());
      CHAIN_OPERATOR::parameter_t v = atof(a[2].c_str());

      if (id1 > 0 && id2 > 0) {
	select_chain_operator(id1);
	select_chain_operator_parameter(id2);
	set_chain_operator_parameter(v);
      }
      else
	set_last_error("Chain operator indexing starts from 1.");
      break; 
    }
  case ec_cop_status: 
    { 
      set_last_string(chain_operator_status()); 
      break; 
    }

    // ---
    // Chain operator parameters
    // ---
  case ec_copp_list: { set_last_string_list(chain_operator_parameter_names()); break; }
  case ec_copp_select: { select_chain_operator_parameter(atoi((action_args_rep).c_str())); break; }
  case ec_copp_selected: { set_last_integer(selected_chain_operator_parameter()); break; }
  case ec_copp_set: { set_chain_operator_parameter(first_action_argument_as_float()); break; }
  case ec_copp_get: { set_last_float(get_chain_operator_parameter()); break; }

    // ---
    // Controllers
    // ---
  case ec_ctrl_add: { add_controller(action_args_rep); break; }
  case ec_ctrl_remove: { remove_controller(); break; }
  case ec_ctrl_list: { set_last_string_list(controller_names()); break; }
  case ec_ctrl_select: { select_controller(atoi((action_args_rep).c_str())); break; }
  case ec_ctrl_selected: { set_last_integer(selected_controller()); break; }
  case ec_ctrl_status: 
    { 
      set_last_string(controller_status()); 
      break; 
    }

  case ec_cop_register: { cop_register(); break; }
  case ec_preset_register: { preset_register(); break; }
  case ec_ladspa_register: { ladspa_register(); break; }
  case ec_ctrl_register: { ctrl_register(); break; }

  case ec_map_cop_list: { cop_descriptions(); break; }
  case ec_map_preset_list: { preset_descriptions(); break; }
  case ec_map_ladspa_list: { ladspa_descriptions(false); break; }
  case ec_map_ladspa_id_list: { ladspa_descriptions(true); break; }
  case ec_map_ctrl_list: { ctrl_descriptions(); break; }

  // ---
  // Engine commands
  // ---
  case ec_engine_launch: { if (is_engine_started() != true) engine_start(); break; }
  case ec_engine_halt: { close_engine(); break; }
  case ec_engine_status: { set_last_string(engine_status()); break; }

  // ---
  // Internal commands
  // ---
  case ec_int_cmd_list: { set_last_string_list(registered_commands_list()); break; }
  case ec_int_output_mode_wellformed: { 
    ECA_LOGGER::attach_logger(new ECA_LOGGER_WELLFORMED()); 
    wellformed_mode_rep = true;
    break; 
  }
  case ec_int_set_float_to_string_precision: { set_float_to_string_precision(atoi((action_args_rep).c_str())); break; }
  case ec_int_version_string: { set_last_string(ecasound_library_version); break; }
  case ec_int_version_lib_current: { set_last_integer(ecasound_library_version_current); break; }
  case ec_int_version_lib_revision: { set_last_integer(ecasound_library_version_revision); break; }
  case ec_int_version_lib_age: { set_last_integer(ecasound_library_version_age); break; }

  // ---
  // Dump commands
  // ---
  case ec_dump_target: { ctrl_dump_rep.set_dump_target(action_args_rep); break; }
  case ec_dump_status: { ctrl_dump_rep.dump_status(); break; }
  case ec_dump_position: { ctrl_dump_rep.dump_position(); break; }
  case ec_dump_length: { ctrl_dump_rep.dump_length(); break; }
  case ec_dump_cs_status: { ctrl_dump_rep.dump_chainsetup_status(); break; }
  case ec_dump_c_selected: { ctrl_dump_rep.dump_selected_chain(); break; }
  case ec_dump_ai_selected: { ctrl_dump_rep.dump_selected_audio_input(); break; }
  case ec_dump_ai_position: { ctrl_dump_rep.dump_audio_input_position(); break; }
  case ec_dump_ai_length: { ctrl_dump_rep.dump_audio_input_length(); break; }
  case ec_dump_ai_open_state: { ctrl_dump_rep.dump_audio_input_open_state(); break; }
  case ec_dump_ao_selected: { ctrl_dump_rep.dump_selected_audio_output(); break; }
  case ec_dump_ao_position: { ctrl_dump_rep.dump_audio_output_position(); break; }
  case ec_dump_ao_length: { ctrl_dump_rep.dump_audio_output_length(); break; }
  case ec_dump_ao_open_state: { ctrl_dump_rep.dump_audio_output_open_state(); break; }
  case ec_dump_cop_value: 
    { 
      vector<string> temp = kvu_string_to_tokens_quoted(action_args_rep);
      if (temp.size() > 1) {
	ctrl_dump_rep.dump_chain_operator_value(atoi(temp[0].c_str()),
						atoi(temp[1].c_str()));
      }
      break; 
    }
  } // <-- switch-case

  if (action_reconnect == true) {
    if (is_selected() == false ||
	is_valid() == false) {
      set_last_error("Can't reconnect chainsetup.");
    }
    else {
      connect_chainsetup();
      if (selected_chainsetup() != connected_chainsetup()) {
	set_last_error("Can't reconnect chainsetup.");
      }
      else {
	if (action_restart == true) {
	  DBC_CHECK(is_running() != true);
	  start();
	}
      }
    }
  }
}

string ECA_CONTROL::last_value_to_string(void)
{
  string type = last_type();
  string result;

  if (type == "e") {
    result = last_error();
  }
  else if (type == "s")
    result = last_string();
  else if (type == "S")
    // FIXME: escape commas and double-backlashes
    result = kvu_vector_to_string(last_string_list(), ",");
  else if (type == "i")
    result = kvu_numtostr(last_integer());
  else if (type == "li")
    result = kvu_numtostr(last_long_integer());
  else if (type == "f")
    result = ECA_CONTROL_BASE::float_to_string(last_float());

  return result;
}

void ECA_CONTROL::print_last_value(void)
{
  string type = last_type();
  string result;

  if (type == "e") {
    result += "(eca-control) ERROR: ";
  }

  result += last_value_to_string();

  if (wellformed_mode_rep != true) {
    if (result.size() > 0) {
      ECA_LOG_MSG(ECA_LOGGER::eiam_return_values, result);
    }
  }
  else {
    /* in wellformed-output-mode we always create return output */
    ECA_LOG_MSG(ECA_LOGGER::eiam_return_values, type + " " + result);
  }
}

string ECA_CONTROL::chainsetup_details_to_string(const ECA_CHAINSETUP* cs) const
{
  string result;

  result += "\n -> Objects: " + kvu_numtostr(cs->inputs.size());
  result += " inputs, " + kvu_numtostr(cs->outputs.size());
  result += " outputs, " + kvu_numtostr(cs->chains.size());
  result += " chains";

  if (cs->is_valid()) 
    result += "\n -> State:   valid (can be connected)";
  else
    result += "\n -> State:   not valid (cannot be connected)";

  result += "\n -> Options: ";
  result += cs->options_to_string();
  
  return result;
}

string ECA_CONTROL::chainsetup_status(void) const 
{
  vector<ECA_CHAINSETUP*>::const_iterator cs_citer = session_repp->chainsetups_rep.begin();
  int index = 0;
  string result ("### Chainsetup status ###\n");

  while(cs_citer != session_repp->chainsetups_rep.end()) {
    result += "Chainsetup ("  + kvu_numtostr(++index) + ") \"";
    result += (*cs_citer)->name() + "\" ";
    if ((*cs_citer)->name() == selected_chainsetup()) result += "[selected] ";
    if ((*cs_citer)->name() == connected_chainsetup()) result += "[connected] ";
    result += chainsetup_details_to_string((*cs_citer));
    
    ++cs_citer;
    if (cs_citer != session_repp->chainsetups_rep.end()) result += "\n";
  }

  return result;
}

string ECA_CONTROL::chain_status(void) const
{
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------
  
  MESSAGE_ITEM mitem;
  vector<CHAIN*>::const_iterator chain_citer;
  const vector<string>& schains = selected_chainsetup_repp->selected_chains();
  mitem << "### Chain status (chainsetup '" 
	<< selected_chainsetup()
	<< "') ###\n";

  for(chain_citer = selected_chainsetup_repp->chains.begin(); chain_citer != selected_chainsetup_repp->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\" ";
    if ((*chain_citer)->is_muted()) mitem << "[muted] ";
    if ((*chain_citer)->is_processing() == false) mitem << "[bypassed] ";
    if (find(schains.begin(), schains.end(), (*chain_citer)->name()) != schains.end()) mitem << "[selected] ";
    for(int n = 0; n < (*chain_citer)->number_of_chain_operators(); n++) {
      mitem << "\"" << (*chain_citer)->get_chain_operator(n)->name() << "\"";
      if (n == (*chain_citer)->number_of_chain_operators()) mitem << " -> ";
    }
    ++chain_citer;
    if (chain_citer != selected_chainsetup_repp->chains.end()) mitem << "\n";
  }

  return mitem.to_string();
}

string ECA_CONTROL::chain_operator_status(void) const
{
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  MESSAGE_ITEM msg;
  string st_info_string;
  vector<CHAIN*>::const_iterator chain_citer = selected_chainsetup_repp->chains.begin();

  msg << "### Chain operator status (chainsetup '" 
      << selected_chainsetup() 
      << "') ###\n";

  while(chain_citer != selected_chainsetup_repp->chains.end()) {
    msg << "Chain \"" << (*chain_citer)->name() << "\":\n";
    for(int p = 0; p < (*chain_citer)->number_of_chain_operators(); p++) {
      const CHAIN_OPERATOR* cop = (*chain_citer)->get_chain_operator(p);
      msg << "\t" << p + 1 << ". " <<	cop->name();
      for(int n = 0; n < cop->number_of_params(); n++) {
	if (n == 0) msg << ": ";
	msg << "[" << n + 1 << "] ";
	msg << cop->get_parameter_name(n + 1);
	msg << " ";
	msg << ECA_CONTROL_BASE::float_to_string(cop->get_parameter(n + 1));
	if (n + 1 < cop->number_of_params()) msg <<  ", ";
      }
      st_info_string = cop->status();
      if (st_info_string.empty() == false) {
	msg << "\n\tStatus info:\n" << st_info_string;
      }
      if (p + 1 < (*chain_citer)->number_of_chain_operators()) msg << "\n";
    }
    ++chain_citer;
    if (chain_citer != selected_chainsetup_repp->chains.end()) msg << "\n";
  }
  return msg.to_string();
}

string ECA_CONTROL::controller_status(void) const
{
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  MESSAGE_ITEM mitem;
  string st_info_string;
  vector<CHAIN*>::const_iterator chain_citer;

  mitem << "### Controller status (chainsetup '"
	<< selected_chainsetup()
	<< "') ###\n";

  for(chain_citer = selected_chainsetup_repp->chains.begin(); chain_citer != selected_chainsetup_repp->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\":\n";
    for(int p = 0; p < (*chain_citer)->number_of_controllers(); p++) {
      const GENERIC_CONTROLLER* gtrl = (*chain_citer)->get_controller(p);
      mitem << "\t" << p + 1 << ". " << gtrl->name() << ": ";
      for(int n = 0; n < gtrl->number_of_params(); n++) {
	mitem << "\n\t\t[" << n + 1 << "] ";
	mitem << gtrl->get_parameter_name(n + 1);
	mitem << " ";
	mitem << ECA_CONTROL_BASE::float_to_string(gtrl->get_parameter(n + 1));
	if (n + 1 < gtrl->number_of_params()) mitem <<  ", ";
      }
      st_info_string = gtrl->status();
      if (st_info_string.empty() == false) {
	mitem << "\n\t -- Status info: " << st_info_string;
      }
      if (p + 1 < (*chain_citer)->number_of_controllers()) mitem << "\n";
    }
    ++chain_citer;
    if (chain_citer != selected_chainsetup_repp->chains.end()) mitem << "\n";
  }
  return mitem.to_string();
}

string ECA_CONTROL::aio_status(void) const
{
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  string st_info_string;
  vector<AUDIO_IO*>::size_type adev_sizet = 0;
  vector<AUDIO_IO*>::const_iterator adev_citer = selected_chainsetup_repp->inputs.begin();

  st_info_string += "### Audio input/output status (chainsetup '" +
    selected_chainsetup() + "') ###\n";

  while(adev_citer != selected_chainsetup_repp->inputs.end()) {
    st_info_string += "Input (" + kvu_numtostr(adev_sizet + 1) + "): \"";
    for(int n = 0; n < (*adev_citer)->number_of_params(); n++) {
      st_info_string += (*adev_citer)->get_parameter(n + 1);
      if (n + 1 < (*adev_citer)->number_of_params()) st_info_string += ",";
    }
    st_info_string += "\" - [" + (*adev_citer)->name() + "]";
    if ((*adev_citer) == selected_audio_input_repp) st_info_string += " [selected]";
    st_info_string += "\n -> connected to chains \"";
    vector<string> temp = selected_chainsetup_repp->get_attached_chains_to_input((selected_chainsetup_repp->inputs)[adev_sizet]);
    vector<string>::const_iterator p = temp.begin();
    while (p != temp.end()) {
      st_info_string += *p; 
      ++p;
      if (p != temp.end())  st_info_string += ",";
    }
    st_info_string += "\": " + (*adev_citer)->status() + "\n";
    ++adev_sizet;
    ++adev_citer;
  }

  adev_sizet = 0;
  adev_citer = selected_chainsetup_repp->outputs.begin();
  while(adev_citer != selected_chainsetup_repp->outputs.end()) {
    st_info_string += "Output (" + kvu_numtostr(adev_sizet + 1) + "): \"";
    for(int n = 0; n < (*adev_citer)->number_of_params(); n++) {
      st_info_string += (*adev_citer)->get_parameter(n + 1);
      if (n + 1 < (*adev_citer)->number_of_params()) st_info_string += ",";
    }
    st_info_string += "\" - [" + (*adev_citer)->name() + "]";
    if ((*adev_citer) == selected_audio_output_repp) st_info_string += " [selected]";
    st_info_string += "\n -> connected to chains \"";
    vector<string> temp = selected_chainsetup_repp->get_attached_chains_to_output((selected_chainsetup_repp->outputs)[adev_sizet]);
    vector<string>::const_iterator p = temp.begin();
    while (p != temp.end()) {
      st_info_string += *p; 
      ++p;
      if (p != temp.end())  st_info_string += ",";
    }
    st_info_string += "\": ";
    st_info_string += (*adev_citer)->status();
    ++adev_sizet;
    ++adev_citer;
    if (adev_sizet < selected_chainsetup_repp->outputs.size()) st_info_string += "\n";
  }
  return st_info_string;
}

void ECA_CONTROL::aio_register(void)
{
  ECA_LOG_MSG(ECA_LOGGER::info, "Registered audio object types:\n");
  string result (eca_aio_register_sub(ECA_OBJECT_FACTORY::audio_io_nonrt_map()));

  result += "\n";

  result += eca_aio_register_sub(ECA_OBJECT_FACTORY::audio_io_rt_map());

  set_last_string(result);  
}

static string eca_aio_register_sub(ECA_OBJECT_MAP& objmap)
{
  string result;
  const list<string>& objlist = objmap.registered_objects();
  list<string>::const_iterator p = objlist.begin();
  int count = 1;
  while(p != objlist.end()) {
    string temp;
    const AUDIO_IO* q = dynamic_cast<const AUDIO_IO*>(objmap.object_expr(*p));
    
    DBC_CHECK(q != 0);

    if (q != 0) {
      int params = q->number_of_params();
      if (params > 0) {
	temp += ": ";
	for(int n = 0; n < params; n++) {
	  temp += q->get_parameter_name(n + 1);
	  if (n + 1 < params) temp += ",";
	}
      }

      result += kvu_numtostr(count) + ". " + q->name() + ", regex: " + 
	        objmap.keyword_to_expr(*p) + ", params" + temp;
      result += "\n";

      ++count;
    }
    //  else std::cerr << "Failed obj: " << *p << "." << std::endl;

    ++p;
  }
  return result;
}

void ECA_CONTROL::cop_register(void)
{
  ECA_LOG_MSG(ECA_LOGGER::info, "Registered chain operators:\n");
  string result;
  const list<string>& objlist = ECA_OBJECT_FACTORY::chain_operator_map().registered_objects();
  list<string>::const_iterator p = objlist.begin();
  int count = 1;
  while(p != objlist.end()) {
    string temp;
    const CHAIN_OPERATOR* q = dynamic_cast<const CHAIN_OPERATOR*>(ECA_OBJECT_FACTORY::chain_operator_map().object(*p));
    if (q != 0) {
      int params = q->number_of_params();
      for(int n = 0; n < params; n++) {
	if (n == 0) temp += ":";
	temp += q->get_parameter_name(n + 1);
	if (n + 1 < params) temp += ",";
      }
      result += kvu_numtostr(count) + ". " + q->name() + ", -" + *p + temp;
      result += "\n";
      ++count;
    }
    ++p;
  }
  set_last_string(result);
}

void ECA_CONTROL::preset_register(void)
{
  ECA_LOG_MSG(ECA_LOGGER::info, "Registered effect presets:\n");
  string result;
#ifndef ECA_DISABLE_EFFECTS
  const list<string>& objlist = ECA_OBJECT_FACTORY::preset_map().registered_objects();
  list<string>::const_iterator p = objlist.begin();
  int count = 1;
  while(p != objlist.end()) {
    string temp;
    const PRESET* q = dynamic_cast<const PRESET*>(ECA_OBJECT_FACTORY::preset_map().object(*p));
    if (q != 0) {
      int params = q->number_of_params();
      for(int n = 0; n < params; n++) {
	if (n == 0) temp += ":";
	temp += q->get_parameter_name(n + 1);
	if (n + 1 < params) temp += ",";
      }

      result += kvu_numtostr(count) + ". " + q->name() + ", -pn:" + *p + temp;
      result += "\n";

      ++count;
    }
    ++p;
  }
#endif
  set_last_string(result);
}

void ECA_CONTROL::ladspa_register(void)
{
  ECA_LOG_MSG(ECA_LOGGER::info, "Registered LADSPA plugins:\n");
  string result;
#ifndef ECA_DISABLE_EFFECTS
  const list<string>& objlist = ECA_OBJECT_FACTORY::ladspa_plugin_map().registered_objects();
  list<string>::const_iterator p = objlist.begin();
  int count = 1;
  while(p != objlist.end()) {
    const EFFECT_LADSPA* q = dynamic_cast<const EFFECT_LADSPA*>(ECA_OBJECT_FACTORY::ladspa_plugin_map().object(*p));
    if (q != 0) {
      string temp = "\n\t-el:" + q->unique() + ",";
      int params = q->number_of_params();
      for(int n = 0; n < params; n++) {
	temp += "'" + q->get_parameter_name(n + 1) + "'";
	if (n + 1 < params) temp += ",";
      }
      
      result += kvu_numtostr(count) + ". " + q->name() + "" + temp;
      result += "\n";

      ++count;
    }
    ++p;
  }
#endif
  set_last_string(result);
}

void ECA_CONTROL::ctrl_register(void)
{
  ECA_LOG_MSG(ECA_LOGGER::info, "Registered controllers:\n");
  string result;
  const list<string>& objlist = ECA_OBJECT_FACTORY::controller_map().registered_objects();
  list<string>::const_iterator p = objlist.begin();
  int count = 1;
  while(p != objlist.end()) {
    string temp;
    const GENERIC_CONTROLLER* q = dynamic_cast<const GENERIC_CONTROLLER*>(ECA_OBJECT_FACTORY::controller_map().object(*p));
    if (q != 0) {
      int params = q->number_of_params();
      for(int n = 0; n < params; n++) {
	if (n == 0) temp += ":";
	temp += q->get_parameter_name(n + 1);
	if (n + 1 < params) temp += ",";
      }

      result += kvu_numtostr(count) + ". " + q->name() + ", -" + *p +
	temp;
      result += "\n";

      ++count;
    }
    ++p;
  }
  set_last_string(result);
}

/**
 * Print description of all chain operators and 
 * their parameters.
 */
void ECA_CONTROL::operator_descriptions_helper(const ECA_OBJECT_MAP& arg, string* result)
{
  /* switch to "C" locale to avoid strange floating point 
   * presentations that could break the output format
   * (for example "a,b" insteof of "a.b" */
#if defined(HAVE_LOCALE_H) && defined(HAVE_SETLOCALE)
  string old_locale (setlocale(LC_ALL, "C"));
#endif

  const list<string>& objlist = arg.registered_objects();
  list<string>::const_iterator p = objlist.begin();
  int count = 1;
  while(p != objlist.end()) {
    string temp;
    const OPERATOR* q = dynamic_cast<const OPERATOR*>(arg.object(*p));
    if (q != 0) {
      /* 1. keyword */
      *result += kvu_string_search_and_replace(*p, ',', '_');
      /* 2. name */
      *result += "," + kvu_string_search_and_replace(q->name(), ',', '_');
      /* 3. description */
      *result += "," + kvu_string_search_and_replace(q->description(), ',', '_');

      int params = q->number_of_params();

      /* 4. number of params */
      *result += "," + kvu_numtostr(params);

      /* 5. description of params (for all params) */
      for(int n = 0; n < params; n++) {
	struct OPERATOR::PARAM_DESCRIPTION pd;
	q->parameter_description(n + 1, &pd);

	/* 5.1 name of param */
	*result += "," + kvu_string_search_and_replace(q->get_parameter_name(n + 1), ',', '_');
	/* 5.2 description */
	*result += "," + kvu_string_search_and_replace(pd.description, ',', '_');
	/* 5.3 default value */
	*result += "," + ECA_CONTROL_BASE::float_to_string(pd.default_value);
	/* 5.4 is bounded above (1=yes, 0=no) */
	*result += ",above=" + kvu_numtostr(static_cast<int>(pd.bounded_above));
	/* 5.5 upper bound */
	*result += ",upper=" + ECA_CONTROL_BASE::float_to_string(pd.upper_bound);
	/* 5.6 is bounded below (1=yes, 0=no) */
	*result += ",below=" + kvu_numtostr(static_cast<int>(pd.bounded_below));
	/* 5.7 lower bound */
	*result += ",lower=" + ECA_CONTROL_BASE::float_to_string(pd.lower_bound);
	/* 5.8. is toggled (1=yes, 0=no) */
	*result += "," + kvu_numtostr(static_cast<int>(pd.toggled));
	/* 5.9. is integer value (1=yes, 0=no) */
	*result += "," + kvu_numtostr(static_cast<int>(pd.integer));
	/* 5.10. is logarithmis value (1=yes, 0=no) */
	*result += "," + kvu_numtostr(static_cast<int>(pd.logarithmic));
	/* 5.11. is output value (1=yes, 0=no) */
	*result += ",output=" + kvu_numtostr(static_cast<int>(pd.output));
      }
      *result += "\n";
      ++count;
    }
    ++p;
  }

  /* see above */
#if defined(HAVE_LOCALE_H) && defined(HAVE_SETLOCALE)
  setlocale(LC_ALL, old_locale.c_str());
#endif
}

/**
 * Print the description of all chain operators and 
 * their parameters.
 */
void ECA_CONTROL::cop_descriptions(void)
{
  string result;
  operator_descriptions_helper(ECA_OBJECT_FACTORY::chain_operator_map(), &result);
  set_last_string(result);
}

/**
 * Prints the description of all effect presets and 
 * their parameters.
 */
void ECA_CONTROL::preset_descriptions(void)
{
  string result;
  operator_descriptions_helper(ECA_OBJECT_FACTORY::preset_map(), &result);
  set_last_string(result);
}

/**
 * Prints the description of all LADSPA plugins and 
 * their parameters.
 */
void ECA_CONTROL::ladspa_descriptions(bool use_id)
{
  string result;
  if (use_id) {
    operator_descriptions_helper(ECA_OBJECT_FACTORY::ladspa_plugin_id_map(), &result);
  }
  else {
    operator_descriptions_helper(ECA_OBJECT_FACTORY::ladspa_plugin_map(), &result);
  }
  set_last_string(result);
}

/**
 * Print the description of all controllers and 
 * their parameters.
 */
void ECA_CONTROL::ctrl_descriptions(void)
{
  string result;
  operator_descriptions_helper(ECA_OBJECT_FACTORY::controller_map(), &result);
  set_last_string(result);
}
