// ------------------------------------------------------------------------
// eca-controller.cpp: Class for controlling the whole ecasound library
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include "eca-controller.h"
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

ECA_CONTROLLER::ECA_CONTROLLER (ECA_SESSION* psession) 
  : ECA_CONTROLLER_DUMP(psession) { }

void ECA_CONTROLLER::command(const string& cmd) throw(ECA_ERROR*) {
  vector<string> cmds = string_to_words(cmd);

  vector<string>::iterator p = cmds.begin();
  if (p != cmds.end()) {
    if (*p == "") return;
    if (ECA_IAMODE_PARSER::cmd_map.find(string_search_and_replace(*p, '_', '-')) == ECA_IAMODE_PARSER::cmd_map.end()) {
      if (p->size() > 0 && (*p)[0] == '-') {
	direct_command(cmd);
      }
      else {
	ecadebug->msg("(eca-controller) ERROR: Unknown command!");
      }
    }
    else {
      if (ECA_IAMODE_PARSER::cmd_map[string_search_and_replace(*p, '_', '-')] == ec_help) {
	show_controller_help();
      }
      else {
	string first = *p;
	++p;
	if (p == cmds.end()) {
	  vector<string> empty;
	  action(ECA_IAMODE_PARSER::cmd_map[string_search_and_replace(first, '_', '-')], empty);
	}
	else {
	  action(ECA_IAMODE_PARSER::cmd_map[string_search_and_replace(first, '_', '-')], vector<string> (p, cmds.end()));
	}
      }
    }
  }
}

void ECA_CONTROLLER::direct_command(const string& cmd) {
  string prefix = get_argument_prefix(cmd);
  if (prefix == "el" || prefix == "pn") { // --- LADSPA plugins and presets
    if (selected_chains().size() == 1) 
      add_chain_operator(cmd);
    else
      ecadebug->msg("(eca-controller) When adding chain operators, only one chain can be selected.");
  }
  else if (ECA_CONTROLLER_MAP::object(prefix) != 0) {
    if (selected_chains().size() == 1) 
      add_controller(cmd);
    else
      ecadebug->msg("(eca-controller) When adding controllers, only one chain can be selected.");
  }
  else
    action(ec_direct_option, string_to_words(cmd));
}

