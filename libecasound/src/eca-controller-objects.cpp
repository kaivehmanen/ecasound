// ------------------------------------------------------------------------
// eca-controller-objects.cpp: Class for configuring libecasound objects
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
#include "eca-controller-objects.h"

#include "eca-error.h"
#include "eca-debug.h"

ECA_CONTROLLER_OBJECTS::ECA_CONTROLLER_OBJECTS (ECA_SESSION* psession) 
  : ECA_CONTROLLER_BASE(psession) {

  selected_chainop_rep = 0;
  selected_audio_object_rep = 0;
}

void ECA_CONTROLLER_OBJECTS::add_chainsetup(const string& name) {
  // --------
  require(name != "", __FILE__, __LINE__);
  // --------

  session_rep->add_chainsetup(name);
  select_chainsetup(name);
  ecadebug->msg("(eca-controller) Added a new chainsetup with name \"" + name + "\".");

  // --------
  ensure(selected_chainsetup() == name,__FILE__, __LINE__);
  // --------
}

void ECA_CONTROLLER_OBJECTS::remove_chainsetup(void) {
  // --------
  // require:
  assert(connected_chainsetup() != selected_chainsetup());
  assert(is_selected() == true);
  // --------

  ecadebug->msg("(eca-controller) Removing chainsetup:  \"" + selected_chainsetup() + "\".");
  session_rep->remove_chainsetup();
  selected_chainsetup_rep = 0;

  // --------
  // ensure:
  assert(selected_chainsetup().empty() == true);
  // --------
}

void ECA_CONTROLLER_OBJECTS::load_chainsetup(const string& filename) {
  session_rep->load_chainsetup(filename);
  select_chainsetup(get_chainsetup_filename(filename)->name());
  ecadebug->msg("(eca-controller) Loaded chainsetup from file \"" + filename + "\".");
}

void ECA_CONTROLLER_OBJECTS::save_chainsetup(const string& filename) {
  // --------
  // require:
  assert(selected_chainsetup().empty() != true);
  // --------

  if (filename.empty() == true) 
    session_rep->save_chainsetup();
  else 
    session_rep->save_chainsetup(filename);
  
  ecadebug->msg("(eca-controller) Saved selected chainsetup \"" + selected_chainsetup() + "\".");
}

void ECA_CONTROLLER_OBJECTS::select_chainsetup(const string& name) {
  // --------
  // require:
  assert(name != "");
  // --------

  session_rep->select_chainsetup(name);
  selected_chainsetup_rep = session_rep->selected_chainsetup;
  if (selected_chainsetup_rep != 0)
    ecadebug->msg("(eca-controller) Selected chainsetup:  \"" + selected_chainsetup() + "\".");
  else
    ecadebug->msg("(eca-controller) Chainsetup \"" + name + "\" doesn't exist!");

  // --------
  // ensure:
  assert(name == selected_chainsetup() ||
	 is_selected() == false);
  // --------
}

void ECA_CONTROLLER_OBJECTS::select_chainsetup_by_index(const string& index) { 
  // --------
  // require:
  assert(index.empty() != true);
  assert(index[0] == 'c');
  // --------

  int index_number = atoi(string(index.begin() + 1,
				 index.end()).c_str());

  for(vector<ECA_CHAINSETUP*>::size_type p = 0; 
      p != session_rep->chainsetups.size();
      p++) {
    if (index_number == static_cast<int>(p + 1)) {
      select_chainsetup(session_rep->chainsetups[p]->name());
      break;
    }
  }
}

string ECA_CONTROLLER_OBJECTS::selected_chainsetup(void) const {
 if (selected_chainsetup_rep != 0)
   return(selected_chainsetup_rep->name());

 return("");
}

