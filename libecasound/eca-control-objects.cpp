// ------------------------------------------------------------------------
// eca-control-objects.cpp: Class for configuring libecasound objects
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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h> /* getenv() */
#include <cstdio>

#include <kvutils/dbc.h> /* DBC_* */
#include <kvutils/value_queue.h>
#include <kvutils/temporary_file_directory.h>

#include "eca-engine.h"
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
void ECA_CONTROL_OBJECTS::add_chainsetup(const std::string& name) {
  // --------
  DBC_REQUIRE(name != "");
  // --------

  bool no_errors = true;
  int count = static_cast<int>(session_repp->chainsetups_rep.size());
  session_repp->add_chainsetup(name);
  if (static_cast<int>(session_repp->chainsetups_rep.size()) > count) {
    select_chainsetup(name);
    ecadebug->msg("(eca-controller) Added a new chainsetup with name \"" + name + "\".");
  }
  else {
    set_last_error("(eca-contol-objects) Unable to add chainsetup with name \"" + name + "\".");
    no_errors = false;
  }

  // --------
  DBC_ENSURE(selected_chainsetup() == name || (last_error().size() > 0 && no_errors != true));
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
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(is_selected() == true);
  // --------

  ecadebug->msg("(eca-controller) Removing chainsetup:  \"" + selected_chainsetup() + "\".");
  session_repp->remove_chainsetup();
  selected_chainsetup_repp = 0;

  // --------
  DBC_ENSURE(selected_chainsetup().empty() == true);
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
void ECA_CONTROL_OBJECTS::load_chainsetup(const std::string& filename) {
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
void ECA_CONTROL_OBJECTS::save_chainsetup(const std::string& filename) {
  // --------
  DBC_REQUIRE(selected_chainsetup().empty() != true);
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
void ECA_CONTROL_OBJECTS::select_chainsetup(const std::string& name) {
  // --------
  DBC_REQUIRE(name != "");
  // --------

  session_repp->select_chainsetup(name);
  selected_chainsetup_repp = session_repp->selected_chainsetup_repp;
  if (selected_chainsetup_repp == 0) {
    ecadebug->msg("(eca-controller) Chainsetup \"" + name + "\" doesn't exist!");
    set_last_error("Chainsetup \"" + name + "\" doesn't exist!");
  }
  //  else { ecadebug->msg("(eca-controller) Selected chainsetup:  \"" + selected_chainsetup() + "\"."); }

  // --------
  DBC_ENSURE(name == selected_chainsetup() ||
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
  DBC_REQUIRE(index_number > 0);
  // --------

  for(std::vector<ECA_CHAINSETUP*>::size_type p = 0; 
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
  DBC_REQUIRE(selected_chainsetup().empty() != true);
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
	set_chainsetup_position(pos);
	if (is_connected() == true) {
	  if (restart == true) start();
	}
	select_chainsetup("cs-edit-temp");
      }
      else {
	connect_chainsetup();
	set_chainsetup_position(pos);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
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
  DBC_REQUIRE(output_mode == AUDIO_IO::io_write || output_mode == AUDIO_IO::io_readwrite);
  // --------
  selected_chainsetup_repp->set_output_openmode(output_mode);
}

/**
 * Sets chainsetup buffersize (in samples).
 *
 * @pre is_selected() == true
 */
void ECA_CONTROL_OBJECTS::set_chainsetup_buffersize(int bsize) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------
  selected_chainsetup_repp->set_buffersize(bsize); 
}

/**
 * Toggles chainsetup looping
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::toggle_chainsetup_looping(void) {
 // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
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
  DBC_REQUIRE(is_selected());
  DBC_REQUIRE(is_valid());
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
  DBC_ENSURE(is_connected() || no_errors != true);
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
  DBC_REQUIRE(is_connected());
  // --------

  if (is_engine_started() == true) {
    stop_on_condition();
    close_engine();
  }

  ecadebug->msg("(eca-controller) Disconnecting chainsetup:  \"" + connected_chainsetup() + "\".");
  session_repp->disconnect_chainsetup();

  // --------
  DBC_ENSURE(connected_chainsetup() == "");
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
  DBC_REQUIRE(is_selected());
  // --------

  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    if (seconds < 0)
      send_chain_commands_to_engine(ECA_ENGINE::ep_rewind, 
				    -seconds);
    else
      send_chain_commands_to_engine(ECA_ENGINE::ep_forward,
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
  DBC_REQUIRE(is_selected());
  // --------

  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    send_chain_commands_to_engine(ECA_ENGINE::ep_setpos, seconds);
  }
  else {
    selected_chainsetup_repp->set_position_exact(seconds);
    selected_chainsetup_repp->seek_position();
  }
}

/**
 * Gets a vector of al chainsetup names.
 */
std::vector<std::string> ECA_CONTROL_OBJECTS::chainsetup_names(void) const {
  return(session_repp->chainsetup_names());
}

/**
 * Gets a pointer to selected chainsetup, or 0 if no 
 * chainsetup is selected.
 */
const ECA_CHAINSETUP* ECA_CONTROL_OBJECTS::get_chainsetup(void) const {
  return(selected_chainsetup_repp);
}

/**
 * Gets a pointer to chainsetup with filename 'filename'.
 */
const ECA_CHAINSETUP* ECA_CONTROL_OBJECTS::get_chainsetup_filename(const std::string&
							      filename) const {
  std::vector<ECA_CHAINSETUP*>::const_iterator p = session_repp->chainsetups_rep.begin();
  while(p != session_repp->chainsetups_rep.end()) {
    if ((*p)->filename() == filename) {
      return((*p));
    }
    ++p;
    }
  return(0);
}

/**
 * Returns current buffersize of selected chainsetup.
 *
 * ®pre is_selected() == true
 */
int ECA_CONTROL_OBJECTS::chainsetup_buffersize(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->buffersize());
}

/** 
 * Gets chainsetup filename (used by save_chainsetup())
 *
 * ®pre is_selected() == true
 */
const std::string& ECA_CONTROL_OBJECTS::chainsetup_filename(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
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
void ECA_CONTROL_OBJECTS::set_chainsetup_filename(const std::string& name) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(name.empty() != true);
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
void ECA_CONTROL_OBJECTS::set_chainsetup_parameter(const std::string& name) {
  // --------
  DBC_REQUIRE(is_selected() == true  && 
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
void ECA_CONTROL_OBJECTS::set_chainsetup_sample_format(const std::string& name) {
  // --------
  DBC_REQUIRE(is_selected() == true  && 
	 name.empty() != true);
  // --------

  selected_chainsetup_repp->interpret_object_option("-f:" + name);
  if (selected_chainsetup_repp->interpret_result() != true) {
    set_last_error(selected_chainsetup_repp->interpret_result_verbose());
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
void ECA_CONTROL_OBJECTS::add_chain(const std::string& name) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chainsetup() != connected_chainsetup());
  // --------

  add_chains(std::vector<std::string> (1, name));

  // --------
  DBC_ENSURE(selected_chains().size() > 0);
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
void ECA_CONTROL_OBJECTS::add_chains(const std::string& names) { 
  // --------
  DBC_REQUIRE(is_selected() == true &&
	 is_connected() == false);
  // --------

  add_chains(string_to_vector(names, ','));
  
  // --------
  DBC_ENSURE(selected_chains().size() > 0);
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
void ECA_CONTROL_OBJECTS::add_chains(const std::vector<std::string>& new_chains) { 
  // --------
  DBC_REQUIRE(is_selected() == true &&
	 is_connected() == false);
  // --------

  selected_chainsetup_repp->add_new_chains(new_chains);
  selected_chainsetup_repp->select_chains(new_chains);

  ecadebug->msg("(eca-controller) Added chains: " +
		vector_to_string(new_chains, ", ") + ".");

  // --------
  DBC_ENSURE(selected_chains().size() == new_chains.size());
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
  DBC_REQUIRE(is_selected() == true &&
	 selected_chains().size() > 0 &&
	 is_connected() == false);
  // --------

  selected_chainsetup_repp->remove_chains();

  ecadebug->msg("(eca-controlled) Removed selected chains.");

  // --------
  DBC_ENSURE(selected_chains().size() == 0);
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
void ECA_CONTROL_OBJECTS::select_chains_by_index(const std::vector<int>& index_numbers) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
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
void ECA_CONTROL_OBJECTS::select_chain(const std::string& chain) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  std::vector<std::string> c (1);
  c[0] = chain;
  selected_chainsetup_repp->select_chains(c);
  //  ecadebug->msg(ECA_DEBUG::user_objects, "(eca-controller) Selected chain: " + chain + ".");

  // --------
  DBC_ENSURE(selected_chains().size() == 1);
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
void ECA_CONTROL_OBJECTS::select_chains(const std::vector<std::string>& chains) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  selected_chainsetup_repp->select_chains(chains);

  //  ecadebug->msg(ECA_DEBUG::user_objects, "(eca-controller) Selected chains: " +
//  		vector_to_string(chains, ", ") + ".");
}

/**
 * Deselects chains (currently selected chainsetup)
 *
 * @param chains vector of chain names
 *
 * require:
 *   is_selected() == true
 */
void ECA_CONTROL_OBJECTS::deselect_chains(const std::vector<std::string>& chains) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  std::vector<std::string> schains = selected_chainsetup_repp->selected_chains();
  std::vector<std::string>::const_iterator p = chains.begin();
  while(p != chains.end()) {
    std::vector<std::string>::iterator o = schains.begin();
    while(o != schains.end()) {
      if (*p == *o) {
	//  ecadebug->msg("(eca-controller-objects) Deselected chain " + *o  + ".");
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
  DBC_REQUIRE(is_selected() == true);
  // --------

  selected_chainsetup_repp->select_all_chains();

  //  ecadebug->msg("(eca-controller) Selected chains: " + vector_to_string(selected_chains(), ", ") + ".");
}

/**
 * Returns a list of selected chains (currently selected chainsetup)
 *
 * require:
 *  is_selected() == true
 */
const std::vector<std::string>& ECA_CONTROL_OBJECTS::selected_chains(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->selected_chains());
}

/**
 * Gets a vector of all chain names.
 *
 * require:
 *  is_selected() == true
 */
std::vector<std::string> ECA_CONTROL_OBJECTS::chain_names(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
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
const CHAIN* ECA_CONTROL_OBJECTS::get_chain(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------
  const std::vector<std::string>& schains = selected_chainsetup_repp->selected_chains();
  std::vector<std::string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(std::vector<CHAIN*>::size_type p = 0; 
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
 *  is_running() != true
 */
void ECA_CONTROL_OBJECTS::clear_chains(void) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(is_running() != true);
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
void ECA_CONTROL_OBJECTS::rename_chain(const std::string& name) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------
  selected_chainsetup_repp->rename_chain(name);     
}

void ECA_CONTROL_OBJECTS::send_chain_commands_to_engine(int command, double value) {
  // --------
  DBC_CHECK(is_engine_started() == true);
  // --------
  if (is_engine_started() != true) return; 

  const std::vector<std::string>& schains = selected_chainsetup_repp->selected_chains();

  std::vector<std::string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(std::vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_repp->chains.size();
	p++) {
      if (selected_chainsetup_repp->chains[p]->name() == *o) {
	engine_repp->command(ECA_ENGINE::ep_c_select, p);
	engine_repp->command(static_cast<ECA_ENGINE::Engine_command_t>(command), value);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    send_chain_commands_to_engine(ECA_ENGINE::ep_c_mute, 0.0);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    send_chain_commands_to_engine(ECA_ENGINE::ep_c_bypass, 0.0);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    send_chain_commands_to_engine(ECA_ENGINE::ep_c_rewind, pos_in_seconds);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    send_chain_commands_to_engine(ECA_ENGINE::ep_c_forward, pos_in_seconds);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    send_chain_commands_to_engine(ECA_ENGINE::ep_c_setpos, change_in_seconds);
  }
  else {
    const std::vector<std::string>& schains = selected_chainsetup_repp->selected_chains();
    std::vector<std::string>::const_iterator o = schains.begin();
    while(o != schains.end()) {
      for(std::vector<CHAIN*>::size_type p = 0; 
	  p != selected_chainsetup_repp->chains.size();
	  p++) {
	if (selected_chainsetup_repp->chains[p]->name() == *o) {
	  double previous = selected_chainsetup_repp->inputs[selected_chainsetup_repp->chains[p]->connected_input()]->position_in_seconds_exact();
	  selected_chainsetup_repp->inputs[selected_chainsetup_repp->chains[p]->connected_input()]->seek_position_in_seconds(previous + change_in_seconds);
	  previous = selected_chainsetup_repp->outputs[selected_chainsetup_repp->chains[p]->connected_output()]->position_in_seconds_exact();
	  selected_chainsetup_repp->outputs[selected_chainsetup_repp->chains[p]->connected_output()]->seek_position_in_seconds(previous + change_in_seconds);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup() && is_engine_started() == true) {
    send_chain_commands_to_engine(ECA_ENGINE::ep_c_setpos, pos_in_seconds);
  }
  else {
    const std::vector<std::string>& schains = selected_chainsetup_repp->selected_chains();
    std::vector<std::string>::const_iterator o = schains.begin();
    while(o != schains.end()) {
      for(std::vector<CHAIN*>::size_type p = 0; 
	  p != selected_chainsetup_repp->chains.size();
	  p++) {
	if (selected_chainsetup_repp->chains[p]->name() == *o) {
	  selected_chainsetup_repp->inputs[selected_chainsetup_repp->chains[p]->connected_input()]->seek_position_in_seconds(pos_in_seconds);
	  selected_chainsetup_repp->outputs[selected_chainsetup_repp->chains[p]->connected_output()]->seek_position_in_seconds(pos_in_seconds);
	break;
	}
      }
      ++o;
    }
  }
}

void ECA_CONTROL_OBJECTS::audio_input_as_selected(void) {
  /* note, here we check that the pointer is still a valid one */
  if (selected_chainsetup_repp->ok_audio_object(selected_audio_input_repp) != true)
    selected_audio_input_repp = 0;

  selected_audio_object_repp = selected_audio_input_repp;
}

void ECA_CONTROL_OBJECTS::audio_output_as_selected(void) {
  /* note, here we check that the pointer is still a valid one */
  if (selected_chainsetup_repp->ok_audio_object(selected_audio_output_repp) != true)
    selected_audio_output_repp = 0;

  selected_audio_object_repp = selected_audio_output_repp;
}

/**
 * Sets default audio format. This format will be used, when
 * adding audio inputs and outputs.
 *
 * require:
 *  is_selected() == true
 */
void ECA_CONTROL_OBJECTS::set_default_audio_format(const std::string& sfrm,
						   int channels, 
						   long int srate,
						   bool interleaving) {
 // --------
  DBC_REQUIRE(is_selected() == true);
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

  selected_chainsetup_repp->interpret_object_option(format);
  if (selected_chainsetup_repp->interpret_result() != true) {
    set_last_error(selected_chainsetup_repp->interpret_result_verbose());
  }
}

/**
 * Returns the default audio format.
 *
 *  @pre is_selected() == true
 */
const ECA_AUDIO_FORMAT& ECA_CONTROL_OBJECTS::default_audio_format(void) const {
  // --
  DBC_REQUIRE(is_selected() == true);
  // --
  return(selected_chainsetup_repp->default_audio_format());
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
  DBC_REQUIRE(is_selected() == true);
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
void ECA_CONTROL_OBJECTS::select_audio_input(const std::string& name) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  selected_audio_input_repp = 0;
  std::vector<AUDIO_IO*>::size_type p = 0;  
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
void ECA_CONTROL_OBJECTS::select_audio_output(const std::string& name) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  selected_audio_output_repp = 0;
  std::vector<AUDIO_IO*>::size_type p = 0;  
  for(p = 0; p != selected_chainsetup_repp->outputs.size(); p++) {
    if (selected_chainsetup_repp->outputs[p]->label() == name) {
      selected_audio_output_repp = selected_chainsetup_repp->outputs[p];
    }
  }
}

/**
 * Selects an audio input by index.
 *
 * @pre is_selected() == true
 * @pre index_number > 0
 */
void ECA_CONTROL_OBJECTS::select_audio_input_by_index(int index_number) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(index_number > 0);
  // --------

  selected_audio_input_repp = 0;

  if (index_number <= static_cast<int>(selected_chainsetup_repp->inputs.size()))
    selected_audio_input_repp = selected_chainsetup_repp->inputs[index_number-1];
}

/**
 * Selects an audio output by index.
 *
 * @pre is_selected() == true
 * @pre index_number > 0
 */
void ECA_CONTROL_OBJECTS::select_audio_output_by_index(int index_number) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(index_number > 0);
  // --------

  selected_audio_output_repp = 0;

  if (index_number <= static_cast<int>(selected_chainsetup_repp->outputs.size()))
    selected_audio_output_repp = selected_chainsetup_repp->outputs[index_number-1];
}

/**
 * Gets audio format information of the object given as argument.
 * Note! To get audio format information, audio objects need
 * to be opened. Because of this, object argument cannot be given 
 * as a const pointer.
 */
ECA_AUDIO_FORMAT ECA_CONTROL_OBJECTS::get_audio_format(AUDIO_IO* aobj) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(aobj != 0);
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
 * Sets the default audio format to the match the currently 
 * select audio input's audio format.
 *
 *  @pre is_selected() == true
 *  @pre connected_chainsetup() != selected_chainsetup()
 *  @pre selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::set_default_audio_format_to_selected_input(void) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0);
  // --------
  set_default_audio_format(get_audio_format(selected_audio_input_repp));

}

/**
 * Sets the default audio format to the match the currently 
 * select audio output's audio format.
 *
 *  @pre is_selected() == true
 *  @pre connected_chainsetup() != selected_chainsetup()
 *  @pre selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::set_default_audio_format_to_selected_output(void) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_output() != 0);
  // --------
  set_default_audio_format(get_audio_format(selected_audio_output_repp));
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
void ECA_CONTROL_OBJECTS::add_audio_input(const std::string& filename) {
  // --------
  DBC_REQUIRE(filename.empty() == false);
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  // --------

  selected_audio_input_repp = 0;
  selected_chainsetup_repp->interpret_object_option("-i:" + filename);
  if (selected_chainsetup_repp->interpret_result() == true) {
    select_audio_input(filename);
    ecadebug->msg("(eca-controller) Added audio input \"" + filename + "\".");
  }
  else {
    set_last_error(selected_chainsetup_repp->interpret_result_verbose());
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
void ECA_CONTROL_OBJECTS::add_audio_output(const std::string& filename) {
  // --------
  DBC_REQUIRE(filename.empty() == false);
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  // --------

  selected_audio_output_repp = 0;
  selected_chainsetup_repp->interpret_object_option("-o:" + filename);
  if (selected_chainsetup_repp->interpret_result() == true) {
    select_audio_output(filename);
    ecadebug->msg("(eca-controller) Added audio output \"" + filename +
		  "\".");
  } else {
    set_last_error(selected_chainsetup_repp->interpret_result_verbose());
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
  DBC_REQUIRE(selected_chains().size() > 0);
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  // --------
  add_audio_output(resource_value("default-output"));
  ecadebug->msg("(eca-controller) Added default output to selected chains.");
}

/**
 * Gets a vector of all audio input names.
 *
 * require:
 *  is_selected() == true
 */
std::vector<std::string> ECA_CONTROL_OBJECTS::audio_input_names(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------
  return(selected_chainsetup_repp->audio_input_names());
}

/**
 * Gets a vector of all audio output names.
 *
 * require:
 *  is_selected() == true
 */
std::vector<std::string> ECA_CONTROL_OBJECTS::audio_output_names(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
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
const AUDIO_IO* ECA_CONTROL_OBJECTS::get_audio_input(void) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  /* note, here we check that the pointer is still a valid one */
  if (selected_chainsetup_repp->ok_audio_object(selected_audio_input_repp) != true)
    selected_audio_input_repp = 0;

  return(selected_audio_input_repp);
}

/** 
 * Gets a pointer to the currently selected audio output.
 * Returns 0 if no audio object is selected.
 *
 * require:
 *  is_selected() == true
 */
const AUDIO_IO* ECA_CONTROL_OBJECTS::get_audio_output(void) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------
  /* note, here we check that the pointer is still a valid one */
  if (selected_chainsetup_repp->ok_audio_object(selected_audio_output_repp) != true)
    selected_audio_output_repp = 0;

  return(selected_audio_output_repp);
}

/**
 * Removes the selected audio input.
 *
 *  @pre is_selected() == true
 *  @pre connected_chainsetup() != selected_chainsetup()
 *  @pre get_audio_input() != 0
 *
 *  @post selected_audio_input_repp = 0
 */
void ECA_CONTROL_OBJECTS::remove_audio_input(void) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0);
  // --------
  ecadebug->msg("(eca-controller) Removing selected audio input \"" + selected_audio_input_repp->label() +
		"\" from selected chains.");
  selected_chainsetup_repp->remove_audio_input(selected_audio_input_repp->label());
  selected_audio_input_repp = 0;

  // --------
  DBC_ENSURE(selected_audio_input_repp == 0);
  // --------
}

/**
 * Removes the selected audio output.
 *
 *  @pre is_selected() == true
 *  @pre connected_chainsetup() != selected_chainsetup()
 *  @pre get_audio_output() != 0
 *
 *  @post selected_audio_output_repp = 0
 */
void ECA_CONTROL_OBJECTS::remove_audio_output(void) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_output() != 0);
  // --------
  ecadebug->msg("(eca-controller) Removing selected audio output \"" + selected_audio_output_repp->label() +
		"\" from selected chains.");
  selected_chainsetup_repp->remove_audio_output(selected_audio_output_repp->label());
  selected_audio_output_repp = 0;

  // --------
  DBC_ENSURE(selected_audio_output_repp == 0);
  // --------
}

/**
 * Attaches selected audio input to selected chains
 *
 * @pre is_selected() == true
 * @pre connected_chainsetup() != selected_chainsetup()
 * @pre get_audio_input() != 0
 */
void ECA_CONTROL_OBJECTS::attach_audio_input(void) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0);
  // --------
  selected_chainsetup_repp->attach_input_to_selected_chains(selected_audio_input_repp);

  ecadebug->msg("(eca-controller) Attached audio input \"" + selected_audio_input_repp->label() +
		"\" to selected chains.");
}

/**
 * Attaches selected audio output to selected chains
 *
 *  @pre is_selected() == true
 *  @pre connected_chainsetup() != selected_chainsetup()
 *  @pre get_audio_output() != 0
 */
void ECA_CONTROL_OBJECTS::attach_audio_output(void) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_output() != 0);
  // --------
  selected_chainsetup_repp->attach_output_to_selected_chains(selected_audio_output_repp);

  ecadebug->msg("(eca-controller) Attached audio output \"" + selected_audio_output_repp->label() +
		"\" to selected chains.");
}

/**
 * Rewinds selected audio object by 'pos_in_seconds' seconds
 *
 *  @pre is_selected() == true
 *  @pre connected_chainsetup() != selected_chainsetup()
 *  @pre selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::rewind_audio_object(double seconds) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0 || get_audio_output() != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(selected_audio_object_repp->position_in_seconds_exact() - seconds);
}

/**
 * Forwards selected audio object by 'pos_in_seconds' seconds
 *
 *  @pre is_selected() == true
 *  @pre connected_chainsetup() != selected_chainsetup()
 *  @pre selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::forward_audio_object(double seconds) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0 || get_audio_output() != 0);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0 || get_audio_output() != 0);
  // --------
  selected_audio_object_repp->seek_position_in_seconds(seconds);
}

/**
 * Sets position of selected audio object
 *
 * @pre is_selected() == true
 * @pre connected_chainsetup() != selected_chainsetup()
 * @pre selected_audio_object_repp != 0
 */
void ECA_CONTROL_OBJECTS::set_audio_object_position_samples(long int samples) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0 || get_audio_output() != 0);
  // --------
  selected_audio_object_repp->seek_position_in_samples(samples);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(connected_chainsetup() != selected_chainsetup());
  DBC_REQUIRE(get_audio_input() != 0 || get_audio_output() != 0);
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
void ECA_CONTROL_OBJECTS::add_chain_operator(const std::string& chainop_params) {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------
  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  if (chainop_params[0] == '-') {
    selected_chainsetup_repp->interpret_object_option(chainop_params);
    if (selected_chainsetup_repp->interpret_result() != true) {
      set_last_error(selected_chainsetup_repp->interpret_result_verbose());
    }
  }

  if (is_engine_started() == true && was_running == true)
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(cotmp != 0);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  selected_chainsetup_repp->add_chain_operator(cotmp);

  if (is_engine_started() == true && was_running == true)
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);
}