void ECA_CONTROLLER::action(int action_id, 
			       const vector<string>& args) throw(ECA_ERROR*) {
  bool reconnect = false;
  bool restart = false;
  if (args.empty() == true &&
      action_requires_params(action_id)) {
    ecadebug->msg("(eca-controller) Can't perform requested action; argument omitted.");
    return;
  }
  else if (selected_audio_object_rep == 0 &&
      action_requires_selected_audio_object(action_id)) {
    ecadebug->msg("(eca-controller) Can't perform requested action; no audio object selected.");
    return;
  }
  else if (is_selected() == false &&
      action_requires_selected(action_id)) {
    if (!is_connected()) {
      ecadebug->msg("(eca-controller) Can't perform requested action; no chainsetup selected.");
      return;
    }
    else {
      ecadebug->msg("(eca-controller) Warning! No chainsetup selected. Connected chainsetup will be selected.");
      select_chainsetup(connected_chainsetup());
    }
  }
  else if (is_connected() == false &&
      action_requires_connected(action_id)) {
    if (!is_selected()) {
      ecadebug->msg("(eca-controller) Can't perform requested action; no chainsetup connected.");
      return;
    }
    else {
      ecadebug->msg("(eca-controller) Warning! No chainsetup connected. Trying to connect currently selected chainsetup.");
      if (is_valid() == true) connect_chainsetup();
      else return;
    }
  }
  else if (selected_chainsetup() == connected_chainsetup() &&
      action_requires_selected_not_connected(action_id)) {
    ecadebug->msg("(eca-controller) Warning! This operation requires that chainsetup is disconnected. Temporarily disconnecting...");
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
      vector<string> nargs = args;
      selected_chainsetup_rep->interpret_options(nargs);
      break;
    }

    // ---
    // General
    // ---
  case ec_exit: { quit(); break; }
  case ec_start: 
    { 
      start();
      // ecadebug->msg("(eca-controller) Can't perform requested action; no chainsetup connected.");
      break; 
    }
  case ec_stop: { if (is_engine_started() == true) stop(); break; }
  case ec_run: { run(); break; }
  case ec_debug:
    {
      int level = atoi((args[0]).c_str());
      ecadebug->set_debug_level(level);
      ecadebug->msg("Debug level set to " + kvu_numtostr(level) +
		    ".");
      break;
    }

    // ---
    // Chainsetups
    // ---
  case ec_cs_add:
    {
      add_chainsetup(args[0]);
      break;
    }
  case ec_cs_remove: { remove_chainsetup(); break; }
  case ec_cs_select: { select_chainsetup(args[0]); break; }
  case ec_cs_index_select: { 
    if (args[0].empty() != true) {
      if (args[0][0] != 'c') {
	ecadebug->msg("(eca-controller) ERROR! Invalid chainsetup index.");
      }
      else {
	select_chainsetup_by_index(args[0]);
      }
    }
    break; 
  }
  case ec_cs_edit: { edit_chainsetup(); break; }
  case ec_cs_load: { load_chainsetup(args[0]); break; }
  case ec_cs_save: { save_chainsetup(""); break; }
  case ec_cs_save_as: { save_chainsetup(args[0]); break; }
  case ec_cs_connect: 
    { 
      if (is_valid() != false) {
	connect_chainsetup(); 
      }
      else {
	ecadebug->msg("(eca-controller) Can't connect; chainsetup not valid!");
      }
      break; 
    }
  case ec_cs_disconnect: { disconnect_chainsetup(); break; }
  case ec_cs_set: { set_chainsetup_parameter(args[0]); break; }
  case ec_cs_format: { set_chainsetup_sample_format(args[0]); break; }
  case ec_cs_status: { 
    ecadebug->control_flow("Controller/Chainsetup status");
    ecadebug->msg(chainsetup_status()); 
    break; 
  }
  case ec_cs_length: 
    { 
      double value = atof((args[0]).c_str());
      set_chainsetup_processing_length_in_seconds(value); 
      break; 
    }
  case ec_cs_loop: { toggle_chainsetup_looping(); } 

  // ---
  // Chains
  // ---
  case ec_c_add: { add_chains(string_to_vector(args[0], ',')); break; }
  case ec_c_select: { select_chains(string_to_vector(args[0], ',')); break; }
  case ec_c_deselect: { deselect_chains(string_to_vector(args[0], ',')); break; }
  case ec_c_select_add: 
    { 
      select_chains(string_to_vector(args[0] + "," +
				     vector_to_string(selected_chains(), ","), ',')); 
      break; 
    }
  case ec_c_select_all: { select_all_chains(); break; }
  case ec_c_remove: { remove_chains(); break; }
  case ec_c_clear: { clear_chains(); break; }
  case ec_c_name: 
    { 
      if (selected_chains().size() != 1) {
	ecadebug->msg("(eca-controller) When renaming chains, only one chain canbe selected.");
      }
      else {
	rename_chain(args[0]); 
      }
      break;
    }
  case ec_c_mute: { toggle_chain_muting(); break; }
  case ec_c_bypass: { toggle_chain_bypass(); break; }
  case ec_c_forward: 
    { 
      double value = atof((args[0]).c_str());
      forward_chains(value); 
      break; 
    }
  case ec_c_rewind: 
    { 
      double value = atof((args[0]).c_str());
      rewind_chains(value); 
      break; 
    }
  case ec_c_setpos: 
    { 
      double value = atof((args[0]).c_str());
      set_position_chains(value); 
      break; 
    }
  case ec_c_status: 
    { 
      ecadebug->control_flow("Controller/Chain status");
      ecadebug->msg(chain_status()); 
      break; 
    }

    // ---
    // Audio objects
    // ---
  case ec_aio_add_input: { add_audio_input(args[0]); break; }
  case ec_aio_add_output: { if (args.size() == 0) add_default_output(); else add_audio_output(args[0]); break; }
  case ec_aio_select: { select_audio_object(args[0]); break; }
  case ec_aio_select_input: { select_audio_input(args[0]); break; }
  case ec_aio_select_output: { select_audio_output(args[0]); break; }
  case ec_aio_index_select: { 
    if (args[0].empty() != true) {
      if (args[0][0] != 'i' && args[0][0] != 'o') {
	ecadebug->msg("(eca-controller) ERROR! Invalid audio-input/output index.");
      }
      else {
	select_audio_object_by_index(args[0]);
      }
    }
    break; 
  }
  case ec_aio_attach: { attach_audio_object(); break; }
  case ec_aio_remove: { remove_audio_object(); break; }
  case ec_aio_status: 
    { 
      ecadebug->control_flow("Controller/Audio input/output status");
      ecadebug->msg(aio_status()); 
      break; 
    }
  case ec_aio_forward: 
    { 
      double value = atof((args[0]).c_str());
      forward_audio_object(value); 
      break; 
    }
  case ec_aio_rewind: 
    { 
      double value = atof((args[0]).c_str());
      rewind_audio_object(value); 
      break; 
    }
  case ec_aio_setpos: 
    { 
      double value = atof((args[0]).c_str());
      set_audio_object_position(value); 
      break; 
    }
  case ec_aio_wave_edit: { wave_edit_audio_object(); break; }
  case ec_aio_register: { aio_register(); break; }

    // ---
    // Chain operators
    // ---
  case ec_cop_add: { add_chain_operator(args[0]); break; }
  case ec_cop_remove: 
    { 
      vector<string> a = string_to_vector(args[0], ',');
      int id = atoi(a[0].c_str());
      if (id > 0)
	remove_chain_operator(id);
      else
	ecadebug->msg("(eca-controller) ERROR! Chain operator indexing starts from 1.");
      break; 
    }
    
  case ec_cop_set: 
    { 
      vector<string> a = string_to_vector(args[0], ',');
      if (a.size() < 3) {
	ecadebug->msg("(eca-controller) ERROR! Not enough parameters!");
	break;
      }
      int id1 = atoi(a[0].c_str());
      int id2 = atoi(a[1].c_str());
      CHAIN_OPERATOR::parameter_type v = atof(a[2].c_str());

      if (id1 > 0 && id2 > 0) {
	set_chain_operator_parameter(id1,id2, v);
      }
      else
	ecadebug->msg("(eca-controller) ERROR! Chain operator indexing starts from 1.");
      break; 
    }
  case ec_cop_status: 
    { 
      ecadebug->control_flow("Chain operator status");
      ecadebug->msg(chain_operator_status()); 
      break; 
    }
  case ec_cop_register: { cop_register(); break; }

  case ec_preset_register: { preset_register(); break; }
  case ec_ladspa_register: { ladspa_register(); break; }

    // ---
    // Controllers
    // ---
  case ec_ctrl_add: { add_controller(args[0]); break; }
  case ec_ctrl_status: 
    { 
      ecadebug->control_flow("Controller status");
      ecadebug->msg(controller_status()); 
      break; 
    }
  case ec_ctrl_register: { ctrl_register(); break; }

    // ---
    // Changing position
    // ---
  case ec_rewind: {
    ecasound_queue.push_back(ECA_PROCESSOR::ep_rewind, atof(args[0].c_str()));
    break;
  }
  case ec_forward: {
    ecasound_queue.push_back(ECA_PROCESSOR::ep_forward, atof(args[0].c_str()));
    break;
  }
  case ec_setpos: {
    ecasound_queue.push_back(ECA_PROCESSOR::ep_setpos, atof(args[0].c_str()));
    break;
  }

  // ---
  // Session status
  // ---
  case ec_st_general: 
    { 
      ecadebug->control_flow("Controller/General Status");
      print_general_status(); break; 
    }

  // ---
  // Dump commands
  // ---
  case ec_dump_target: { set_dump_target(args[0]); break; }
  case ec_dump_status: { dump_status(); break; }
  case ec_dump_position: { dump_position(); break; }
  case ec_dump_length: { dump_length(); break; }
  case ec_dump_cs_status: { dump_chainsetup_status(); break; }
  case ec_dump_c_selected: { dump_selected_chain(); break; }
  case ec_dump_aio_selected: { dump_selected_audio_object(); break; }
  case ec_dump_aio_position: { dump_audio_object_position(); break; }
  case ec_dump_aio_length: { dump_audio_object_length(); break; }
  case ec_dump_aio_open_state: { dump_audio_object_open_state(); break; }
  case ec_dump_cop_value: 
    { 
      if (args.size() > 1) {
	dump_chain_operator_value(atoi(args[0].c_str()),
				  atoi(args[1].c_str())); 
      }
      break; 
    }
  } // <-- switch-case

  if (reconnect == true) {
    if (is_valid() == false || 
	is_selected() == false) {
      ecadebug->msg("(eca-controller) ERROR! Can't reconnect chainsetup.");
    }
    else {
      connect_chainsetup();
      if (selected_chainsetup() != connected_chainsetup()) {
	ecadebug->msg("(eca-controller) ERROR! Can't reconnect chainsetup.");
      }
      else {
	if (restart == true) start();
      }
    }
  }
}

