// ------------------------------------------------------------------------
// eca-control-objects.cpp: Class for configuring libecasound objects
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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <pthread.h>

#include <kvutils/value_queue.h>

#include "eca-main.h"
#include "eca-session.h"
#include "eca-chainop.h"
#include "eca-chainsetup.h"
#include "eca-control-objects.h"

#include "eca-error.h"
#include "eca-debug.h"

ECA_CONTROL_OBJECTS::ECA_CONTROL_OBJECTS (ECA_SESSION* psession) 
  : ECA_CONTROL_BASE(psession) {
  selected_audio_object_repp = 0;
}

/**
 * Adds a new chainsetup
 *
 * @param name chainsetup name 
 *
 * require:
 *  name != ""
 *
 * ensure:
 *  selected_chainsetup() == name
 */
void ECA_CONTROL_OBJECTS::add_chainsetup(const string& name) {
  // --------
  REQUIRE(name != "");
  // --------

  try {
    session_repp->add_chainsetup(name);
    select_chainsetup(name);
    ecadebug->msg("(eca-controller) Added a new chainsetup with name \"" + name + "\".");
  }
  catch(ECA_ERROR& e) {
    cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }

  // --------
  ENSURE(selected_chainsetup() == name);
  // --------
}

/**
 * Removes chainsetup
 *
 * @param name chainsetup name 
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *
 * ensure:
 *  selected_chainsetup.empty() == true
 */
void ECA_CONTROL_OBJECTS::remove_chainsetup(void) {
  // --------
  REQUIRE(connected_chainsetup() != selected_chainsetup());
  REQUIRE(is_selected() == true);
  // --------

  ecadebug->msg("(eca-controller) Removing chainsetup:  \"" + selected_chainsetup() + "\".");
  session_repp->remove_chainsetup();
  selected_chainsetup_repp = 0;

  // --------
  ENSURE(selected_chainsetup().empty() == true);
  // --------
}

/**
 * Loads chainsetup from file 'filename'.
 *
 * @param name chainsetup filename 
 *
 * require:
 *  filename != ""
 *
 * ensure:
 *  filename exists implies loaded chainsetup == selected_chainsetup()
 */
