// ------------------------------------------------------------------------
// eca-control.cpp: Class for controlling the whole ecasound library
// Copyright (C) 1999,2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <iostream.h>
#include <fstream.h>
#include <string>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>

#include <kvutils/value_queue.h>
#include <kvutils/message_item.h>

#include "eca-main.h"
#include "eca-session.h"
#include "eca-control.h"
#include "eca-chainop.h"
#include "eca-chainsetup.h"

#include "generic-controller.h"
#include "eca-chainop.h"

#include "audiofx_ladspa.h"
#include "eca-static-object-maps.h"
#include "eca-audio-object-map.h"
#include "eca-ladspa-plugin-map.h"
#include "eca-chainop-map.h"
#include "eca-controller-map.h"

#include "eca-error.h"
#include "eca-debug.h"

ECA_CONTROL::ECA_CONTROL (ECA_SESSION* psession) 
  : ECA_CONTROL_OBJECTS(psession),
    ctrl_dump_rep(this)
{ }

ECA_CONTROL::~ECA_CONTROL(void) { }

void ECA_CONTROL::command(const string& cmd) {
  clear_last_values();
  clear_action_arguments();
  vector<string> cmds = string_to_words(cmd);
  vector<string>::iterator p = cmds.begin();
  if (p != cmds.end()) {
    if (ECA_IAMODE_PARSER::cmd_map_rep.find(*p) == ECA_IAMODE_PARSER::cmd_map_rep.end()) {
      // ---
      // *p is not recognized as a iamode command
      // ---
      if (p->size() > 0 && (*p)[0] == '-') {
	direct_command(cmd);
      }
      else {
	set_last_error("Unknown command!");
      }
    }
    else {
      int action_id = ECA_IAMODE_PARSER::cmd_map_rep[*p];
      if (action_id == ec_help) {
	show_controller_help();
      }
      else {
	string first = *p;
	++p;
	if (p != cmds.end()) {
	  set_action_argument(vector<string> (p, cmds.end()));
	}
	action(action_id);
      }
    }
  }
}

void ECA_CONTROL::set_action_argument(const vector<string>& s) {
  action_args_rep = s;
  action_arg_f_set_rep = false;
}

void ECA_CONTROL::set_action_argument(double v) {
  action_arg_f_rep = v;
  action_arg_f_set_rep = true;
}

void ECA_CONTROL::clear_action_arguments(void) {
  action_args_rep.clear();
  action_arg_f_rep = 0.0f;
  action_arg_f_set_rep = false;
}

double ECA_CONTROL::first_argument_as_number(void) const {
  if (action_arg_f_set_rep == true)
    return(action_arg_f_rep);

  return(atof(action_args_rep[0].c_str()));
}

void ECA_CONTROL::command_float_arg(const string& cmd, double arg) {
  clear_action_arguments();
  set_action_argument(arg);
  int action_id = ec_unknown;
  if (ECA_IAMODE_PARSER::cmd_map_rep.find(cmd) != ECA_IAMODE_PARSER::cmd_map_rep.end()) {
    action_id = ECA_IAMODE_PARSER::cmd_map_rep[cmd];
  }
  action(action_id);
}

/**
 * Performs a direct EIAM command (prefixed with '-').
 */
void ECA_CONTROL::direct_command(const string& cmd) {
  string prefix = get_argument_prefix(cmd);
  if (prefix == "el" || prefix == "pn") { // --- LADSPA plugins and presets
    if (selected_chains().size() == 1) 
      add_chain_operator(cmd);
    else
      set_last_error("When adding chain operators, only one chain can be selected.");
  }
  else if (ECA_CHAIN_OPERATOR_MAP::object(prefix) != 0) {
    if (selected_chains().size() == 1) 
      add_chain_operator(cmd);
    else
      set_last_error("When adding chain operators, only one chain can be selected.");
  }
  else if (ECA_CONTROLLER_MAP::object(prefix) != 0) {
    if (selected_chains().size() == 1) 
      add_controller(cmd);
    else
      set_last_error("When adding controllers, only one chain can be selected.");
  }
  else {
    set_action_argument(string_to_words(cmd));
    action(ec_direct_option);
  }
}