void ECA_CONTROLLER::print_general_status(void) {
  MESSAGE_ITEM st_info_string;

  if (is_selected()) {
    st_info_string << "Selected chainsetup: " +
                       selected_chainsetup() + "\n";
    st_info_string << "Selected chain(s): ";
    st_info_string << vector_to_string(selected_chainsetup_rep->selected_chains(),",");
    st_info_string << "\n";
  }
  else {
    st_info_string << "Selected chainsetup: -\n";
  }

  st_info_string << "Engine status: \"" << engine_status() << "\"\n";
  if (session_rep->multitrack_mode) st_info_string << "Multitrack-mode: enabled\n";
  if (session_rep->multitrack_mode) st_info_string << "Multitrack-mode: enabled\n";
  else st_info_string << "Multitrack-mode: disabled\n";
  if (session_rep->raised_priority()) st_info_string << "Raised-priority mode: enabled\n";

  ecadebug->msg(st_info_string.to_string());
}

string ECA_CONTROLLER::chainsetup_status(void) const { 
  vector<ECA_CHAINSETUP*>::const_iterator cs_citer = session_rep->chainsetups.begin();

  int index = 0;
  string result;
  while(cs_citer != session_rep->chainsetups.end()) {
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
    if (cs_citer != session_rep->chainsetups.end()) result += "\n";
  }

  return(result);
}