void ECA_CONTROLLER_OBJECTS::edit_chainsetup(void) {
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
  string origname = selected_chainsetup_rep->name();
  string origfilename = selected_chainsetup_rep->filename();
  string filename = string(tmpnam(NULL));
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
  int res = system(editori.c_str());

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

void ECA_CONTROLLER_OBJECTS::set_chainsetup_processing_length_in_seconds(double value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(value > 0.0);
  // --------
  selected_chainsetup_rep->length_in_seconds(value);
  ecadebug->msg("(eca-controller) Set processing length to \"" + kvu_numtostr(value) + "\" seconds.");
}

void ECA_CONTROLLER_OBJECTS::set_chainsetup_processing_length_in_samples(long int value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(value > 0);
  // --------
  selected_chainsetup_rep->length_in_samples(value);
  ecadebug->msg("(eca-controller) Set processing length to \"" + 
		 kvu_numtostr(selected_chainsetup_rep->length_in_seconds()) + "\" seconds.");
}

void ECA_CONTROLLER_OBJECTS::toggle_chainsetup_looping(void) {
 // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  if (selected_chainsetup_rep->looping_enabled()) {
    selected_chainsetup_rep->toggle_looping(false);
    ecadebug->msg("(eca-controller) Disabled looping.");
  }
  else {
    selected_chainsetup_rep->toggle_looping(true);
    ecadebug->msg("(eca-controller) Enabled looping.");
  }
}

void ECA_CONTROLLER_OBJECTS::connect_chainsetup(void) {
  // --------
  // require:
  assert(is_selected());
  assert(is_valid());
  // --------

  if (is_connected() == true) {
    disconnect_chainsetup();
  }
  session_rep->connect_chainsetup();
  ecadebug->msg("(eca-controller) Connected chainsetup:  \"" + connected_chainsetup() + "\".");

  // --------
  // ensure:
  assert(is_connected());
  // --------
}

string ECA_CONTROLLER_OBJECTS::connected_chainsetup(void) const {
  if (session_rep->connected_chainsetup != 0) {
    return(session_rep->connected_chainsetup->name());
  }

  return("");
}

void ECA_CONTROLLER_OBJECTS::disconnect_chainsetup(void) {
  // --------
  // require:
  assert(is_connected());
  // --------

  if (is_engine_started() == true) {
    stop();
    close_engine();
  }

  ecadebug->msg("(eca-controller) Disconnecting chainsetup:  \"" + connected_chainsetup() + "\".");
  session_rep->disconnect_chainsetup();

  // --------
  // ensure:
  assert(connected_chainsetup() == "");
  // --------
}


ECA_CHAINSETUP* ECA_CONTROLLER_OBJECTS::get_chainsetup(void) const {
  // --------
  REQUIRE(is_selected());
  // --------
  return(selected_chainsetup_rep);
}

ECA_CHAINSETUP* ECA_CONTROLLER_OBJECTS::get_chainsetup_filename(const string&
							      filename) const {
  vector<ECA_CHAINSETUP*>::const_iterator p = session_rep->chainsetups.begin();
  while(p != session_rep->chainsetups.end()) {
    if ((*p)->filename() == filename) {
      return((*p));
    }
    ++p;
    }
  return(0);
}

const string& ECA_CONTROLLER_OBJECTS::chainsetup_filename(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  return(selected_chainsetup_rep->filename());
}

void ECA_CONTROLLER_OBJECTS::set_chainsetup_filename(const string& name) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(name.empty() != true);
  // --------
  selected_chainsetup_rep->set_filename(name);
}

void ECA_CONTROLLER_OBJECTS::set_chainsetup_parameter(const string& name) {
  // --------
  // require:
  assert(is_selected() == true  && 
	 name.empty() != true);
  // --------

  selected_chainsetup_rep->interpret_general_option(name);
}

void ECA_CONTROLLER_OBJECTS::set_chainsetup_sample_format(const string& name) {
  // --------
  // require:
  assert(is_selected() == true  && 
	 name.empty() != true);
  // --------

  selected_chainsetup_rep->interpret_audio_format("-f:" + name);
}

void ECA_CONTROLLER_OBJECTS::add_chain(const string& name) { 
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

void ECA_CONTROLLER_OBJECTS::add_chains(const string& names) { 
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

void ECA_CONTROLLER_OBJECTS::add_chains(const vector<string>& new_chains) { 
  // --------
  // require:
  assert(is_selected() == true &&
	 is_connected() == false);
  // --------

  selected_chainsetup_rep->add_new_chains(new_chains);
  selected_chainsetup_rep->select_chains(new_chains);

  ecadebug->msg("(eca-controller) Added chains: " +
		vector_to_string(new_chains, ", ") + ".");

  // --------
  // ensure:
  assert(selected_chains().size() == new_chains.size());
  // --------
}

void ECA_CONTROLLER_OBJECTS::remove_chains(void) { 
  // --------
  // require:
  assert(is_selected() == true &&
	 selected_chains().size() > 0 &&
	 is_connected() == false);
  // --------

  selected_chainsetup_rep->remove_chains();

  ecadebug->msg("(eca-controlled) Removed selected chains.");

  // --------
  // ensure:
  assert(selected_chains().size() == 0);
  // --------
}

void ECA_CONTROLLER_OBJECTS::select_chains(const vector<string>& chains) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  selected_chainsetup_rep->select_chains(chains);

  ecadebug->msg("(eca-controller) Selected chains: " +
		vector_to_string(chains, ", ") + ".");
}

void ECA_CONTROLLER_OBJECTS::deselect_chains(const vector<string>& chains) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  vector<string> schains = selected_chainsetup_rep->selected_chains();
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

  selected_chainsetup_rep->select_chains(schains);
}

