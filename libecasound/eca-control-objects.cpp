// ------------------------------------------------------------------------
// eca-control-objects.cpp: Class for configuring libecasound objects
// Copyright (C) 2000,2001 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include <stdlib.h> /* getenv() */

#include <kvutils/value_queue.h>
#include <kvutils/temporary_file_directory.h>

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
  selected_audio_input_repp = 0;
  selected_audio_output_repp = 0;
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
 *  selected_chainsetup() == name || (last_error().size() > 0 && no_errors != true)
 */
void ECA_CONTROL_OBJECTS::add_chainsetup(const string& name) {
  // --------
  REQUIRE(name != "");
  // --------

  bool no_errors = true;
  try {
    session_repp->add_chainsetup(name);
    select_chainsetup(name);
    ecadebug->msg("(eca-controller) Added a new chainsetup with name \"" + name + "\".");
  }
  catch(ECA_ERROR& e) {
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
    no_errors = false;
  }

  // --------
  ENSURE(selected_chainsetup() == name || (last_error().size() > 0 && no_errors != true));
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
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
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
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
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
  else {
    ecadebug->msg("(eca-controller) Chainsetup \"" + name + "\" doesn't exist!");
    set_last_error("Chainsetup \"" + name + "\" doesn't exist!");
  }

  // --------
  // ensure:
  assert(name == selected_chainsetup() ||
	 is_selected() == false);
  // --------
}

/**
 * Selects a chainsetup by index.
 *
 * @param index_number an integer identifier 
 *
 * require:
 *  index_number > 0
 */
void ECA_CONTROL_OBJECTS::select_chainsetup_by_index(int index_number) { 
  // --------
  REQUIRE(index_number > 0);
  // --------

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

  TEMPORARY_FILE_DIRECTORY tempfile_dir_rep;
  string tmpdir ("ecasound-");
  char* tmp_p = getenv("USER");
  if (tmp_p != NULL) {
    tmpdir += string(tmp_p);
    tempfile_dir_rep.reserve_directory(tmpdir);
  }
  if (tempfile_dir_rep.is_valid() != true) {
    ecadebug->msg("(eca-controller) Warning! Unable to create temporary directory \"" + tmpdir + "\".");
    return;
  }

  string filename = tempfile_dir_rep.create_filename("cs-edit-tmp", ".ecs");

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
	session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_setpos, pos);
	if (is_connected() == true) {
	  if (restart == true) start();
	}
	select_chainsetup("cs-edit-temp");
      }
      else {
	connect_chainsetup();
	session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_setpos, pos);
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
  ecadebug->msg("(eca-controller) Set chainsetup processing length to \"" + kvu_numtostr(value) + "\" seconds.");
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
  ecadebug->msg("(eca-controller) Set chainsetup processing length to \"" + 
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
 *  is_connected() == true || (last_error().size() > 0 && no_errors != true)
 */
void ECA_CONTROL_OBJECTS::connect_chainsetup(void) {
  // --------
  // require:
  assert(is_selected());
  assert(is_valid());
  // --------

  bool no_errors = true;
  if (is_connected() == true) {
    disconnect_chainsetup();
  }
  try {
    session_repp->connect_chainsetup();
    ecadebug->msg("(eca-controller) Connected chainsetup:  \"" + connected_chainsetup() + "\".");
  }
  catch(ECA_ERROR& e) {
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
    no_errors = false;
  }
  if (is_connected() != true) {
    set_last_error("ECA-CONTROL-OBJECTS: Connecting chainsetup failed.");
    no_errors = false;
  }

  // --------
  // ensure:
  assert(is_connected() || no_errors != true);
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
 * Changes the chainsetup position relatively to the current position. 
 * Behaves differently depending on whether the selected
 * chainsetup is connected or not.
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::change_chainsetup_position(double seconds) { 
  // --------
  REQUIRE(is_selected());
  // --------

  if (connected_chainsetup() == selected_chainsetup()) {
    if (seconds < 0)
      send_chain_commands_to_engine(ECA_PROCESSOR::ep_rewind, 
				    -seconds);
    else
      send_chain_commands_to_engine(ECA_PROCESSOR::ep_forward,
				    seconds);
  }
  else {
    selected_chainsetup_repp->change_position_exact(seconds);
  }
}

/**
 * Sets the chainsetup position. Behaves differently depending on 
 * whether the selected chainsetup is connected or not.
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_position(double seconds) {
  // --------
  REQUIRE(is_selected());
  // --------

  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_setpos, seconds);
  }
  else {
    selected_chainsetup_repp->set_position_exact(seconds);
  }
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
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
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
 * Selects a set of chains using index numbers. Previously 
 * selected chains are first all deselected.
 * 
 *
 * @param index_numbers set of integer identifiers
 *
 * require:
 *   is_selected() == true
 */
void ECA_CONTROL_OBJECTS::select_chains_by_index(const vector<int>& index_numbers) { 
  // --------
  REQUIRE(is_selected() == true);
  // --------

  vector<string> selchains;
  for(vector<CHAIN*>::size_type p = 0; 
      p != selected_chainsetup_repp->chains.size();
      p++) {
    for(vector<int>::size_type q = 0;
	q != index_numbers.size();
	q++) {
      if (index_numbers[q] == static_cast<int>(p + 1)) {
	selchains.push_back(selected_chainsetup_repp->chains[p]->name());
	break;
      }
    }
  }
  select_chains(selchains);
}

/**
 * Selects a chains (currently selected chainsetup). Previously 
 * selected chains are first all deselected.
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
 * Selects chains (currently selected chainsetup). Previously 
 * selected chains are first all deselected.
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
  const vector<string>& schains = selected_chainsetup_repp->selected_chains();
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
 */
void ECA_CONTROL_OBJECTS::clear_chains(void) { 
  // --------
  REQUIRE(is_selected() == true);
  REQUIRE(connected_chainsetup() != selected_chainsetup());
  // --------
  selected_chainsetup_repp->clear_chains();
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
  const vector<string>& schains = selected_chainsetup_repp->selected_chains();

  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_c_select, p);
	session_repp->ecasound_queue_rep.push_back(command, value);
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
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_rewind, pos_in_seconds);
  }
  else {
    change_position_chains(-pos_in_seconds);
  }
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
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_forward, pos_in_seconds);
  }
  else {
    change_position_chains(pos_in_seconds);
  }

}