string ECA_CONTROLLER::chain_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  MESSAGE_ITEM mitem;
  vector<CHAIN*>::const_iterator chain_citer;
  vector<CHAIN_OPERATOR*>::const_iterator chainop_citer;
  const vector<string>& schains = selected_chainsetup_rep->selected_chains();

  for(chain_citer = selected_chainsetup_rep->chains.begin(); chain_citer != selected_chainsetup_rep->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\" ";
    if ((*chain_citer)->is_muted()) mitem << "[muted] ";
    if ((*chain_citer)->is_processing() == false) mitem << "[bypassed] ";
    if (find(schains.begin(), schains.end(), (*chain_citer)->name()) != schains.end()) mitem << "[selected] ";
    for(chainop_citer = (*chain_citer)->chainops.begin(); chainop_citer != (*chain_citer)->chainops.end();) {
      mitem << "\"" << (*chainop_citer)->name() << "\"";
      ++chainop_citer;
      if (chainop_citer != (*chain_citer)->chainops.end()) mitem << " -> ";   
    }
    ++chain_citer;
    if (chain_citer != selected_chainsetup_rep->chains.end()) mitem << "\n";
  }

  return(mitem.to_string());
}

string ECA_CONTROLLER::chain_operator_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  MESSAGE_ITEM mitem;
  string st_info_string;
  vector<CHAIN*>::const_iterator chain_citer;
  vector<CHAIN_OPERATOR*>::size_type p;

  for(chain_citer = selected_chainsetup_rep->chains.begin(); chain_citer != selected_chainsetup_rep->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\":\n";
    for(p = 0; p < (*chain_citer)->chainops.size(); p++) {
      mitem << "\t" << p + 1 << ". " <<	(*chain_citer)->chainops[p]->name();
      for(int n = 0; n < (*chain_citer)->chainops[p]->number_of_params(); n++) {
	if (n == 0) mitem << ": ";
	mitem << "[" << n + 1 << "] ";
	mitem << (*chain_citer)->chainops[p]->get_parameter_name(n + 1);
	mitem << " ";
	mitem << kvu_numtostr((*chain_citer)->chainops[p]->get_parameter(n + 1));
	if (n + 1 < (*chain_citer)->chainops[p]->number_of_params()) mitem <<  ", ";
      }
      st_info_string = (*chain_citer)->chainops[p]->status();
      if (st_info_string.empty() == false) {
	mitem << "\n\tStatus info:\n" << st_info_string;
      }
      if (p < (*chain_citer)->chainops.size()) mitem << "\n";
    }
    ++chain_citer;
  }
  return(mitem.to_string());
}