void ECA_CONTROLLER_OBJECTS::select_all_chains(void) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  selected_chainsetup_rep->select_all_chains();

  ecadebug->msg("(eca-controller) Selected chains: " +
		vector_to_string(selected_chains(), ", ") + ".");
}

const vector<string>& ECA_CONTROLLER_OBJECTS::selected_chains(void) const {
 // --------
  // require:
  assert(is_selected() == true);
  // --------

  return(selected_chainsetup_rep->selected_chains());
}

void ECA_CONTROLLER_OBJECTS::clear_chains(void) { 
 // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() > 0);
  // --------
  selected_chainsetup_rep->clear_chains();
}

void ECA_CONTROLLER_OBJECTS::rename_chain(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  // --------
  selected_chainsetup_rep->rename_chain(name);     
}

void ECA_CONTROLLER_OBJECTS::send_chain_commands_to_engine(int command, double value) {
  vector<string> schains = selected_chainsetup_rep->selected_chains();

  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_rep->chains.size();
	p++) {
      if (selected_chainsetup_rep->chains[p]->name() == *o) {
	ecasound_queue.push_back(ECA_PROCESSOR::ep_c_select, p);
	ecasound_queue.push_back(command, value);
	break;
      }
    }
    ++o;
  }
}

void ECA_CONTROLLER_OBJECTS::toggle_chain_muting(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_mute, 0.0);
  } 
  else {
    selected_chainsetup_rep->toggle_chain_muting();
  }
}

void ECA_CONTROLLER_OBJECTS::toggle_chain_bypass(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_bypass, 0.0);
  }
  else {
    selected_chainsetup_rep->toggle_chain_bypass();
  }
}

void ECA_CONTROLLER_OBJECTS::rewind_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_rewind, pos_in_seconds);
}

void ECA_CONTROLLER_OBJECTS::forward_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_forward, pos_in_seconds);
}

void ECA_CONTROLLER_OBJECTS::set_position_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  if (connected_chainsetup() == selected_chainsetup()) {
    send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_setpos, pos_in_seconds);
  }
  else {
    vector<string> schains = selected_chainsetup_rep->selected_chains();
    vector<string>::const_iterator o = schains.begin();
    while(o != schains.end()) {
      for(vector<CHAIN*>::size_type p = 0; 
	  p != selected_chainsetup_rep->chains.size();
	  p++) {
	if (selected_chainsetup_rep->chains[p]->name() == *o) {
	  selected_chainsetup_rep->chains[p]->input_id->seek_position_in_seconds(pos_in_seconds);
	  selected_chainsetup_rep->chains[p]->output_id->seek_position_in_seconds(pos_in_seconds);
	break;
	}
      }
      ++o;
    }
  }
}

void ECA_CONTROLLER_OBJECTS::set_default_audio_format(const string& sfrm,
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

  selected_chainsetup_rep->interpret_audio_format(format);
}

void ECA_CONTROLLER_OBJECTS::set_default_audio_format(const ECA_AUDIO_FORMAT* format) {
 // --------
  // require:
  assert(is_selected() == true);
  // --------

  set_default_audio_format(format->format_string(), 
			   static_cast<int>(format->channels()), 
			   static_cast<long int>(format->samples_per_second()));
}

void ECA_CONTROLLER_OBJECTS::select_audio_object(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  select_audio_input(name);
  select_audio_output(name);
}

void ECA_CONTROLLER_OBJECTS::select_audio_input(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  vector<AUDIO_IO*>::size_type p = 0;  
  for(p = 0; p != selected_chainsetup_rep->inputs.size(); p++) {
    if (selected_chainsetup_rep->inputs[p]->label() == name) {
      selected_audio_object_rep = selected_chainsetup_rep->inputs[p];
    }
  }
}