void ECA_CONTROL::action(int action_id, 
			 const vector<string>& args) 
{
  clear_action_arguments();
  set_action_argument(args);
  action(action_id);
}

void ECA_CONTROL::action(int action_id) {
  clear_last_values();
  bool reconnect = false;
  bool restart = false;
  if (action_arg_f_set_rep == false && 
      action_args_rep.size() == 0 &&
      action_requires_params(action_id)) {
    set_last_error("Can't perform requested action; argument omitted.");
    return;
  }
  else if (selected_audio_object_repp == 0 &&
      action_requires_selected_audio_object(action_id)) {
    set_last_error("Can't perform requested action; no audio object selected.");
    return;
  }
  else if (is_selected() == false &&
      action_requires_selected(action_id)) {
    if (!is_connected()) {
      set_last_error("Can't perform requested action; no chainsetup selected.");
      return;
    }
    else {
      set_last_error("Warning! No chainsetup selected. Connected chainsetup will be selected.");
      select_chainsetup(connected_chainsetup());
    }
  }
  else if (is_connected() == false &&
      action_requires_connected(action_id)) {
    if (!is_selected()) {
      set_last_error("Can't perform requested action; no chainsetup connected.");
      return;
    }
    else {
      set_last_error("Warning! No chainsetup connected. Trying to connect currently selected chainsetup.");
      if (is_valid() == true) {
	connect_chainsetup();
      }
      if (is_connected() != true) {
	set_last_error("Warning! Selected chainsetup cannot be connected. Can't perform requested action.");
	return;
      }
    }
  }
  else if (selected_chainsetup() == connected_chainsetup() &&
      action_requires_selected_not_connected(action_id)) {
    set_last_error("Warning! This operation requires that chainsetup is disconnected. Temporarily disconnecting...");
    if (is_running()) restart = true;
    disconnect_chainsetup();
    reconnect = true;
  }

  switch(action_id) {
    // ---
    // Direct options
    // ---
  case ec_direct_option: 
    {
      try {
	selected_chainsetup_repp->interpret_options(action_args_rep);
      }
      catch(ECA_ERROR& e) {
	set_last_error("[" + e.error_section() + "] : \"" + e.error_message() + "\"");
      }
      break;
    }

  case ec_unknown: { set_last_error("Unknown command!"); break; }

    // ---
    // General
    // ---
  case ec_exit: { quit(); break; }
  case ec_start: 
    { 
      start();
      // ecadebug->msg("(eca-control) Can't perform requested action; no chainsetup connected.");
      break; 
    }
  case ec_stop: { if (is_engine_started() == true) stop(); break; }
  case ec_run: { run(); break; }
  case ec_debug:
    {
      int level = atoi((action_args_rep[0]).c_str());
      ecadebug->set_debug_level(level);
      set_last_string("Debug level set to " + kvu_numtostr(level) + ".");
      break;
    }

    // ---
    // Chainsetups
    // ---
  case ec_cs_add:
    {
      add_chainsetup(action_args_rep[0]);
      break;
    }
  case ec_cs_remove: { remove_chainsetup(); break; }
  case ec_cs_select: { select_chainsetup(action_args_rep[0]); break; }
  case ec_cs_selected: { set_last_string(selected_chainsetup()); break; }
  case ec_cs_list: { set_last_string_list(chainsetup_names()); break; }
  case ec_cs_index_select: { 
    if (action_args_rep[0].empty() != true) {
      if (action_args_rep[0][0] != 'c') {
	set_last_error("Invalid chainsetup index.");
      }
      else {
	select_chainsetup_by_index(action_args_rep[0]);
      }
    }
    break; 
  }
  case ec_cs_edit: { edit_chainsetup(); break; }
  case ec_cs_load: { load_chainsetup(action_args_rep[0]); break; }
  case ec_cs_save: { save_chainsetup(""); break; }
  case ec_cs_save_as: { save_chainsetup(action_args_rep[0]); break; }
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
  case ec_cs_set: { set_chainsetup_parameter(action_args_rep[0]); break; }
  case ec_cs_format: { set_chainsetup_sample_format(action_args_rep[0]); break; }
  case ec_cs_status: { 
    ecadebug->control_flow("Controller/Chainsetup status");
    set_last_string(chainsetup_status()); 
    break; 
  }
  case ec_cs_length: 
    { 
      set_chainsetup_processing_length_in_seconds(first_argument_as_number()); 
      break; 
    }
  case ec_cs_loop: { toggle_chainsetup_looping(); break; } 

  // ---
  // Chains
  // ---
  case ec_c_add: { add_chains(string_to_vector(action_args_rep[0], ',')); break; }
  case ec_c_select: { select_chains(string_to_vector(action_args_rep[0], ',')); break; }
  case ec_c_selected: { set_last_string_list(selected_chains()); break; }
  case ec_c_deselect: { deselect_chains(string_to_vector(action_args_rep[0], ',')); break; }
  case ec_c_select_add: 
    { 
      select_chains(string_to_vector(action_args_rep[0] + "," +
				     vector_to_string(selected_chains(), ","), ',')); 
      break; 
    }
  case ec_c_select_all: { select_all_chains(); break; }
  case ec_c_remove: { remove_chains(); break; }
  case ec_c_clear: { clear_chains(); break; }
  case ec_c_name: 
    { 
      if (selected_chains().size() != 1) {
	set_last_error("When renaming chains, only one chain canbe selected.");
      }
      else {
	rename_chain(action_args_rep[0]); 
      }
      break;
    }
  case ec_c_mute: { toggle_chain_muting(); break; }
  case ec_c_bypass: { toggle_chain_bypass(); break; }
  case ec_c_forward: 
    { 
      forward_chains(first_argument_as_number()); 
      break; 
    }
  case ec_c_rewind: 
    { 
      rewind_chains(first_argument_as_number()); 
      break; 
    }
  case ec_c_setpos: 
    { 
      set_position_chains(first_argument_as_number()); 
      break; 
    }
  case ec_c_status: 
    { 
      ecadebug->control_flow("Controller/Chain status");
      set_last_string(chain_status()); 
      break; 
    }
  case ec_c_list: { set_last_string_list(chain_names()); break; }

    // ---
    // Audio objects
    // ---
  case ec_aio_add_input: { add_audio_input(action_args_rep[0]); break; }
  case ec_aio_add_output: { if (action_args_rep.size() == 0) add_default_output(); else add_audio_output(action_args_rep[0]); break; }
  case ec_aio_select: { select_audio_object(action_args_rep[0]); break; }
  case ec_aio_select_input: { select_audio_input(action_args_rep[0]); break; }
  case ec_aio_select_output: { select_audio_output(action_args_rep[0]); break; }
  case ec_aio_selected: { set_last_string(get_audio_object()->label()); break; }
  case ec_aio_index_select: { 
    if (action_args_rep[0].empty() != true) {
      if (action_args_rep[0][0] != 'i' && action_args_rep[0][0] != 'o') {
	set_last_error("Invalid audio-input/output index.");
      }
      else {
	select_audio_object_by_index(action_args_rep[0]);
      }
    }
    break; 
  }
  case ec_aio_attach: { attach_audio_object(); break; }
  case ec_aio_remove: { remove_audio_object(); break; }
  case ec_aio_status: 
    { 
      ecadebug->control_flow("Controller/Audio input/output status");
      set_last_string(aio_status()); 
      break; 
    }
  case ec_aio_forward: 
    { 
      forward_audio_object(first_argument_as_number()); 
      break; 
    }
  case ec_aio_rewind: 
    { 
      rewind_audio_object(first_argument_as_number()); 
      break; 
    }
  case ec_aio_setpos: 
    { 
      set_audio_object_position(first_argument_as_number()); 
      break; 
    }
  case ec_aio_get_position: { set_last_float(get_audio_object()->position().seconds()); break; }
  case ec_aio_get_length: { set_last_float(get_audio_object()->length().seconds()); break; }
  case ec_aio_wave_edit: { wave_edit_audio_object(); break; }
  case ec_aio_register: { aio_register(); break; }

    // ---
    // Chain operators
    // ---
  case ec_cop_add: { add_chain_operator(action_args_rep[0]); break; }
  case ec_cop_select: { select_chain_operator(atoi((action_args_rep[0]).c_str())); break; }
  case ec_cop_remove: { remove_chain_operator(); break; }
    
  case ec_cop_set: 
    { 
      vector<string> a = string_to_vector(action_args_rep[0], ',');
      if (a.size() < 3) {
	set_last_error("Not enough parameters!");
	break;
      }
      int id1 = atoi(a[0].c_str());
      int id2 = atoi(a[1].c_str());
      CHAIN_OPERATOR::parameter_type v = atof(a[2].c_str());

      if (id1 > 0 && id2 > 0) {
	select_chain_operator(id1);
	select_chain_operator_parameter(id2);
	set_chain_operator_parameter(v);
      }
      else
	set_last_error("Chain operator indexing starts from 1.");
      break; 
    }
  case ec_copp_select: { select_chain_operator_parameter(atoi((action_args_rep[0]).c_str())); break; }
  case ec_copp_set: { set_chain_operator_parameter(first_argument_as_number()); break; }
  case ec_copp_get: { set_last_float(get_chain_operator_parameter()); break; }

  case ec_cop_status: 
    { 
      ecadebug->control_flow("Chain operator status");
      set_last_string(chain_operator_status()); 
      break; 
    }
  case ec_cop_register: { cop_register(); break; }

  case ec_preset_register: { preset_register(); break; }
  case ec_ladspa_register: { ladspa_register(); break; }

    // ---
    // Controllers
    // ---
  case ec_ctrl_add: { add_controller(action_args_rep[0]); break; }
  case ec_ctrl_select: { select_controller(atoi((action_args_rep[0]).c_str())); break; }
  case ec_ctrl_remove: {  remove_chain_operator(); break; }
  case ec_ctrl_status: 
    { 
      ecadebug->control_flow("Controller status");
      set_last_string(controller_status()); 
      break; 
    }
  case ec_ctrl_register: { ctrl_register(); break; }

    // ---
    // Changing position
    // ---
  case ec_rewind: {
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_rewind, first_argument_as_number());
    break;
  }
  case ec_forward: {
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_forward, first_argument_as_number());
    break;
  }
  case ec_setpos: {
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_setpos, first_argument_as_number());
    break;
  }
  case ec_get_position: { set_last_float(position_in_seconds_exact()); break; }
  case ec_get_length: { set_last_float(length_in_seconds_exact()); break; }

  // ---
  // Session status
  // ---
  case ec_st_general: 
    { 
      ecadebug->control_flow("Controller/General Status");
      print_general_status(); break; 
    }
  case ec_engine_status: { set_last_string(engine_status()); break; }

  // ---
  // Dump commands
  // ---
  case ec_dump_target: { ctrl_dump_rep.set_dump_target(action_args_rep[0]); break; }
  case ec_dump_status: { ctrl_dump_rep.dump_status(); break; }
  case ec_dump_position: { ctrl_dump_rep.dump_position(); break; }
  case ec_dump_length: { ctrl_dump_rep.dump_length(); break; }
  case ec_dump_cs_status: { ctrl_dump_rep.dump_chainsetup_status(); break; }
  case ec_dump_c_selected: { ctrl_dump_rep.dump_selected_chain(); break; }
  case ec_dump_aio_selected: { ctrl_dump_rep.dump_selected_audio_object(); break; }
  case ec_dump_aio_position: { ctrl_dump_rep.dump_audio_object_position(); break; }
  case ec_dump_aio_length: { ctrl_dump_rep.dump_audio_object_length(); break; }
  case ec_dump_aio_open_state: { ctrl_dump_rep.dump_audio_object_open_state(); break; }
  case ec_dump_cop_value: 
    { 
      if (action_args_rep.size() > 1) {
	ctrl_dump_rep.dump_chain_operator_value(atoi(action_args_rep[0].c_str()),
						atoi(action_args_rep[1].c_str())); 
      }
      break; 
    }
  } // <-- switch-case

  if (reconnect == true) {
    if (is_valid() == false || 
	is_selected() == false) {
      set_last_error("Can't reconnect chainsetup.");
    }
    else {
      connect_chainsetup();
      if (selected_chainsetup() != connected_chainsetup()) {
	set_last_error("Can't reconnect chainsetup.");
      }
      else {
	if (restart == true) start();
      }
    }
  }
}