void ECA_CONTROL_OBJECTS::load_chainsetup(const string& filename) {
  try {
    session_repp->load_chainsetup(filename);
    select_chainsetup(get_chainsetup_filename(filename)->name());
    ecadebug->msg("(eca-controller) Loaded chainsetup from file \"" + filename + "\".");
  }
  catch(ECA_ERROR& e) {
    cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
}

/**
 * Save selected chainsetup.
 *
 * @param filename chainsetup filename (if omitted, previously used filename will be used, if any)
 *
 * require:
 *  selected_chainsetup().empty() != true
 */
void ECA_CONTROL_OBJECTS::save_chainsetup(const string& filename) {
  // --------
  // require:
  assert(selected_chainsetup().empty() != true);
  // --------
  
  try {
    if (filename.empty() == true) 
      session_repp->save_chainsetup();
    else 
      session_repp->save_chainsetup(filename);
    
  ecadebug->msg("(eca-controller) Saved selected chainsetup \"" + selected_chainsetup() + "\".");
  }
  catch(ECA_ERROR& e) {
    cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
}

/**
 * Selects chainsetup
 *
 * @param name chainsetup name 
 *
 * require:
 *   name != ""
 *
 * ensure:
 *  name == selected_chainsetup() ||
 *  selected_chainsetup_rep == 0
 */
void ECA_CONTROL_OBJECTS::select_chainsetup(const string& name) {
  // --------
  // require:
  assert(name != "");
  // --------

  session_repp->select_chainsetup(name);
  selected_chainsetup_repp = session_repp->selected_chainsetup_repp;
  if (selected_chainsetup_repp != 0)
    ecadebug->msg("(eca-controller) Selected chainsetup:  \"" + selected_chainsetup() + "\".");
  else
    ecadebug->msg("(eca-controller) Chainsetup \"" + name + "\" doesn't exist!");

  // --------
  // ensure:
  assert(name == selected_chainsetup() ||
	 is_selected() == false);
  // --------
}

/**
 * Selects chainsetup by index (see chainsetup_status())
 *
 * @param name chainsetup name 
 *
 * require:
 *  index.empty() != true
 *  index[0] == 'c'
 *
 * ensure:
 *  selected_chainsetup_rep == 0
 */
void ECA_CONTROL_OBJECTS::select_chainsetup_by_index(const string& index) { 
  // --------
  // require:
  assert(index.empty() != true);
  assert(index[0] == 'c');
  // --------

  int index_number = ::atoi(string(index.begin() + 1,
				 index.end()).c_str());

  for(vector<ECA_CHAINSETUP*>::size_type p = 0; 
      p != session_repp->chainsetups_rep.size();
      p++) {
    if (index_number == static_cast<int>(p + 1)) {
      select_chainsetup(session_repp->chainsetups_rep[p]->name());
      break;
    }
  }
}

/**
 * Name of currently active chainsetup
 */
string ECA_CONTROL_OBJECTS::selected_chainsetup(void) const {
 if (selected_chainsetup_repp != 0)
   return(selected_chainsetup_repp->name());

 return("");
}

/**
 * Spawns an external editor for editing selected chainsetup
 *
 * require:
 *  is_selected() 
 *  connected_chainsetup() != selected_chainsetup()
 */
void ECA_CONTROL_OBJECTS::edit_chainsetup(void) {
  // --------
  // require:
  assert(selected_chainsetup().empty() != true);
  // --------

  bool hot_swap = false;
  bool restart = false;
  if (connected_chainsetup() == selected_chainsetup()) {
    hot_swap = true;
    if (is_running()) restart = true;
  }
  string origname = selected_chainsetup_repp->name();
  string origfilename = selected_chainsetup_repp->filename();
  string filename = string(::tmpnam(NULL));
  filename += ".ecs";

  if (hot_swap == true)
    set_chainsetup_parameter("-n:cs-edit-temp");

  save_chainsetup(filename);

  if (hot_swap == true)
    set_chainsetup_parameter(string("-n:") + origname);
  else
    remove_chainsetup();

  string editori = "";
  if (resource_value("ext-text-editor-use-getenv") == "true") {
    if (getenv("EDITOR") != 0) {
      editori = getenv("EDITOR");
    }
  }
  if (editori == "") 
    editori = resource_value("ext-text-editor");

  if (editori == "") {
    ecadebug->msg("(eca-controller) Can't edit; no text editor specified/available.");
  }

  editori += " " + filename;
  int res = ::system(editori.c_str());

  if (res == 127 || res == -1) {
    ecadebug->msg("(eca-controller) Can't edit; unable to open file in text editor \"" + string(editori.c_str()) + "\".");

  }
  else {
    load_chainsetup(filename);
    if (origfilename.empty() == false) set_chainsetup_filename(origfilename);
    remove(filename.c_str());

    if (hot_swap == true) {
      double pos = position_in_seconds_exact();
      disconnect_chainsetup();
      select_chainsetup("cs-edit-temp");
      if (origfilename.empty() == false) set_chainsetup_filename(origfilename);
      if (is_valid() == false) {
	ecadebug->msg("(eca-controller) Can't connect; edited chainsetup not valid.");
	select_chainsetup(origname);
	connect_chainsetup();
	ecasound_queue.push_back(ECA_PROCESSOR::ep_setpos, pos);
	if (is_connected() == true) {
	  if (restart == true) start();
	}
	select_chainsetup("cs-edit-temp");
      }
      else {
	connect_chainsetup();
	ecasound_queue.push_back(ECA_PROCESSOR::ep_setpos, pos);
	if (is_connected() == true) {
	  select_chainsetup(origname);
	  remove_chainsetup();
	  if (restart == true) start();
	  select_chainsetup("cs-edit-temp");
	  set_chainsetup_parameter(string("-n:") + origname);
	}
      }
    }
  }
}

/**
 * Sets processing length in seconds. If 'value' is 0,
 * length in unspecified.
 *
 * require:
 *  is_selected() == true
 *  value >= 0
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_processing_length_in_seconds(double value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  selected_chainsetup_repp->length_in_seconds(value);
  ecadebug->msg("(eca-controller) Set processing length to \"" + kvu_numtostr(value) + "\" seconds.");
}

/**
 * Sets processing length in samples. If 'value' is 0,
 * length in unspecified.
 *
 * require:
 *  is_selected() == true
 *  value >= 0
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_processing_length_in_samples(long int value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  selected_chainsetup_repp->length_in_samples(value);
  ecadebug->msg("(eca-controller) Set processing length to \"" + 
		 kvu_numtostr(selected_chainsetup_repp->length_in_seconds_exact()) + "\" seconds.");
}

/**
 * Sets default open mode for audio outputs.
 *
 * require:
 *  output_mode == AUDIO_IO::io_write || output_mode == AUDIO_IO::io_readwrite
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_output_mode(int output_mode) {
  // --------
  REQUIRE(output_mode == AUDIO_IO::io_write || output_mode == AUDIO_IO::io_readwrite);
  // --------
  selected_chainsetup_repp->set_output_openmode(output_mode);
}

/**
 * Toggles chainsetup looping
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::toggle_chainsetup_looping(void) {
 // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  if (selected_chainsetup_repp->looping_enabled()) {
    selected_chainsetup_repp->toggle_looping(false);
    ecadebug->msg("(eca-controller) Disabled looping.");
  }
  else {
    selected_chainsetup_repp->toggle_looping(true);
    ecadebug->msg("(eca-controller) Enabled looping.");
  }
}

/**
 * Connects selected chainsetup
 *
 * require:
 *  is_selected() == true
 *  is_valid() == true
 *
 * ensure:
 *  is_connected() == true
 */
void ECA_CONTROL_OBJECTS::connect_chainsetup(void) {
  // --------
  // require:
  assert(is_selected());
  assert(is_valid());
  // --------

  if (is_connected() == true) {
    disconnect_chainsetup();
  }
  session_repp->connect_chainsetup();
  ecadebug->msg("(eca-controller) Connected chainsetup:  \"" + connected_chainsetup() + "\".");

  // --------
  // ensure:
  assert(is_connected());
  // --------
}

/**
 * Name of connected chainsetup.
 */
string ECA_CONTROL_OBJECTS::connected_chainsetup(void) const {
  if (session_repp->connected_chainsetup_repp != 0) {
    return(session_repp->connected_chainsetup_repp->name());
  }

  return("");
}

/**
 * Disconnects activate chainsetup
 *
 * require:
 *  is_connected() == true
 *
 * ensure:
 *  connected_chainsetup() == ""
 */
void ECA_CONTROL_OBJECTS::disconnect_chainsetup(void) {
  // --------
  // require:
  assert(is_connected());
  // --------

  if (is_engine_started() == true) {
    stop_on_condition();
    close_engine();
  }

  ecadebug->msg("(eca-controller) Disconnecting chainsetup:  \"" + connected_chainsetup() + "\".");
  session_repp->disconnect_chainsetup();

  // --------
  // ensure:
  assert(connected_chainsetup() == "");
  // --------
}

/**
 * Gets a vector of al chainsetup names.
 */
vector<string> ECA_CONTROL_OBJECTS::chainsetup_names(void) const {
  return(session_repp->chainsetup_names());
}

/**
 * Gets a pointer to selected chainsetup, or 0 if no 
 * chainsetup is selected.
 */
ECA_CHAINSETUP* ECA_CONTROL_OBJECTS::get_chainsetup(void) const {
  return(selected_chainsetup_repp);
}

/**
 * Gets a pointer to chainsetup with filename 'filename'.
 */
ECA_CHAINSETUP* ECA_CONTROL_OBJECTS::get_chainsetup_filename(const string&
							      filename) const {
  vector<ECA_CHAINSETUP*>::const_iterator p = session_repp->chainsetups_rep.begin();
  while(p != session_repp->chainsetups_rep.end()) {
    if ((*p)->filename() == filename) {
      return((*p));
    }
    ++p;
    }
  return(0);
}

/** 
 * Gets chainsetup filename (used by save_chainsetup())
 *
 * require:
 *  is_selected() == true
 */
const string& ECA_CONTROL_OBJECTS::chainsetup_filename(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->filename());
}

/**
 * Sets chainsetup filename (used by save_chainsetup())
 *
 * require:
 *  is_selected() == true && 
 *  name.empty() != true
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_filename(const string& name) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(name.empty() != true);
  // --------
  selected_chainsetup_repp->set_filename(name);
}

/**
 * Sets general chainsetup chainsetup parameter
 *
 * require:
 *  is_selected() == true && 
 *  name.empty() != true
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_parameter(const string& name) {
  // --------
  // require:
  assert(is_selected() == true  && 
	 name.empty() != true);
  // --------

  selected_chainsetup_repp->interpret_global_option(name);
}

/**
 * Sets general chainsetup chainsetup parameter
 *
 * require:
 *  is_selected() == true && 
 *  name.empty() != true
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_sample_format(const string& name) {
  // --------
  // require:
  assert(is_selected() == true  && 
	 name.empty() != true);
  // --------

  try {
    selected_chainsetup_repp->interpret_object_option("-f:" + name);
  }
  catch(ECA_ERROR& e) {
    ecadebug->msg("(eca-control-objects) ERROR: [" + e.error_section() + "] : \"" + e.error_message() + "\"");
  }

}

/**
 * Adds a new chain (selected chainsetup). Added chain is automatically
 * selected.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  
 * ensure:
 *   selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::add_chain(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chainsetup() != connected_chainsetup());
  // --------

  add_chains(vector<string> (1, name));

  // --------
  // ensure:
  assert(selected_chains().size() > 0);
  // --------
}

/**
 * Adds new chains (selected chainsetup).  Added chains are automatically
 * selected.
 *
 * @param names comma separated list of chain names
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  
 * ensure:
 *   selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::add_chains(const string& names) { 
  // --------
  // require:
  assert(is_selected() == true &&
	 is_connected() == false);
  // --------

  add_chains(string_to_vector(names, ','));
  
  // --------
  // ensure:
  assert(selected_chains().size() > 0);
  // --------
}

/**
 * Adds new chains (selected chainsetup). Added chains are automatically
 * selected.
 *
 * @param namess vector of chain names
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  
 * ensure:
 *   selected_chains().size() == names.size()
 */
void ECA_CONTROL_OBJECTS::add_chains(const vector<string>& new_chains) { 
  // --------
  // require:
  assert(is_selected() == true &&
	 is_connected() == false);
  // --------

  selected_chainsetup_repp->add_new_chains(new_chains);
  selected_chainsetup_repp->select_chains(new_chains);

  ecadebug->msg("(eca-controller) Added chains: " +
		vector_to_string(new_chains, ", ") + ".");

  // --------
  // ensure:
  assert(selected_chains().size() == new_chains.size());
  // --------
}

/**
 * Removes currently selected chain (selected chainsetup)
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() > 0 &&
 *
 * ensure:
 *  selected_chains().size() == 0
 */
void ECA_CONTROL_OBJECTS::remove_chains(void) { 
  // --------
  // require:
  assert(is_selected() == true &&
	 selected_chains().size() > 0 &&
	 is_connected() == false);
  // --------

  selected_chainsetup_repp->remove_chains();

  ecadebug->msg("(eca-controlled) Removed selected chains.");

  // --------
  // ensure:
  assert(selected_chains().size() == 0);
  // --------
}

/**
 * Selects a chains (currently selected chainsetup)
 *
 * require:
 *   is_selected() == true
 *
 * ensure:
 *   selected_chains().size() == 1
 */
void ECA_CONTROL_OBJECTS::select_chain(const string& chain) {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  vector<string> c (1);
  c[0] = chain;
  selected_chainsetup_repp->select_chains(c);
  ecadebug->msg(ECA_DEBUG::user_objects, "(eca-controller) Selected chain: " + chain + ".");

  // --------
  ENSURE(selected_chains().size() == 1);
  // --------
}


/**
 * Selects chains (currently selected chainsetup)
 *
 * @param chains vector of chain names
 *
 * require:
 *   is_selected() == true
 *
 * ensure:
 *   selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::select_chains(const vector<string>& chains) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  selected_chainsetup_repp->select_chains(chains);

  ecadebug->msg(ECA_DEBUG::user_objects, "(eca-controller) Selected chains: " +
		vector_to_string(chains, ", ") + ".");
}

/**
 * Deselects chains (currently selected chainsetup)
 *
 * @param chains vector of chain names
 *
 * require:
 *   is_selected() == true
 */
void ECA_CONTROL_OBJECTS::deselect_chains(const vector<string>& chains) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  vector<string> schains = selected_chainsetup_repp->selected_chains();
  vector<string>::const_iterator p = chains.begin();
  while(p != chains.end()) {
    vector<string>::iterator o = schains.begin();
    while(o != schains.end()) {
      if (*p == *o) {
	ecadebug->msg("(eca-controller-objects) Deselected chain " + *o  + ".");
	schains.erase(o);
      }
      else
	++o;
    }
    ++p;
  }

  selected_chainsetup_repp->select_chains(schains);
}