/** 
 * Returns a pointer to the the selected chain operator. If no chain 
 * operator is selected, 0 is returned.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
const CHAIN_OPERATOR* ECA_CONTROL_OBJECTS::get_chain_operator(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    return (selected_chainsetup_repp->chains[p]->get_selected_chain_operator());

  return(0);
}

/** 
 * Returns a list of chain operator names.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
std::vector<std::string> ECA_CONTROL_OBJECTS::chain_operator_names(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------

  std::vector<std::string> result; 
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    selected_chainsetup_repp->chains[p]->remove_chain_operator();

  if (is_engine_started() == true && was_running == true)
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(chainop_id > 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    if (is_engine_started() == true &&
	selected_chainsetup() == connected_chainsetup()) {
      engine_repp->command(ECA_ENGINE::ep_c_select, p);
      engine_repp->command(ECA_ENGINE::ep_cop_select, chainop_id);
    }
    if (chainop_id < selected_chainsetup_repp->chains[p]->number_of_chain_operators() + 1) {
      selected_chainsetup_repp->chains[p]->select_chain_operator(chainop_id);
      selected_chainsetup_repp->active_chain_index_rep = p;
      selected_chainsetup_repp->active_chainop_index_rep = chainop_id;
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
std::vector<std::string> ECA_CONTROL_OBJECTS::chain_operator_parameter_names(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(get_chain_operator() != 0);
  // --------

  std::vector<std::string> result; 
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(param > 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    if (is_engine_started() == true && 
	selected_chainsetup() == connected_chainsetup()) {
      engine_repp->command(ECA_ENGINE::ep_copp_select, param);
    }
    else {
      selected_chainsetup_repp->active_chainop_param_index_rep = param;
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(get_chain_operator() != 0);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    if (is_engine_started() == true && 
	selected_chainsetup() == connected_chainsetup()) {
      engine_repp->command(ECA_ENGINE::ep_copp_value, value);
    }
    else {
      if (selected_chainsetup_repp->chains[p]->selected_chain_operator() > 0 &&
	  selected_chainsetup_repp->chains[p]->selected_chain_operator_parameter() > 0)
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size()) {
    if (selected_chainsetup_repp->chains[p]->selected_chain_operator() > 0 &&
	selected_chainsetup_repp->chains[p]->selected_chain_operator_parameter() > 0)
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(get_chain_operator() != 0);
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
void ECA_CONTROL_OBJECTS::add_controller(const std::string& gcontrol_params) { 
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() > 0);
  // --------

  bool was_running = false;
  if (selected_chainsetup() == connected_chainsetup() && is_running() == true) {
    was_running = true;
    stop_on_condition();
  }

  if (gcontrol_params[0] == '-') {
      selected_chainsetup_repp->interpret_object_option(gcontrol_params);
      if (selected_chainsetup_repp->interpret_result() != true) {
	set_last_error(selected_chainsetup_repp->interpret_result_verbose());
      }
  }

  if (is_engine_started() == true && was_running == true)
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(controller_id > 0);
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
  
  if (is_engine_started() == true && was_running == true)
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);
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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  DBC_REQUIRE(get_controller() != 0);
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

  if (is_engine_started() == true && was_running == true)
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);
}

/** 
 * Returns a list of controller names.
 *
 * require:
 *  is_selected() == true
 *  selected_chains().size() == 1
 */
std::vector<std::string> ECA_CONTROL_OBJECTS::controller_names(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------

  std::vector<std::string> result; 
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
const GENERIC_CONTROLLER* ECA_CONTROL_OBJECTS::get_controller(void) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    return (selected_chainsetup_repp->chains[p]->get_selected_controller());

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
  DBC_REQUIRE(is_selected() == true);
  DBC_REQUIRE(selected_chains().size() == 1);
  // --------

  unsigned int p = selected_chainsetup_repp->first_selected_chain();
  if (p < selected_chainsetup_repp->chains.size())
    return (selected_chainsetup_repp->chains[p]->selected_controller());

  return(0);
}