void ECA_CONTROL::print_last_value(void) {
  string type = last_type();
  string result;
  if (type == "s") 
    result = last_string();
  else if (type == "S")
    result = vector_to_string(last_string_list(), ",");
  else if (type == "i")
    result = kvu_numtostr(last_integer());
  else if (type == "li")
    result = kvu_numtostr(last_long_integer());
  else if (type == "f")
    result = kvu_numtostr(last_float());
   
  if (result.size() > 0) {
    ecadebug->msg(result);
  }
}

void ECA_CONTROL::print_last_error(void) {
  if (last_error().size() > 0) {
    ecadebug->msg("(eca-control) ERROR: " + last_error());
  }
}

void ECA_CONTROL::print_general_status(void) {
  MESSAGE_ITEM st_info_string;

  if (is_selected()) {
    st_info_string << "Selected chainsetup: " +
                       selected_chainsetup() + "\n";
    st_info_string << "Selected chain(s): ";
    st_info_string << vector_to_string(selected_chainsetup_repp->selected_chains(),",");
    st_info_string << "\n";
  }
  else {
    st_info_string << "Selected chainsetup: -\n";
  }

  st_info_string << "Engine status: \"" << engine_status() << "\"\n";
  if (session_repp->multitrack_mode_rep) st_info_string << "Multitrack-mode: enabled\n";
  else st_info_string << "Multitrack-mode: disabled\n";
  if (session_repp->raised_priority()) st_info_string << "Raised-priority mode: enabled\n";

  set_last_string(st_info_string.to_string());
}