void ECA_CONTROLLER_OBJECTS::select_audio_output(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  vector<AUDIO_IO*>::size_type p = 0;  
  for(p = 0; p != selected_chainsetup_rep->outputs.size(); p++) {
    if (selected_chainsetup_rep->outputs[p]->label() == name) {
      selected_audio_object_rep = selected_chainsetup_rep->outputs[p];
    }
  }
}

void ECA_CONTROLLER_OBJECTS::select_audio_object_by_index(const string& index) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(index.empty() != true);
  assert(index[0] == 'i' || index[0] == 'o');
  // --------

  int index_number = atoi(string(index.begin() + 1,
				 index.end()).c_str());

  vector<AUDIO_IO*>::size_type p = 0;
  if (index[0] == 'i') {
    for(p = 0; p != selected_chainsetup_rep->inputs.size(); p++) {
      if (index_number == static_cast<int>(p + 1)) {
	selected_audio_object_rep = selected_chainsetup_rep->inputs[p];
      }
    }
  }  
  else if (index[0] == 'o') {
    for(p = 0; p != selected_chainsetup_rep->outputs.size(); p++) {
      if (index_number == static_cast<int>(p + 1)) {
	selected_audio_object_rep = selected_chainsetup_rep->outputs[p];
      }
    }
  }
}

ECA_AUDIO_FORMAT ECA_CONTROLLER_OBJECTS::get_audio_format(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_audio_object_rep != 0);
  // --------

  bool was_open = true;
  if (selected_audio_object_rep->is_open() == false) {
    was_open = false;
    selected_audio_object_rep->open();
  }
  ECA_AUDIO_FORMAT t (selected_audio_object_rep->channels(), 
		      selected_audio_object_rep->samples_per_second(), 
		      selected_audio_object_rep->sample_format(),
		      selected_audio_object_rep->interleaved_channels());
  if (was_open == false) selected_audio_object_rep->close();
  return(t);
}

void ECA_CONTROLLER_OBJECTS::add_audio_input(const string& filename) {
  // --------
  REQUIRE(filename.empty() == false);
  REQUIRE(is_selected() == true);
  REQUIRE(connected_chainsetup() != selected_chainsetup());
  // --------

  selected_chainsetup_rep->interpret_audioio_device("-i:" + filename);
  select_audio_object(filename);
  ecadebug->msg("(eca-controller) Added audio input \"" + filename + "\".");
}

void ECA_CONTROLLER_OBJECTS::add_audio_output(const string& filename) {
  // --------
  // require:
  assert(filename.empty() == false);
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  selected_chainsetup_rep->interpret_audioio_device("-o:" + filename);
  select_audio_object(filename);
  ecadebug->msg("(eca-controller) Added audio output \"" + filename +
		"\".");
}

void ECA_CONTROLLER_OBJECTS::add_default_output(void) {
  // --------
  // require:
  assert(selected_chains().size() > 0);
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  add_audio_output(session_rep->ecaresources.resource("default-output"));
  ecadebug->msg("(eca-controller) Added default output to selected chains.");
}

AUDIO_IO* ECA_CONTROLLER_OBJECTS::get_audio_object(void) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------
  return(selected_audio_object_rep);
}

void ECA_CONTROLLER_OBJECTS::remove_audio_object(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  if (selected_audio_object_rep->io_mode() == AUDIO_IO::io_read) 
    selected_chainsetup_rep->remove_audio_input(selected_audio_object_rep->label());
  else 
    selected_chainsetup_rep->remove_audio_output(selected_audio_object_rep->label());

  ecadebug->msg("(eca-controller) Removed selected audio object \"" + selected_audio_object_rep->label() +
		"\" to selected chains.");

  selected_audio_object_rep = 0;

  // --------
  // ensure:
  assert(selected_audio_object_rep == 0);
  // --------
}

void ECA_CONTROLLER_OBJECTS::attach_audio_object(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() > 0);
  assert(selected_audio_object_rep != 0);
  // --------
  if (selected_audio_object_rep->io_mode() == AUDIO_IO::io_read) 
    selected_chainsetup_rep->attach_input_to_selected_chains(selected_audio_object_rep->label());
  else
    selected_chainsetup_rep->attach_output_to_selected_chains(selected_audio_object_rep->label());

  ecadebug->msg("(eca-controller) Attached audio object \"" + selected_audio_object_rep->label() +
		"\" to selected chains.");
}