string ECA_CONTROLLER::controller_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  MESSAGE_ITEM mitem;
  string st_info_string;
  vector<CHAIN*>::const_iterator chain_citer;
  vector<GENERIC_CONTROLLER*>::size_type p;

  for(chain_citer = selected_chainsetup_rep->chains.begin(); chain_citer != selected_chainsetup_rep->chains.end();) {
    mitem << "Chain \"" << (*chain_citer)->name() << "\":\n";
    for(p = 0; p < (*chain_citer)->gcontrollers.size(); p++) {
      mitem << "\t" << p + 1 << ". " << (*chain_citer)->gcontrollers[p]->name() << ": ";
      for(int n = 0; n < (*chain_citer)->gcontrollers[p]->number_of_params(); n++) {
	mitem << "\n\t\t[" << n + 1 << "] ";
	mitem << (*chain_citer)->gcontrollers[p]->get_parameter_name(n + 1);
	mitem << " ";
	mitem << kvu_numtostr((*chain_citer)->gcontrollers[p]->get_parameter(n + 1));
	if (n + 1 < (*chain_citer)->gcontrollers[p]->number_of_params()) mitem <<  ", ";
      }
      st_info_string = (*chain_citer)->gcontrollers[p]->status();
      if (st_info_string.empty() == false) {
	mitem << "\n\t -- Status info: " << st_info_string;
      }
      if (p < (*chain_citer)->gcontrollers.size()) mitem << "\n";
    }
    ++chain_citer;
  }
  return(mitem.to_string());
}