/**
 * Change the relative position of selected chains by 
 * 'pos_in_seconds' seconds. Affects all inputs and outputs
 * connected to these chains.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::change_position_chains(double change_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_setpos, change_in_seconds);
  }
  else {
    const vector<string>& schains = selected_chainsetup_repp->selected_chains();
    vector<string>::const_iterator o = schains.begin();
    while(o != schains.end()) {
      for(vector<CHAIN*>::size_type p = 0; 
	  p != selected_chainsetup_repp->chains.size();
	  p++) {
	if (selected_chainsetup_repp->chains[p]->name() == *o) {
	  double previous = selected_chainsetup_repp->chains[p]->input_id_repp->position_in_seconds_exact();
	  selected_chainsetup_repp->chains[p]->input_id_repp->seek_position_in_seconds(previous + change_in_seconds);
	  previous = selected_chainsetup_repp->chains[p]->output_id_repp->position_in_seconds_exact();
	  selected_chainsetup_repp->chains[p]->output_id_repp->seek_position_in_seconds(previous + change_in_seconds);
	  break;
	}
      }
      ++o;
    }
  }
}

/**
 * Sets position of selected chains to 'pos_in_seconds' seconds.
 * Affects all inputs and outputs connected to these chains.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() > 0
 */
void ECA_CONTROL_OBJECTS::set_position_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_setpos, pos_in_seconds);
  }
  else {
    const vector<string>& schains = selected_chainsetup_repp->selected_chains();
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

void ECA_CONTROL_OBJECTS::audio_input_as_selected(void) {
  selected_audio_object_repp = selected_audio_input_repp;
}

void ECA_CONTROL_OBJECTS::audio_output_as_selected(void) {
  selected_audio_object_repp = selected_audio_output_repp;
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
						   long int srate,
						   bool interleaving) {
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
  format += ",";
  if (interleaving == true) 
    format += "i";
  else
    format += "n";

  try {
    selected_chainsetup_repp->interpret_object_option(format);
  }
  catch(ECA_ERROR& e) {
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
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
			   static_cast<long int>(format.samples_per_second()),
			   format.interleaved_channels());
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

  selected_audio_input_repp = 0;
  vector<AUDIO_IO*>::size_type p = 0;  
  for(p = 0; p != selected_chainsetup_repp->inputs.size(); p++) {
    if (selected_chainsetup_repp->inputs[p]->label() == name) {
      selected_audio_input_repp = selected_chainsetup_repp->inputs[p];
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

  selected_audio_output_repp = 0;
  vector<AUDIO_IO*>::size_type p = 0;  
  for(p = 0; p != selected_chainsetup_repp->outputs.size(); p++) {
    if (selected_chainsetup_repp->outputs[p]->label() == name) {
      selected_audio_output_repp = selected_chainsetup_repp->outputs[p];
    }
  }
}

/**
 * Selects an audio input by index.
 *
 * require:
 *  is_selected() == true
 *  index > 0
 */
void ECA_CONTROL_OBJECTS::select_audio_input_by_index(int index_number) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(index > 0);
  // --------

  selected_audio_input_repp = 0;
  vector<AUDIO_IO*>::size_type p = 0;
  for(p = 0; p != selected_chainsetup_repp->inputs.size(); p++) {
    if (index_number == static_cast<int>(p + 1)) {
      selected_audio_input_repp = selected_chainsetup_repp->inputs[p];
    }
  }
}

/**
 * Selects an audio output by index.
 *
 * require:
 *  is_selected() == true
 *  index > 0
 */
void ECA_CONTROL_OBJECTS::select_audio_output_by_index(int index_number) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(index > 0);
  // --------

  selected_audio_output_repp = 0;
  vector<AUDIO_IO*>::size_type p = 0;
  for(p = 0; p != selected_chainsetup_repp->outputs.size(); p++) {
    if (index_number == static_cast<int>(p + 1)) {
      selected_audio_output_repp = selected_chainsetup_repp->outputs[p];
    }
  }
}

/**
 * Gets audio format information of the object given as argument.
 * Note! To get audio format information, audio objects need
 * to be opened. Because of this, object argument cannot be given 
 * as a const pointer.
 *
 * require:
 *  selected_audio_object_repp != 0
 */
ECA_AUDIO_FORMAT ECA_CONTROL_OBJECTS::get_audio_format(AUDIO_IO* aobj) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(aobj != 0);
  // --------

  bool was_open = true;
  if (aobj->is_open() == false) {
    was_open = false;
    try {
      aobj->open();
    }
    catch(AUDIO_IO::SETUP_ERROR&) { 
      // FIXME: what to do here?
    }
  }
  ECA_AUDIO_FORMAT t (aobj->channels(), 
		      aobj->samples_per_second(), 
		      aobj->sample_format(),
		      aobj->interleaved_channels());
  if (was_open == false) aobj->close();
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
    selected_audio_input_repp = 0;
    selected_chainsetup_repp->interpret_object_option("-i:" + filename);
    select_audio_input(filename);
    ecadebug->msg("(eca-controller) Added audio input \"" + filename + "\".");
  }
  catch(ECA_ERROR& e) {
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
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
    selected_audio_output_repp = 0;
    selected_chainsetup_repp->interpret_object_option("-o:" + filename);
    select_audio_output(filename);
    ecadebug->msg("(eca-controller) Added audio output \"" + filename +
		  "\".");
  }
  catch(ECA_ERROR& e) {
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
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
 * Gets a vector of all audio input names.
 *
 * require:
 *  is_selected() == true
 */
vector<string> ECA_CONTROL_OBJECTS::audio_input_names(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->audio_input_names());
}