string ECA_CONTROL::chainsetup_status(void) const { 
  vector<ECA_CHAINSETUP*>::const_iterator cs_citer = session_repp->chainsetups_rep.begin();

  int index = 0;
  string result;
  while(cs_citer != session_repp->chainsetups_rep.end()) {
    result += "Chainsetup (c"  + kvu_numtostr(++index) + ") \"";
    result += (*cs_citer)->name() + "\" ";
    if ((*cs_citer)->name() == selected_chainsetup()) result += "[selected] ";
    if ((*cs_citer)->name() == connected_chainsetup()) result += "[connected] ";
    result += "\n\tFilename:\t\t" + (*cs_citer)->filename();
    result += "\n\tSetup:\t\t\tinputs " + kvu_numtostr((*cs_citer)->inputs.size());
    result += " - outputs " + kvu_numtostr((*cs_citer)->outputs.size());
    result += " - chains " + kvu_numtostr((*cs_citer)->chains.size());
    result += "\n\tBuffersize:\t\t" + kvu_numtostr((*cs_citer)->buffersize());
    result += "\n\tInternal sample rate:\t" + kvu_numtostr((*cs_citer)->sample_rate());
    result += "\n\tDefault sformat:\t" + kvu_numtostr(static_cast<int>((*cs_citer)->default_audio_format().bits())) 
                                    + "bit/"
                                    + kvu_numtostr(static_cast<int>((*cs_citer)->default_audio_format().channels())) 
                                    + "ch/"
                                     + kvu_numtostr((*cs_citer)->default_audio_format().samples_per_second());
    result += "\n\tFlags:\t\t\t";
    if ((*cs_citer)->double_buffering()) result += "D";
    if ((*cs_citer)->precise_sample_rates()) result += "P";
    if ((*cs_citer)->is_valid()) 
      result += "\n\tState: \t\t\tvalid - can be connected";
    else
      result += "\n\tState: \t\t\tnot valid - cannot be connected";

    ++cs_citer;
    if (cs_citer != session_repp->chainsetups_rep.end()) result += "\n";
  }

  return(result);
}