/**
 * Selects all chains (currently selected chainsetup)
 *
 * require:
 *   is_selected() == true
 */
void ECA_CONTROL_OBJECTS::select_all_chains(void) {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  selected_chainsetup_repp->select_all_chains();

  ecadebug->msg("(eca-controller) Selected chains: " +
		vector_to_string(selected_chains(), ", ") + ".");
}

/**
 * Returns a list of selected chains (currently selected chainsetup)
 *
 * require:
 *  is_selected() == true
 */
const vector<string>& ECA_CONTROL_OBJECTS::selected_chains(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->selected_chains());
}

/**
 * Gets a vector of all chain names.
 *
 * require:
 *  is_selected() == true
 */
vector<string> ECA_CONTROL_OBJECTS::chain_names(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->chain_names());
}

/**
 * Gets a pointer to selected chain, or 0 if no chain is selected.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
CHAIN* ECA_CONTROL_OBJECTS::get_chain(void) const {
  // --------
  REQUIRE(is_selected() == true);
  REQUIRE(selected_chains().size() == 1);
  // --------
  vector<string> schains = selected_chainsetup_repp->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o)
	return(selected_chainsetup_repp->chains[p]);
    }
    ++o;
  }
  return(0);
}

/**
 * Clears all selected chains (all chain operators and controllers
 * are removed)
 *
 * @param name chain name 
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::clear_chains(void) { 
  // --------
  REQUIRE(is_selected() == true);
  REQUIRE(selected_chains().size() > 0);
  // --------
  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) { was_running = true; stop_on_condition(); }
  selected_chainsetup_repp->clear_chains();
  if (was_running == true) ::ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/**
 * Clears all selected chains (all chain operators and controllers
 * are removed)
 *
 * @param name chain name 
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 */
void ECA_CONTROL_OBJECTS::rename_chain(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  // --------
  selected_chainsetup_repp->rename_chain(name);     
}