string ECA_CONTROLLER::aio_status(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  string st_info_string;
  vector<AUDIO_IO*>::const_iterator adev_citer;
  vector<AUDIO_IO*>::size_type adev_sizet = 0;

  adev_citer = selected_chainsetup_rep->inputs.begin();
  
  while(adev_citer != selected_chainsetup_rep->inputs.end()) {
    st_info_string += "Input (i" + kvu_numtostr(adev_sizet + 1) + "): \"";
    for(int n = 0; n < (*adev_citer)->number_of_params(); n++) {
      st_info_string += (*adev_citer)->get_parameter(n + 1);
      if (n + 1 < (*adev_citer)->number_of_params()) st_info_string += ",";
    }
    st_info_string += "\" - [" + (*adev_citer)->name() + "]";
    if ((*adev_citer) == selected_audio_object_rep) st_info_string += " [selected]";
    st_info_string += "\n\tconnected to chains \"";
    vector<string> temp = selected_chainsetup_rep->get_attached_chains_to_input((selected_chainsetup_rep->inputs)[adev_sizet]);
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
  adev_citer = selected_chainsetup_rep->outputs.begin();
  while(adev_citer != selected_chainsetup_rep->outputs.end()) {
    st_info_string += "Output (o" + kvu_numtostr(adev_sizet + 1) + "): \"";
    for(int n = 0; n < (*adev_citer)->number_of_params(); n++) {
      st_info_string += (*adev_citer)->get_parameter(n + 1);
      if (n + 1 < (*adev_citer)->number_of_params()) st_info_string += ",";
    }
    st_info_string += "\" - [" + (*adev_citer)->name() + "]";
    if ((*adev_citer) == selected_audio_object_rep) st_info_string += " [selected]";
    st_info_string += "\n\tconnected to chains \"";
    vector<string> temp = selected_chainsetup_rep->get_attached_chains_to_output((selected_chainsetup_rep->outputs)[adev_sizet]);
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
    if (adev_sizet < selected_chainsetup_rep->outputs.size()) st_info_string += "\n";
  }

  return(st_info_string);
}

void ECA_CONTROLLER::aio_register(void) const { 
  ecadebug->control_flow("Registered audio objects");
  const map<string,string>& kmap = ::eca_audio_object_map.registered_objects();
  int count = 1;
  map<string,string>::const_iterator p = kmap.begin();
  while(p != kmap.end()) {
    string temp;
    AUDIO_IO* q = ECA_AUDIO_OBJECT_MAP::object(p->second);
    q->map_parameters();
    int params = q->number_of_params();
    for(int n = 1; n < params; n++) {
      if (n == 1) temp += "\n\t--> ";
      temp += q->get_parameter_name(n + 1);
      if (n + 1 < params) temp += ",";
    }
    ecadebug->msg(kvu_numtostr(count) + ". " + p->first + ", keyword \"" + p->second
		  + "\"."  + temp);
    ++count;
    ++p;
  }
}

void ECA_CONTROLLER::cop_register(void) const { 
  ecadebug->control_flow("Registered chain operators");
  const map<string,string>& kmap = ECA_CHAIN_OPERATOR_MAP::registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    string temp;
    CHAIN_OPERATOR* q = ECA_CHAIN_OPERATOR_MAP::object(p->second);
    q->map_parameters();
    int params = q->number_of_params();
    for(int n = 0; n < params; n++) {
      if (n == 0) temp += ":";
      temp += q->get_parameter_name(n + 1);
      if (n + 1 < params) temp += ",";
    }

    ecadebug->msg(kvu_numtostr(count) + ". " + p->first + " -" + p->second
		  + temp);
    ++count;
    ++p;
  }
}

void ECA_CONTROLLER::preset_register(void) const { 
  ecadebug->control_flow("Registered effect presets");
  const map<string,string>& kmap = ::eca_preset_map.registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    ecadebug->msg(kvu_numtostr(count) + ". " + p->first);
    ++count;
    ++p;
  }
}

void ECA_CONTROLLER::ladspa_register(void) const { 
  ecadebug->control_flow("Registered LADSPA plugins");
  const map<string,string>& kmap = ECA_LADSPA_PLUGIN_MAP::registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    string temp = "\n\t-el:" + p->second + ",";
    EFFECT_LADSPA* q = ECA_LADSPA_PLUGIN_MAP::object(p->second);
    q->map_parameters();
    int params = q->number_of_params();
    for(int n = 0; n < params; n++) {
      temp += "\"" + q->get_parameter_name(n + 1) + "\"";
      if (n + 1 < params) temp += ",";
    }

    ecadebug->msg(kvu_numtostr(count) + ". " + p->first + temp);
    ++count;
    ++p;
  }
}

void ECA_CONTROLLER::ctrl_register(void) const { 
  ecadebug->control_flow("Registered controllers");
  const map<string,string>& kmap = ECA_CONTROLLER_MAP::registered_objects();
  map<string,string>::const_iterator p = kmap.begin();
  int count = 1;
  while(p != kmap.end()) {
    string temp;
    GENERIC_CONTROLLER* q = ECA_CONTROLLER_MAP::object(p->second);
    q->map_parameters();
    int params = q->number_of_params();
    for(int n = 0; n < params; n++) {
      if (n == 0) temp += ":";
      temp += q->get_parameter_name(n + 1);
      if (n + 1 < params) temp += ",";
    }
    ecadebug->msg(kvu_numtostr(count) + ". " + p->first + " -" + p->second
		  + temp);
    ++count;
    ++p;
  }
}