string ECA_CONTROL::chain_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  MESSAGE_ITEM mitem;
  vector<CHAIN*>::const_iterator chain_citer;
  vector<CHAIN_OPERATOR*>::const_iterator chainop_citer;
  const vector<string>& schains = selected_chainsetup_repp->selected_chains();

  for(chain_citer = selected_chainsetup_repp->chains.begin(); chain_citer != selected_chainsetup_repp->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\" ";
    if ((*chain_citer)->is_muted()) mitem << "[muted] ";
    if ((*chain_citer)->is_processing() == false) mitem << "[bypassed] ";
    if (find(schains.begin(), schains.end(), (*chain_citer)->name()) != schains.end()) mitem << "[selected] ";
    for(chainop_citer = (*chain_citer)->chainops_rep.begin(); chainop_citer != (*chain_citer)->chainops_rep.end();) {
      mitem << "\"" << (*chainop_citer)->name() << "\"";
      ++chainop_citer;
      if (chainop_citer != (*chain_citer)->chainops_rep.end()) mitem << " -> ";   
    }
    ++chain_citer;
    if (chain_citer != selected_chainsetup_repp->chains.end()) mitem << "\n";
  }

  return(mitem.to_string());
}

string ECA_CONTROL::chain_operator_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  MESSAGE_ITEM mitem;
  string st_info_string;
  vector<CHAIN*>::const_iterator chain_citer;
  vector<CHAIN_OPERATOR*>::size_type p;

  for(chain_citer = selected_chainsetup_repp->chains.begin(); chain_citer != selected_chainsetup_repp->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\":\n";
    for(p = 0; p < (*chain_citer)->chainops_rep.size(); p++) {
      mitem << "\t" << p + 1 << ". " <<	(*chain_citer)->chainops_rep[p]->name();
      for(int n = 0; n < (*chain_citer)->chainops_rep[p]->number_of_params(); n++) {
	if (n == 0) mitem << ": ";
	mitem << "[" << n + 1 << "] ";
	mitem << (*chain_citer)->chainops_rep[p]->get_parameter_name(n + 1);
	mitem << " ";
	mitem << kvu_numtostr((*chain_citer)->chainops_rep[p]->get_parameter(n + 1));
	if (n + 1 < (*chain_citer)->chainops_rep[p]->number_of_params()) mitem <<  ", ";
      }
      st_info_string = (*chain_citer)->chainops_rep[p]->status();
      if (st_info_string.empty() == false) {
	mitem << "\n\tStatus info:\n" << st_info_string;
      }
      if (p < (*chain_citer)->chainops_rep.size()) mitem << "\n";
    }
    ++chain_citer;
  }
  return(mitem.to_string());
}