void ECA_CONTROL_OBJECTS::send_chain_commands_to_engine(int command, double value) {
  vector<string> schains = selected_chainsetup_repp->selected_chains();

  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	ecasound_queue.push_back(ECA_PROCESSOR::ep_c_select, p);
	ecasound_queue.push_back(command, value);
	break;
      }
    }
    ++o;
  }
}

/**
 * Toggles whether chain is muted or not
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::toggle_chain_muting(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_mute, 0.0);
  } 
  else {
    selected_chainsetup_repp->toggle_chain_muting();
  }
}

/**
 * Toggles whether chain operators are enabled or disabled
 *
 * require:
 *  is_selected() == true && is_connected() == true
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::toggle_chain_bypass(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_bypass, 0.0);
  }
  else {
    selected_chainsetup_repp->toggle_chain_bypass();
  }
}

/**
 * Rewinds selected chains by 'pos_in_seconds' seconds
 *
 * require:
 *  is_selected() == true && is_connected() == true
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::rewind_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_rewind, pos_in_seconds);
}

/**
 * Forwards selected chains by 'pos_in_seconds' seconds
 *
 * require:
 *  is_selected() == true && is_connected() == true
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::forward_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_forward, pos_in_seconds);
}

/**
 * Sets position of selected chains to 'pos_in_seconds' seconds
 *
 * require:
 *  is_selected() == true && is_connected() == true
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::set_position_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_setpos, pos_in_seconds);
  }
  else {
    vector<string> schains = selected_chainsetup_repp->selected_chains();
    vector<string>::const_iterator o = schains.begin();
    while(o != schains.end()) {
      for(vector<CHAIN*>::size_type p = 0; 
	  p != selected_chainsetup_repp->chains.size();
	  p++) {
	if (selected_chainsetup_repp->chains[p]->name() == *o) {
	  selected_chainsetup_repp->chains[p]->input_id_repp->seek_position_in_seconds(pos_in_seconds);
	  selected_chainsetup_repp->chains[p]->output_id_repp->seek_position_in_seconds(pos_in_seconds);
	break;
	}
      }
      ++o;
    }
  }
}

/**
 * Sets default audio format. This format will be used, when
 * adding audio inputs and outputs.
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::set_default_audio_format(const string& sfrm,
				      int channels, 
				      long int srate) {
 // --------
  // require:
  assert(is_selected() == true);
  // --------

  string format;
  format = "-f:";
  format += sfrm;
  format += ",";
  format += kvu_numtostr(channels);
  format += ",";
  format += kvu_numtostr(srate);

  try {
    selected_chainsetup_repp->interpret_object_option(format);
  }
  catch(ECA_ERROR& e) {
    ecadebug->msg("(eca-control-objects) ERROR: [" +
		  e.error_section() + "] : \"" + e.error_message() + "\"");
  }

}

/**
 * Sets default audio format. This format will be used, when
 * adding audio inputs and outputs.
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::set_default_audio_format(const ECA_AUDIO_FORMAT& format) {
 // --------
  // require:
  assert(is_selected() == true);
  // --------

  set_default_audio_format(format.format_string(), 
			   static_cast<int>(format.channels()), 
			   static_cast<long int>(format.samples_per_second()));
}

/**
 * Selects an audio object
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::select_audio_object(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  select_audio_input(name);
  select_audio_output(name);
}

/**
 * Selects an audio input
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::select_audio_input(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  vector<AUDIO_IO*>::size_type p = 0;  
  for(p = 0; p != selected_chainsetup_repp->inputs.size(); p++) {
    if (selected_chainsetup_repp->inputs[p]->label() == name) {
      selected_audio_object_repp = selected_chainsetup_repp->inputs[p];
    }
  }
}

/**
 * Selects an audio output
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::select_audio_output(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  vector<AUDIO_IO*>::size_type p = 0;  
  for(p = 0; p != selected_chainsetup_repp->outputs.size(); p++) {
    if (selected_chainsetup_repp->outputs[p]->label() == name) {
      selected_audio_object_repp = selected_chainsetup_repp->outputs[p];
    }
  }
}

/**
 * Selects an audio object by index (see aio_status())
 *
 * require:
 *  is_selected() == true
 *  index.empty() != true
 *  index[0] == 'i' || index[0] == 'o'
 */