void ECA_CONTROLLER_OBJECTS::rewind_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  selected_audio_object_rep->seek_position_in_seconds(selected_audio_object_rep->position_in_seconds_exact() - seconds);
}

void ECA_CONTROLLER_OBJECTS::forward_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  selected_audio_object_rep->seek_position_in_seconds(selected_audio_object_rep->position_in_seconds_exact() + seconds);
}

void ECA_CONTROLLER_OBJECTS::set_audio_object_position(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  selected_audio_object_rep->seek_position_in_seconds(seconds);
}

void ECA_CONTROLLER_OBJECTS::wave_edit_audio_object(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  string name = selected_audio_object_rep->label();

  int res = system(string(resource_value("ext-wave-editor") + " " + name).c_str());
  if (res == 127 || res == -1) {
    ecadebug->msg("(eca-controller) Can't edit; unable to open wave editor \"" 
		  + resource_value("x-wave-editor") + "\".");
  }
}

void ECA_CONTROLLER_OBJECTS::add_chain_operator(const string& chainop_params) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  // --------
  selected_chainsetup_rep->interpret_chain_operator(chainop_params);
}

void ECA_CONTROLLER_OBJECTS::add_chain_operator(CHAIN_OPERATOR* cotmp) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  assert(cotmp != 0);
  // --------
  selected_chainsetup_rep->add_chain_operator(cotmp);
}

CHAIN_OPERATOR* ECA_CONTROLLER_OBJECTS::get_chain_operator(int chainop_id) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  assert(chainop_id > 0);
  // --------

  vector<string> schains = selected_chainsetup_rep->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_rep->chains.size();
	p++) {
      if (selected_chainsetup_rep->chains[p]->name() == *o) {
	if (chainop_id - 1 < static_cast<int>(selected_chainsetup_rep->chains[p]->chainops.size()))
	  return(selected_chainsetup_rep->chains[p]->chainops[chainop_id - 1]);
	else
	  return(0);
      }
    }
    ++o;
  }
  return(0);
}

void ECA_CONTROLLER_OBJECTS::remove_chain_operator(int chainop_id) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  assert(chainop_id > 0);
  // --------

  vector<string> schains = selected_chainsetup_rep->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_rep->chains.size();
	p++) {
      if (selected_chainsetup_rep->chains[p]->name() == *o) {
	selected_chainsetup_rep->chains[p]->select_chain_operator(chainop_id);
	selected_chainsetup_rep->chains[p]->remove_chain_operator();
	return;
      }
    }
    ++o;
  }
}


void ECA_CONTROLLER_OBJECTS::set_chain_operator_parameter(int chainop_id,
						  int param,
						  CHAIN_OPERATOR::parameter_type value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_chains().size() == 1);
  assert(chainop_id > 0);
  assert(param > 0);
  // --------

  vector<string> schains = selected_chainsetup_rep->selected_chains();
  vector<string>::const_iterator o = schains.begin();
  while(o != schains.end()) {
    for(vector<CHAIN*>::size_type p = 0; 
	p != selected_chainsetup_rep->chains.size();
	p++) {
      if (selected_chainsetup_rep->chains[p]->name() == *o) {
	if (selected_chainsetup() == connected_chainsetup()) {
	  ecasound_queue.push_back(ECA_PROCESSOR::ep_c_select, p);
	  ecasound_queue.push_back(ECA_PROCESSOR::ep_cop_select, chainop_id);
	  ecasound_queue.push_back(ECA_PROCESSOR::ep_copp_select, param);
	  ecasound_queue.push_back(ECA_PROCESSOR::ep_copp_value, value);
	}
	else {
	  if (chainop_id < static_cast<int>(selected_chainsetup_rep->chains[p]->chainops.size() + 1)) {
	    selected_chainsetup_rep->chains[p]->select_chain_operator(chainop_id);
	    selected_chainsetup_rep->chains[p]->set_parameter(param,value);
	  }
	}
	return;
      }
    }
    ++o;
  }
}

void ECA_CONTROLLER_OBJECTS::add_controller(const string& gcontrol_params) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() > 0);
  // --------
  selected_chainsetup_rep->interpret_controller(gcontrol_params);
}