string ECA_CONTROL::controller_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  MESSAGE_ITEM mitem;
  string st_info_string;
  vector<CHAIN*>::const_iterator chain_citer;
  vector<GENERIC_CONTROLLER*>::size_type p;

  for(chain_citer = selected_chainsetup_repp->chains.begin(); chain_citer != selected_chainsetup_repp->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\":\n";
    for(p = 0; p < (*chain_citer)->gcontrollers_rep.size(); p++) {
      mitem << "\t" << p + 1 << ". " << (*chain_citer)->gcontrollers_rep[p]->name() << ": ";
      for(int n = 0; n < (*chain_citer)->gcontrollers_rep[p]->number_of_params(); n++) {
	mitem << "\n\t\t[" << n + 1 << "] ";
	mitem << (*chain_citer)->gcontrollers_rep[p]->get_parameter_name(n + 1);
	mitem << " ";
	mitem << kvu_numtostr((*chain_citer)->gcontrollers_rep[p]->get_parameter(n + 1));
	if (n + 1 < (*chain_citer)->gcontrollers_rep[p]->number_of_params()) mitem <<  ", ";
      }
      st_info_string = (*chain_citer)->gcontrollers_rep[p]->status();
      if (st_info_string.empty() == false) {
	mitem << "\n\t -- Status info: " << st_info_string;
      }
      if (p < (*chain_citer)->gcontrollers_rep.size()) mitem << "\n";
    }
    ++chain_citer;
  }
  return(mitem.to_string());
}