void ECA_CONTROL_OBJECTS::select_audio_object_by_index(const string& index) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(index.empty() != true);
  assert(index[0] == 'i' || index[0] == 'o');
  // --------

  int index_number = ::atoi(string(index.begin() + 1,
				 index.end()).c_str());

  vector<AUDIO_IO*>::size_type p = 0;
  if (index[0] == 'i') {
    for(p = 0; p != selected_chainsetup_repp->inputs.size(); p++) {
      if (index_number == static_cast<int>(p + 1)) {
	selected_audio_object_repp = selected_chainsetup_repp->inputs[p];
      }
    }
  }  
  else if (index[0] == 'o') {
    for(p = 0; p != selected_chainsetup_repp->outputs.size(); p++) {
      if (index_number == static_cast<int>(p + 1)) {
	selected_audio_object_repp = selected_chainsetup_repp->outputs[p];
      }
    }
  }
}

/**
 * Gets audio format of currently selected audio object. 
 * Note! This function is not a const member, because
 * it will open the targer audio object, if necessary.
 *
 * require:
 *  get_audio_object() != 0
 *  is_selected() == true
 */
ECA_AUDIO_FORMAT ECA_CONTROL_OBJECTS::get_audio_format(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(get_audio_object() != 0);
  // --------

  bool was_open = true;
  if (selected_audio_object_repp->is_open() == false) {
    was_open = false;
    selected_audio_object_repp->open();
  }
  ECA_AUDIO_FORMAT t (selected_audio_object_repp->channels(), 
		      selected_audio_object_repp->samples_per_second(), 
		      selected_audio_object_repp->sample_format(),
		      selected_audio_object_repp->interleaved_channels());
  if (was_open == false) selected_audio_object_repp->close();
  return(t);
}