/**
 * Gets a vector of all audio output names.
 *
 * require:
 *  is_selected() == true
 */
vector<string> ECA_CONTROL_OBJECTS::audio_output_names(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->audio_output_names());
}

/** 
 * Gets a pointer to the currently selected audio input. 
 * Returns 0 if no audio object is selected.
 *
 * require:
 *  is_selected() == true
 */
AUDIO_IO* ECA_CONTROL_OBJECTS::get_audio_input(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_audio_input_repp);
}

/** 
 * Gets a pointer to the currently selected audio output.
 * Returns 0 if no audio object is selected.
 *
 * require:
 *  is_selected() == true
 */
AUDIO_IO* ECA_CONTROL_OBJECTS::get_audio_output(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_audio_output_repp);
}

/**
 * Removes the selected audio input.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_input() != 0
 *
 * ensure:
 *  selected_audio_input_repp = 0
 */
void ECA_CONTROL_OBJECTS::remove_audio_input(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_input() != 0);
  // --------
  ecadebug->msg("(eca-controller) Removing selected audio input \"" + selected_audio_input_repp->label() +
		"\" from selected chains.");
  selected_chainsetup_repp->remove_audio_input(selected_audio_input_repp->label());
  selected_audio_input_repp = 0;

  // --------
  // ensure:
  assert(selected_audio_input_repp == 0);
  // --------
}