string ECA_CONTROL::aio_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  string st_info_string;
  vector<AUDIO_IO*>::const_iterator adev_citer;
  vector<AUDIO_IO*>::size_type adev_sizet = 0;

  adev_citer = selected_chainsetup_repp->inputs.begin();
  
  while(adev_citer != selected_chainsetup_repp->inputs.end()) {
    st_info_string += "Input (i" + kvu_numtostr(adev_sizet + 1) + "): \"";
    for(int n = 0; n < (*adev_citer)->number_of_params(); n++) {
      st_info_string += (*adev_citer)->get_parameter(n + 1);
      if (n + 1 < (*adev_citer)->number_of_params()) st_info_string += ",";
    }
    st_info_string += "\" - [" + (*adev_citer)->name() + "]";
    if ((*adev_citer) == selected_audio_object_repp) st_info_string += " [selected]";
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
    st_info_string += "Output (o" + kvu_numtostr(adev_sizet + 1) + "): \"";
    for(int n = 0; n < (*adev_citer)->number_of_params(); n++) {
      st_info_string += (*adev_citer)->get_parameter(n + 1);
      if (n + 1 < (*adev_citer)->number_of_params()) st_info_string += ",";
    }
    st_info_string += "\" - [" + (*adev_citer)->name() + "]";
    if ((*adev_citer) == selected_audio_object_repp) st_info_string += " [selected]";
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

  return(st_info_string);
}

void ECA_CONTROL::aio_register(void) { 
  ecadebug->control_flow("Registered audio objects");
  string result;
  const map<string,string>& kmap = ::eca_audio_object_map.registered_objects();
  int count = 1;
  map<string,string>::const_iterator p = kmap.begin();
  while(p != kmap.end()) {
    string temp;
    AUDIO_IO* q = ECA_AUDIO_OBJECT_MAP::object(p->first, false);
    int params = q->number_of_params();
    if (params > 0) {
      temp += " (params: ";
      for(int n = 0; n < params; n++) {
	temp += q->get_parameter_name(n + 1);
	if (n + 1 < params) temp += ",";
      }
      temp += ")";
    }
    result += "\n";
    result += kvu_numtostr(count) + ". " + p->first + ", handled by \"" + p->second + "\"."  + temp;
    ++count;
    ++p;
  }
  set_last_string(result);
}

void ECA_CONTROL::cop_register(void) { 
  ecadebug->control_flow("Registered chain operators");
  string result;
  const map<string,string>& kmap = ECA_CHAIN_OPERATOR_MAP::registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    string temp;
    CHAIN_OPERATOR* q = ECA_CHAIN_OPERATOR_MAP::object(p->first);
    int params = q->number_of_params();
    for(int n = 0; n < params; n++) {
      if (n == 0) temp += ":";
      temp += q->get_parameter_name(n + 1);
      if (n + 1 < params) temp += ",";
    }
    result += kvu_numtostr(count) + ". " + p->first + " - " + p->second + temp;
    result += "\n";
    ++count;
    ++p;
  }
  set_last_string(result);
}

void ECA_CONTROL::preset_register(void) { 
  ecadebug->control_flow("Registered effect presets");
  string result;
  const map<string,string>& kmap = ::eca_preset_map.registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    result += "\n";
    result += kvu_numtostr(count) + ". " + p->first;
    ++count;
    ++p;
  }
  set_last_string(result);
}

void ECA_CONTROL::ladspa_register(void) { 
  ecadebug->control_flow("Registered LADSPA plugins");
  string result;
  const map<string,string>& kmap = ECA_LADSPA_PLUGIN_MAP::registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    string temp = "\n\t-el:" + p->second + ",";
    EFFECT_LADSPA* q = ECA_LADSPA_PLUGIN_MAP::object(p->first);
    int params = q->number_of_params();
    for(int n = 0; n < params; n++) {
      temp += "\"" + q->get_parameter_name(n + 1) + "\"";
      if (n + 1 < params) temp += ",";
    }

    result += "\n";
    result += kvu_numtostr(count) + ". " + p->first + temp;
    ++count;
    ++p;
  }
  set_last_string(result);
}

void ECA_CONTROL::ctrl_register(void) { 
  ecadebug->control_flow("Registered controllers");
  string result;
  const map<string,string>& kmap = ECA_CONTROLLER_MAP::registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    string temp;
    GENERIC_CONTROLLER* q = ECA_CONTROLLER_MAP::object(p->first);
    int params = q->number_of_params();
    for(int n = 0; n < params; n++) {
      if (n == 0) temp += ":";
      temp += q->get_parameter_name(n + 1);
      if (n + 1 < params) temp += ",";
    }
    result += "\n";
    result += kvu_numtostr(count) + ". " + p->first + " -" + p->second + temp;
    ++count;
    ++p;
  }
  set_last_string(result);
}