/** 
 * Adds a new audio input (file, soundcard device, etc). Input 
 * is attached to currently selected chains (if any). If 'filename' 
 * doesn't exist or is otherwise invalid, no input is added.
 *
 * require:
 *   filename.empty() == false
 *   is_selected() == true
 *   connected_chainsetup() != selected_chainsetup()
 */
void ECA_CONTROL_OBJECTS::add_audio_input(const string& filename) {
  // --------
  REQUIRE(filename.empty() == false);
  REQUIRE(is_selected() == true);
  REQUIRE(connected_chainsetup() != selected_chainsetup());
  // --------

  try {
    selected_chainsetup_repp->interpret_object_option("-i:" + filename);
    select_audio_object(filename);
    ecadebug->msg("(eca-controller) Added audio input \"" + filename + "\".");
  }
  catch(ECA_ERROR& e) {
    ecadebug->msg("(eca-control-objects) ERROR: [" + e.error_section() + "] : \"" + e.error_message() + "\"");
  }
}

/** 
 * Adds a new audio output (file, soundcard device, etc). Output 
 * is attached to currently selected chains (if any). If 'filename' 
 * doesn't exist or is otherwise invalid, no input is added.
 *
 * require:
 *   filename.empty() == false
 *   is_selected() == true
 *   connected_chainsetup() != selected_chainsetup()
 */