/**
 * Removes the selected audio output.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_output() != 0
 *
 * ensure:
 *  selected_audio_output_repp = 0
 */
void ECA_CONTROL_OBJECTS::remove_audio_output(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_output() != 0);
  // --------
  ecadebug->msg("(eca-controller) Removing selected audio output \"" + selected_audio_output_repp->label() +
		"\" from selected chains.");
  selected_chainsetup_repp->remove_audio_output(selected_audio_output_repp->label());
  selected_audio_output_repp = 0;

  // --------
  // ensure:
  assert(selected_audio_output_repp == 0);
  // --------
}

/**
 * Attaches selected audio input to selected chains
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_input() != 0
 */
void ECA_CONTROL_OBJECTS::attach_audio_input(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_input() != 0);
  // --------
  selected_chainsetup_repp->attach_input_to_selected_chains(selected_audio_input_repp);

  ecadebug->msg("(eca-controller) Attached audio input \"" + selected_audio_input_repp->label() +
		"\" to selected chains.");
}

/**
 * Attaches selected audio output to selected chains
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  get_audio_output() != 0
 */
void ECA_CONTROL_OBJECTS::attach_audio_output(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_output() != 0);
  // --------
  selected_chainsetup_repp->attach_output_to_selected_chains(selected_audio_output_repp);

  ecadebug->msg("(eca-controller) Attached audio output \"" + selected_audio_output_repp->label() +
		"\" to selected chains.");
}

/**
 * Rewinds selected audio object by 'pos_in_seconds' seconds
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::rewind_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(get_audio_input() != 0 || get_audio_output() != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(selected_audio_object_repp->position_in_seconds_exact() - seconds);
}

/**
 * Forwards selected audio object by 'pos_in_seconds' seconds
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::forward_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_repp != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(selected_audio_object_repp->position_in_seconds_exact() + seconds);
}

/**
 * Sets position of selected audio object
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::set_audio_object_position(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_repp != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(seconds);
}

/**
 * Spawns an external wave editor for editing selected audio object.
 *
 * require:
 *  is_selected() 
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::wave_edit_audio_object(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_repp != 0);
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
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
  }

  if (was_running == true)
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_start, 0.0);
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
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/** 
 * Returns a pointer to the the selected chain operator. If no chain 
 * operator is selected, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
CHAIN_OPERATOR* ECA_CONTROL_OBJECTS::get_chain_operator(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    return (selected_chainsetup_repp->chains[p]->selected_chainop_repp);

  return(0);
}

/** 
 * Returns a list of chain operator names.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
vector<string> ECA_CONTROL_OBJECTS::chain_operator_names(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  // --------

  vector<string> result; 
  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    CHAIN* selected_chain = selected_chainsetup_repp->chains[p];
    int save_selected_cop = selected_chain->selected_chain_operator();
    for(int n = 0; 
	n < selected_chain->number_of_chain_operators();
	n++) {
      selected_chain->select_chain_operator(n + 1);
      result.push_back(selected_chain->chain_operator_name());
    }
    selected_chain->select_chain_operator(save_selected_cop);	  
  }
  return(result);
}

/** 
 * Returns the index of the selected chain operator. If no chain 
 * operator is selected, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
int ECA_CONTROL_OBJECTS::selected_chain_operator(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    return (selected_chainsetup_repp->chains[p]->selected_chain_operator());

  return(0);
}

/**
 * Removes the selected chain operator
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 */
void ECA_CONTROL_OBJECTS::remove_chain_operator(void) { 
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

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    selected_chainsetup_repp->chains[p]->remove_chain_operator();

  if (was_running == true)
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/**
 * Selects chain operator 'chainop_id'.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 *  chainop_id > 0
 */
void ECA_CONTROL_OBJECTS::select_chain_operator(int chainop_id) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(chainop_id > 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    if (selected_chainsetup() == connected_chainsetup()) {
      session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_c_select, p);
      session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_cop_select, chainop_id);
    }
    if (chainop_id < static_cast<int>(selected_chainsetup_repp->chains[p]->chainops_rep.size() + 1)) {
      selected_chainsetup_repp->chains[p]->select_chain_operator(chainop_id);
      session_repp->active_chain_index_rep = p;
      session_repp->active_chainop_index_rep = chainop_id;
    }
  }
}

/** 
 * Returns a list of chain operator parameter names.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 *  get_chain_operator() != 0
 */