void ECA_CONTROL_OBJECTS::add_audio_output(const string& filename) {
  // --------
  // require:
  assert(filename.empty() == false);
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  try {
    selected_chainsetup_repp->interpret_object_option("-o:" + filename);
    select_audio_object(filename);
    ecadebug->msg("(eca-controller) Added audio output \"" + filename +
		  "\".");
  }
  catch(ECA_ERROR& e) {
    ecadebug->msg("(eca-control-objects) ERROR: [" + e.error_section() + "] : \"" + e.error_message() + "\"");
  }
}

/** 
 * Adds a default output (as defined in ~/.ecasoundrc) and attach
 * it to currently selected chains.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 */
void ECA_CONTROL_OBJECTS::add_default_output(void) {
  // --------
  // require:
  assert(selected_chains().size() > 0);
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  add_audio_output(session_repp->ecaresources.resource("default-output"));
  ecadebug->msg("(eca-controller) Added default output to selected chains.");
}

/** 
 * Gets a pointer to the currently selected audio object. 
 * Returns 0 if no audio object is selected.
 *
 * require:
 *  is_selected() == true
 */
AUDIO_IO* ECA_CONTROL_OBJECTS::get_audio_object(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_audio_object_repp);
}

/**
 * Removes selected audio input/output
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_object() != 0
 *
 * ensure:
 *  selected_audio_object_rep = 0
 */
void ECA_CONTROL_OBJECTS::remove_audio_object(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_object() != 0);
  // --------
  ecadebug->msg("(eca-controller) Removing selected audio object \"" + selected_audio_object_repp->label() +
		"\" from selected chains.");
  if (selected_audio_object_repp->io_mode() == AUDIO_IO::io_read) 
    selected_chainsetup_repp->remove_audio_input(selected_audio_object_repp->label());
  else 
    selected_chainsetup_repp->remove_audio_output(selected_audio_object_repp->label());
  selected_audio_object_repp = 0;

  // --------
  // ensure:
  assert(selected_audio_object_repp == 0);
  // --------
}

/**
 * Attaches selected audio object to selected chains
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_object() != 0
 */
void ECA_CONTROL_OBJECTS::attach_audio_object(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_object() != 0);
  // --------
  if (selected_audio_object_repp->io_mode() == AUDIO_IO::io_read) 
    selected_chainsetup_repp->attach_input_to_selected_chains(selected_audio_object_repp);
  else
    selected_chainsetup_repp->attach_output_to_selected_chains(selected_audio_object_repp);

  ecadebug->msg("(eca-controller) Attached audio object \"" + selected_audio_object_repp->label() +
		"\" to selected chains.");
}

/**
 * Rewinds selected audio object by 'pos_in_seconds' seconds
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_object() != 0
 */
void ECA_CONTROL_OBJECTS::rewind_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_object() != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(selected_audio_object_repp->position_in_seconds_exact() - seconds);
}

/**
 * Forwards selected audio object by 'pos_in_seconds' seconds
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_object() != 0
 */
void ECA_CONTROL_OBJECTS::forward_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_object() != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(selected_audio_object_repp->position_in_seconds_exact() + seconds);
}

/**
 * Sets position of selected audio object
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_object() != 0
 */
void ECA_CONTROL_OBJECTS::set_audio_object_position(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_object() != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(seconds);
}

/**
 * Spawns an external wave editor for editing selected audio object.
 *
 * require:
 *  is_selected() 
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_object() != 0
 */
void ECA_CONTROL_OBJECTS::wave_edit_audio_object(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_object() != 0);
  // --------
  string name = selected_audio_object_repp->label();

  int res = ::system(string(resource_value("ext-wave-editor") + " " + name).c_str());
  if (res == 127 || res == -1) {
    ecadebug->msg("(eca-controller) Can't edit; unable to open wave editor \"" 
		  + resource_value("x-wave-editor") + "\".");
  }
}

/**
 * Adds a new chain operator
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
void ECA_CONTROL_OBJECTS::add_chain_operator(const string& chainop_params) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  // --------
  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  try {
    selected_chainsetup_repp->interpret_object_option(chainop_params);
  }
  catch(ECA_ERROR& e) {
    ecadebug->msg("(eca-control-objects) ERROR: [" +
		  e.error_section() + "] : \"" + e.error_message() + "\"");
  }

  if (was_running == true)
    ::ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/**
 * Adds a new chain operator. Pointer given as argument 
 * will remain to be usable, but notice that it is
 * _NOT_ thread-safe to use assigned/registered objects 
 * from client programs. You must be sure that ecasound 
 * isn't using the same object as you are. The 
 * easiest way to assure this is to disconnect 
 * the chainsetup to which object is attached.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 *  cotmp != 0
 */
void ECA_CONTROL_OBJECTS::add_chain_operator(CHAIN_OPERATOR* cotmp) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(cotmp != 0);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  selected_chainsetup_repp->add_chain_operator(cotmp);

  if (was_running == true)
    ::ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/** 
 * Gets a pointer to the Nth chain operator. If chain 
 * operator is not valid, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 *  chainop_id > 0
 */
CHAIN_OPERATOR* ECA_CONTROL_OBJECTS::get_chain_operator(int chainop_id) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  assert(chainop_id > 0);
  // --------

  vector<string> schains = selected_chainsetup_repp->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	if (chainop_id - 1 < static_cast<int>(selected_chainsetup_repp->chains[p]->chainops_rep.size()))
	  return(selected_chainsetup_repp->chains[p]->chainops_rep[chainop_id - 1]);
	else
	  return(0);
      }
    }
    ++o;
  }
  return(0);
}

/**
 * Removes the Nth chain operator
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 *  chainop_id > 0
 */
void ECA_CONTROL_OBJECTS::remove_chain_operator(int chainop_id) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(chainop_id > 0);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }
  
  vector<string> schains = selected_chainsetup_repp->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	selected_chainsetup_repp->chains[p]->select_chain_operator(chainop_id);
	selected_chainsetup_repp->chains[p]->remove_chain_operator();
	break;
      }
    }
    ++o;
  }

  if (was_running == true)
    ::ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/**
 * Sets chain operator parameter value
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 *  chainop_id > 0
 *  param > 0
 */
void ECA_CONTROL_OBJECTS::set_chain_operator_parameter(int chainop_id,
						  int param,
						  CHAIN_OPERATOR::parameter_type value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(chainop_id > 0);
  assert(param > 0);
  // --------

  vector<string> schains = selected_chainsetup_repp->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	if (selected_chainsetup() == connected_chainsetup()) {
	  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_c_select, p);
	  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_cop_select, chainop_id);
	  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_copp_select, param);
	  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_copp_value, value);
	}
	else {
	  if (chainop_id < static_cast<int>(selected_chainsetup_repp->chains[p]->chainops_rep.size() + 1)) {
	    selected_chainsetup_repp->chains[p]->select_chain_operator(chainop_id);
	    selected_chainsetup_repp->chains[p]->set_parameter(param,value);
	  }
	}
	return;
      }
    }
    ++o;
  }
}

/**
 * Adds a new controller
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::add_controller(const string& gcontrol_params) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  try {
    selected_chainsetup_repp->interpret_object_option(gcontrol_params);
  }
  catch(ECA_ERROR& e) {
    ecadebug->msg("(eca-control-objects) ERROR: [" + e.error_section() + "] : \"" + e.error_message() + "\"");
  }

  if (was_running == true)
    ::ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/**
 * Removes the Nth controller.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 *  controller_id > 0
 */
void ECA_CONTROL_OBJECTS::remove_controller(int controller_id) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(controller_id > 0);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }
  
  vector<string> schains = selected_chainsetup_repp->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	selected_chainsetup_repp->chains[p]->select_controller(controller_id);
	selected_chainsetup_repp->chains[p]->remove_controller();
	break;
      }
    }
    ++o;
  }

  if (was_running == true)
    ::ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/** 
 * Gets a pointer to the Nth controller. If controller is not
 * valid, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 *  controller_id > 0
 */
GENERIC_CONTROLLER* ECA_CONTROL_OBJECTS::get_controller(int controller_id) const {
  // --------
  REQUIRE(is_selected() == true);
  REQUIRE(connected_chainsetup() != selected_chainsetup());
  REQUIRE(selected_chains().size() == 1);
  REQUIRE(controller_id > 0);
  // --------

  vector<string> schains = selected_chainsetup_repp->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	if (controller_id - 1 < static_cast<int>(selected_chainsetup_repp->chains[p]->gcontrollers_rep.size()))
	  return(selected_chainsetup_repp->chains[p]->gcontrollers_rep[controller_id - 1]);
	else
	  return(0);
      }
    }
    ++o;
  }
  return(0);
}