vector<string> ECA_CONTROL_OBJECTS::chain_operator_parameter_names(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(get_chain_operator() != 0);
  // --------

  vector<string> result; 
  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    CHAIN* selected_chain = selected_chainsetup_repp->chains[p];
    int save_selected_copp = selected_chain->selected_chain_operator_parameter();
    for(int n = 0; 
	n < selected_chain->number_of_chain_operator_parameters();
	n++) {
      selected_chain->select_chain_operator_parameter(n + 1);
      result.push_back(selected_chain->chain_operator_parameter_name());
    }
    selected_chain->select_chain_operator_parameter(save_selected_copp);
  }
  return(result);
}

/**
 * Selects chain operator parameter 'param'.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 *  get_chain_operator() != 0
 *  param > 0
 */
void ECA_CONTROL_OBJECTS::select_chain_operator_parameter(int param) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(param > 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    if (selected_chainsetup() == connected_chainsetup()) {
      session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_copp_select, param);
    }
    else {
      session_repp->active_chainop_param_index_rep = param;
    }
    selected_chainsetup_repp->chains[p]->select_chain_operator_parameter(param);

  } 
}


/**
 * Sets chain operator parameter value
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 *  get_chain_operator() != 0
 */
void ECA_CONTROL_OBJECTS::set_chain_operator_parameter(CHAIN_OPERATOR::parameter_type value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(get_chain_operator() != 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    if (selected_chainsetup() == connected_chainsetup()) {
      session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_copp_value, value);
    }
    else {
      selected_chainsetup_repp->chains[p]->set_parameter(value);
    }
  }
}

/**
 * Returns the selected chain operator parameter value
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 *  get_chain_operator() != 0
 */
CHAIN_OPERATOR::parameter_type ECA_CONTROL_OBJECTS::get_chain_operator_parameter(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(get_chain_operator() != 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    return(selected_chainsetup_repp->chains[p]->get_parameter());
  }
  return(0.0f);
}

/**
 * Returns the index number of selected  chain operator parameter.
 * If no parameter is selected, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 *  get_chain_operator() != 0
 */
int ECA_CONTROL_OBJECTS::selected_chain_operator_parameter(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(get_chain_operator() != 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    return(selected_chainsetup_repp->chains[p]->selected_chain_operator_parameter());
  }
  return(0);
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
    set_last_error(e.error_section() + ": \"" + e.error_message() + "\"");
  }

  if (was_running == true)
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/**
 * Selects the Nth controller.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 *  controller_id > 0
 */
void ECA_CONTROL_OBJECTS::select_controller(int controller_id) { 
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

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    selected_chainsetup_repp->chains[p]->select_controller(controller_id);
  }
  
  if (was_running == true)
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/**
 * Removes the selected controller.
 *
 * require:
 *  is_selected() == true
 *  connected_chainsetup() != selected_chainsetup()
 *  selected_chains().size() == 1
 *  get_controller() != 0
 */
void ECA_CONTROL_OBJECTS::remove_controller(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(get_controller() != 0);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    selected_chainsetup_repp->chains[p]->remove_controller();
  }

  if (was_running == true)
    session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_start, 0.0);
}

/** 
 * Returns a list of controller names.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
vector<string> ECA_CONTROL_OBJECTS::controller_names(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  // --------

  vector<string> result; 
  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    CHAIN* selected_chain = selected_chainsetup_repp->chains[p];
    int save_selected_ctrl = selected_chain->selected_controller();
    for(int n = 0; 
	n < selected_chain->number_of_controllers();
	n++) {
      selected_chain->select_controller(n + 1);
      result.push_back(selected_chain->controller_name());
    }
    selected_chain->select_controller(save_selected_ctrl);
  }
  return(result);
}

/** 
 * Returns a pointer to the selected controller. If no controller is 
 * selected, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
GENERIC_CONTROLLER* ECA_CONTROL_OBJECTS::get_controller(void) const {
  // --------
  REQUIRE(is_selected() == true);
  REQUIRE(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    return (selected_chainsetup_repp->chains[p]->selected_controller_repp);

  return(0);
}

/** 
 * Returns the index number of the selected controller. If no controller is
 * selected, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
int ECA_CONTROL_OBJECTS::selected_controller(void) const {
  // --------
  REQUIRE(is_selected() == true);
  REQUIRE(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    return (selected_chainsetup_repp->chains[p]->selected_controller());

  return(0);
}
