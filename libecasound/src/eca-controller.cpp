// ------------------------------------------------------------------------
// eca-controller.cpp: Class for controlling the whole ecasound library
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <kvutils.h>

#include "eca-main.h"
#include "eca-session.h"
#include "eca-controller.h"
#include "eca-chainop.h"
#include "eca-chainsetup.h"

#include "eca-error.h"
#include "eca-debug.h"

string ecasound_lockfile;

ECA_CONTROLLER::ECA_CONTROLLER (ECA_SESSION* psession) {
  session_rep = psession;
  selected_chainsetup_rep = psession->selected_chainsetup;
  selected_chainop_rep = 0;
  selected_audio_object_rep = 0;
  
  engine_started = false;
  ecasound_lockfile = "/var/lock/ecasound.lck." + kvu_numtostr(getpid());
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
      selected_chainsetup_rep->interpret_options(args);
      break;
    }

    // ---
    // General
    // ---
  case ec_exit: { quit(); break; }
  case ec_start: 
    { 
      if (is_connected()) start();
      else 
	ecadebug->msg("(eca-controller) Can't perform requested action; no chainsetup connected.");
      break; 
    }
  case ec_stop: { if (is_engine_started() == true) stop(); break; }
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
  case ec_c_add: { add_chains(args); break; }
  case ec_c_select: { select_chains(args); break; }
  case ec_c_select_all: { select_all_chains(); break; }
  case ec_c_remove: { remove_chains(); break; }
  case ec_c_clear: { clear_chains(); break; }
  case ec_c_name: 
    { 
      if (selected_chains().size() != 1) {
	ecadebug->msg("(eca-controller) When renaming chains, only one chain has to selected.");
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
  case ec_aio_add_output: { add_audio_output(args[0]); break; }
  case ec_aio_select: { select_audio_object(args[0]); break; }
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
      DYNAMIC_PARAMETERS::parameter_type v = atof(a[2].c_str());

      if (id1 > 0 && id2 > 0) {
	set_chain_operator_parameter(id1,id2, v);
      }
      else
	ecadebug->msg("(eca-controller) ERROR! Chain operator indexing starts from 1.");
      break; 
    }
  case ec_cop_add_controller: { add_controller(args[0]); break; }
  case ec_cop_status: 
    { 
      ecadebug->control_flow("Controller/Chain operator status");
      ecadebug->msg(chain_operator_status()); 
      break; 
    }

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
  }
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

void ECA_CONTROLLER::add_chainsetup(const string& name) {
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

void ECA_CONTROLLER::remove_chainsetup(void) {
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

void ECA_CONTROLLER::load_chainsetup(const string& filename) {
  session_rep->load_chainsetup(filename);
  select_chainsetup(get_chainsetup_filename(filename)->name());
  ecadebug->msg("(eca-controller) Loaded chainsetup from file \"" + filename + "\".");
}

void ECA_CONTROLLER::save_chainsetup(const string& filename) {
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

void ECA_CONTROLLER::select_chainsetup(const string& name) {
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

void ECA_CONTROLLER::select_chainsetup_by_index(const string& index) { 
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
    if (index_number == p + 1) {
      select_chainsetup(session_rep->chainsetups[p]->name());
      break;
    }
  }
}

string ECA_CONTROLLER::selected_chainsetup(void) const {
 if (selected_chainsetup_rep != 0)
   return(selected_chainsetup_rep->name());

 return("");
}

void ECA_CONTROLLER::edit_chainsetup(void) {
  // --------
  // require:
  assert(selected_chainsetup().empty() != true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------

  string origname = selected_chainsetup_rep->filename();
  string filename = string(tmpnam(NULL));
  filename += ".ecs";

  save_chainsetup(filename);
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
    select_chainsetup(get_chainsetup_filename(filename)->name());
    if (origname.empty() == false) {
      set_chainsetup_filename(origname);
    }
    remove(filename.c_str());
  }
}

void ECA_CONTROLLER::set_chainsetup_processing_length_in_seconds(double value) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(value > 0.0);
  // --------
  selected_chainsetup_rep->length_in_seconds(value);
  ecadebug->msg("(eca-controller) Set processing length to \"" + kvu_numtostr(value) + "\" seconds.");
}

void ECA_CONTROLLER::set_chainsetup_processing_length_in_samples(long int value) {
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

void ECA_CONTROLLER::toggle_chainsetup_looping(void) {
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

bool ECA_CONTROLLER::is_valid(void) const {
  // --------
  // require:
  assert(is_selected());
  // --------

  return(selected_chainsetup_rep->is_valid());
}

void ECA_CONTROLLER::connect_chainsetup(void) {
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

string ECA_CONTROLLER::connected_chainsetup(void) const {
  if (session_rep->connected_chainsetup != 0) {
    return(session_rep->connected_chainsetup->name());
  }

  return("");
}

void ECA_CONTROLLER::disconnect_chainsetup(void) {
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


const ECA_CHAINSETUP* ECA_CONTROLLER::get_chainsetup(const string&
						     name) const {
  vector<ECA_CHAINSETUP*>::const_iterator p = session_rep->chainsetups.begin();
  while(p != session_rep->chainsetups.end()) {
    if ((*p)->name() == name) {
      return((*p));
    }
    ++p;
    }
  return(0);
}

const ECA_CHAINSETUP* ECA_CONTROLLER::get_chainsetup_filename(const string&
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

const string& ECA_CONTROLLER::chainsetup_filename(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  return(selected_chainsetup_rep->filename());
}

void ECA_CONTROLLER::set_chainsetup_filename(const string& name) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(name.empty() != true);
  // --------
  selected_chainsetup_rep->set_filename(name);
}

void ECA_CONTROLLER::set_chainsetup_parameter(const string& name) {
  // --------
  // require:
  assert(is_selected() == true  && 
	 name.empty() != true);
  // --------

  selected_chainsetup_rep->interpret_general_option(name);
}

void ECA_CONTROLLER::set_chainsetup_sample_format(const string& name) {
  // --------
  // require:
  assert(is_selected() == true  && 
	 name.empty() != true);
  // --------

  selected_chainsetup_rep->interpret_audio_format("-f:" + name);
}

void ECA_CONTROLLER::start(bool ignore_lock) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (session_rep->status() == ep_status_running) return;
  ecadebug->control_flow("Controller/Processing started");

  if (session_rep->status() == ep_status_notready) {
    start_engine(ignore_lock);
  }

  if (is_engine_started() == false) {
    ecadebug->msg("(eca-controller) Can't start processing: couldn't start engine.");
    return;
  }  

  ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);

//    struct timespec sleepcount;
//    sleepcount.tv_sec = 1;
//    sleepcount.tv_nsec = 0;
//    int count = 0;
//    while(session_rep->status() != ep_status_running) {
//      ++count;
//      nanosleep(&sleepcount, NULL);
//      if (session_rep->status() == ep_status_finished) break;
//      if (count > 5000) {
//        throw(new ECA_ERROR("ECA_CONTROLLER", "start() failed, engine is not responding...", ECA_ERROR::stop));
//      }
//    }

  // --------
  // ensure:
  assert(is_engine_started() == true);
  // --------
}

void ECA_CONTROLLER::stop(void) {
  // --------
  // require:
  assert(is_engine_started() == true);
  // --------

  if (session_rep->status() != ep_status_running) return;
  ecadebug->control_flow("Controller/Processing stopped");
  ecasound_queue.push_back(ECA_PROCESSOR::ep_stop, 0.0);

//    int count = 0;
//    struct timespec sleepcount;
//    sleepcount.tv_sec = 1;
//    sleepcount.tv_nsec = 0;
//    while(session_rep->status() == ep_status_running) {
//      //    usleep(++usleep_count); 
//      ++count;
//      nanosleep(&sleepcount, NULL);
//      if (count > 5000) {
//        throw(new ECA_ERROR("ECA_CONTROLLER", "stop() failed, engine is not responding...", ECA_ERROR::stop));
//      }
//    }

  // --------
  // ensure:
  // assert(is_running() == false); 
  // -- there's a small timeout so assertion cannot be checked
  // --------
}

void ECA_CONTROLLER::quit(void) {
  close_engine();
  int n = ECA_QUIT;
  throw(n);
}

void ECA_CONTROLLER::start_engine(bool ignore_lock) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (engine_started == true) return;

  ifstream fin(ecasound_lockfile.c_str());
  if (!fin || ignore_lock) {
    struct sched_param sparam;
    sparam.sched_priority = 10;

    if (session_rep->connected_chainsetup->raised_priority() == true) {
      if (sched_setscheduler(0, SCHED_FIFO, &sparam) == -1) 
  	ecadebug->msg("(eca-controller) Unable to change scheduling policy!");
      else 
  	ecadebug->msg("(eca-controller) Using realtime-scheduling (SCHED_FIFO/10).");
      pthread_attr_t th_attr;
      pthread_attr_init(&th_attr);
      //      pthread_attr_setschedpolicy(&th_attr, SCHED_FIFO);
      //      pthread_attr_setschedparam(&th_attr, &sparam);
      //      ecadebug->msg("(eca-controller) Using realtime-scheduling (SCHED_FIFO/10, engine-thread).");
      start_normal_thread(session_rep, retcode, &th_cqueue, &th_attr);
    }
    else 
      start_normal_thread(session_rep, retcode, &th_cqueue, NULL);
  }
  else {
    MESSAGE_ITEM mitem;
    mitem << "(eca-controller) Can't execute; processing module already running!" << 'c' << "\n";
    ecadebug->msg(1,mitem.to_string());
  }
  fin.close();

  engine_started = true;

  // --------
  // ensure:
  assert(is_engine_started() == true);
  // --------
}

void ECA_CONTROLLER::close_engine(void) {
  if (!engine_started) return;
  ecasound_queue.push_back(ECA_PROCESSOR::ep_exit, 0.0);
//    ifstream fin(ecasound_lockfile.c_str());
//    while(fin) {
//      fin.close();
//      ecadebug->msg(1, "(eca-controller) Waiting for the processing thread...");
//      struct timespec sleepcount;
//      sleepcount.tv_sec = 1;
//      sleepcount.tv_nsec = 0;
//      nanosleep(&sleepcount, NULL);
//      fin.open(ecasound_lockfile.c_str());
//    }
//    while(ecasound_queue.is_empty() == false) ecasound_queue.pop_front();
  engine_started = false;

  // --------
  // ensure:
  assert(is_engine_started() == false);
  // --------
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
  else st_info_string << "Multitrack-mode: disabled\n";

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
    if ((*cs_citer)->raised_priority()) result += "P";
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
      mitem << "\t" << p + 1 << ". " << (*chain_citer)->chainops[p]->name() << ": ";
      for(int n = 0; n < (*chain_citer)->chainops[p]->number_of_params(); n++) {
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
    st_info_string += (*adev_citer)->label() + "\"";
    if ((*adev_citer) == selected_audio_object_rep) st_info_string += " [selected]";
    st_info_string += "\n\tconnected to chains \"";
    vector<string> temp = selected_chainsetup_rep->get_connected_chains_to_input((selected_chainsetup_rep->inputs)[adev_sizet]);
    vector<string>::const_iterator p = temp.begin();
    while (p != temp.end()) {
      st_info_string += *p; 
      ++p;
      if (p != temp.end())  st_info_string += ",";
    }
    st_info_string += "\": ";
    st_info_string += (*adev_citer)->status();
    st_info_string += "\n";
    ++adev_sizet;
    ++adev_citer;
  }

  adev_sizet = 0;
  adev_citer = selected_chainsetup_rep->outputs.begin();
  while(adev_citer != selected_chainsetup_rep->outputs.end()) {
    st_info_string += "Output (o" + kvu_numtostr(adev_sizet + 1) + "): \"";
    st_info_string += (*adev_citer)->label() + "\"";
    if ((*adev_citer) == selected_audio_object_rep) st_info_string += " [selected]";
    st_info_string += "\n\tconnected to chains \"";
    vector<string> temp = selected_chainsetup_rep->get_connected_chains_to_output((selected_chainsetup_rep->outputs)[adev_sizet]);
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

bool ECA_CONTROLLER::is_connected(void) const {
  if (session_rep->connected_chainsetup == 0) return(false);
  return(session_rep->connected_chainsetup->is_valid());
}

bool ECA_CONTROLLER::is_selected(void) const {
  if (selected_chainsetup_rep == 0) return(false);
  return(true);
}

bool ECA_CONTROLLER::is_running(void) const {
  if (session_rep->status() == ep_status_running) return(true);
  else return(false);
}

string ECA_CONTROLLER::engine_status(void) const {
  switch(session_rep->status()) {
  case ep_status_running: 
    {
    return("running"); 
    }
  case ep_status_stopped: 
    {
    return("stopped"); 
    }
  case ep_status_finished:
    {
    return("finished"); 
    }
  case ep_status_notready: 
    {
    return("not ready"); 
    }
  default: 
    {
    return("unknown status"); 
    }
  }
}

string ECA_CONTROLLER::connected_chains_input(AUDIO_IO* aiod) const {
  if (session_rep->connected_chainsetup == 0) return("");

  vector<string> t = session_rep->get_connected_chains_to_input(aiod);
  string out = "";
  vector<string>::const_iterator p = t.begin();
  while(p != t.end()) {
    out += *p;
    ++p;
    if (p != t.end()) out += ",";
  }
  return(out);
}

string ECA_CONTROLLER::connected_chains_output(AUDIO_IO* aiod) const {
  if (session_rep->connected_chainsetup == 0) return("");

  vector<string> t = session_rep->get_connected_chains_to_output(aiod);
  string out = "";
  vector<string>::const_iterator p = t.begin();
  while(p != t.end()) {
    out += *p;
    ++p;
    if (p != t.end()) out += ",";
  }
  return(out);
}

vector<string> ECA_CONTROLLER::connected_chains(const string&
						filename) const {
  if (session_rep->connected_chainsetup == 0) return(*(new vector<string> (0)));
  return(session_rep->connected_chainsetup->get_connected_chains_to_iodev(filename));
}

void ECA_CONTROLLER::add_chain(const string& name) { 
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

void ECA_CONTROLLER::add_chains(const string& names) { 
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

void ECA_CONTROLLER::add_chains(const vector<string>& new_chains) { 
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

void ECA_CONTROLLER::remove_chains(void) { 
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

void ECA_CONTROLLER::select_chains(const vector<string>& chains) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  selected_chainsetup_rep->select_chains(chains);

  ecadebug->msg("(eca-controller) Selected chains: " +
		vector_to_string(chains, ", ") + ".");
}

void ECA_CONTROLLER::select_all_chains(void) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  selected_chainsetup_rep->select_all_chains();

  ecadebug->msg("(eca-controller) Selected chains: " +
		vector_to_string(selected_chains(), ", ") + ".");
}

const vector<string>& ECA_CONTROLLER::selected_chains(void) const {
 // --------
  // require:
  assert(is_selected() == true);
  // --------

  return(selected_chainsetup_rep->selected_chains());
}

void ECA_CONTROLLER::clear_chains(void) { 
 // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() > 0);
  // --------
  selected_chainsetup_rep->clear_chains();
}

void ECA_CONTROLLER::rename_chain(const string& name) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  // --------
  selected_chainsetup_rep->rename_chain(name);     
}

void ECA_CONTROLLER::send_chain_commands_to_engine(int command, double value) {
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

void ECA_CONTROLLER::toggle_chain_muting(void) { 
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

void ECA_CONTROLLER::toggle_chain_bypass(void) { 
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

void ECA_CONTROLLER::rewind_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_rewind, pos_in_seconds);
}

void ECA_CONTROLLER::forward_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_forward, pos_in_seconds);
}

void ECA_CONTROLLER::set_position_chains(double pos_in_seconds) { 
  // --------
  // require:
  assert(is_selected() == true && is_connected() == true);
  assert(selected_chains().size() > 0);
  // --------
  send_chain_commands_to_engine(ECA_PROCESSOR::ep_c_setpos, pos_in_seconds);
}

void ECA_CONTROLLER::set_default_audio_format(const string& sfrm,
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

void ECA_CONTROLLER::set_default_audio_format(const ECA_AUDIO_FORMAT* format) {
 // --------
  // require:
  assert(is_selected() == true);
  // --------

  set_default_audio_format(format->format_string(), 
			   static_cast<int>(format->channels()), 
			   static_cast<long int>(format->samples_per_second()));
}

void ECA_CONTROLLER::select_audio_object(const string& name) { 
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

  for(p = 0; p != selected_chainsetup_rep->outputs.size(); p++) {
    if (selected_chainsetup_rep->outputs[p]->label() == name) {
      selected_audio_object_rep = selected_chainsetup_rep->outputs[p];
    }
  }
}

void ECA_CONTROLLER::select_audio_object_by_index(const string& index) { 
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
      if (index_number == p + 1) {
	selected_audio_object_rep = selected_chainsetup_rep->inputs[p];
      }
    }
  }  
  else if (index[0] == 'o') {
    for(p = 0; p != selected_chainsetup_rep->outputs.size(); p++) {
      if (index_number == p + 1) {
	selected_audio_object_rep = selected_chainsetup_rep->outputs[p];
      }
    }
  }
}

ECA_AUDIO_FORMAT ECA_CONTROLLER::get_audio_format(void) const {
  // --------
  // require:
  assert(is_selected() == true);
  assert(selected_audio_object_rep != 0);
  // --------

  ECA_AUDIO_FORMAT t (selected_audio_object_rep->channels(), 
		      selected_audio_object_rep->samples_per_second(), 
		      selected_audio_object_rep->sample_format(),
		      selected_audio_object_rep->interleaved_channels());

  return(t);
}

void ECA_CONTROLLER::add_audio_input(const string& filename) {
  // --------
  // require:
  assert(filename.empty() == false);
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  selected_chainsetup_rep->interpret_audioio_device("-i", filename);
  select_audio_object(filename);
  ecadebug->msg("(eca-controller) Added audio input \"" + filename + "\".");

  // --------
  // ensure:
  assert(get_audio_object(filename) == selected_audio_object_rep);
  // --------
}

void ECA_CONTROLLER::add_audio_output(const string& filename) {
  // --------
  // require:
  assert(filename.empty() == false);
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  selected_chainsetup_rep->interpret_audioio_device("-o", filename);
  select_audio_object(filename);
  ecadebug->msg("(eca-controller) Added audio output \"" + filename +
		"\".");

  // --------
  // ensure:
  assert(get_audio_object(filename) == selected_audio_object_rep);
  // --------
}

void ECA_CONTROLLER::add_default_output(void) {
  // --------
  // require:
  assert(selected_chains().size() > 0);
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  // --------
  add_audio_output(session_rep->ecaresources.resource("default-output"));
  ecadebug->msg("(eca-controller) Added default output to selected chains.");
}

AUDIO_IO* ECA_CONTROLLER::get_audio_object(const string& name) {
  // --------
  // require:
  assert(is_selected() == true);
  // --------

  vector<AUDIO_IO*>::size_type p = 0;
  for(p = 0; p != selected_chainsetup_rep->inputs.size(); p++) {
    if (selected_chainsetup_rep->inputs[p]->label() == name) {
      return(selected_chainsetup_rep->inputs[p]);
    }
  }

  for(p = 0; p != selected_chainsetup_rep->outputs.size(); p++) {
    if (selected_chainsetup_rep->outputs[p]->label() == name) {
      return(selected_chainsetup_rep->outputs[p]);
    }
  }

  return(0);
}

void ECA_CONTROLLER::remove_audio_object(void) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  if (selected_audio_object_rep->io_mode() == si_read) 
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

void ECA_CONTROLLER::attach_audio_object(void) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() > 0);
  assert(selected_audio_object_rep != 0);
  // --------
  if (selected_audio_object_rep->io_mode() == si_read) 
    selected_chainsetup_rep->attach_input_to_selected_chains(selected_audio_object_rep->label());
  else
    selected_chainsetup_rep->attach_output_to_selected_chains(selected_audio_object_rep->label());

  ecadebug->msg("(eca-controller) Attached audio object \"" + selected_audio_object_rep->label() +
		"\" to selected chains.");
}

void ECA_CONTROLLER::rewind_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  selected_audio_object_rep->seek_position_in_seconds(selected_audio_object_rep->position_in_seconds_exact() - seconds);
}

void ECA_CONTROLLER::forward_audio_object(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  selected_audio_object_rep->seek_position_in_seconds(selected_audio_object_rep->position_in_seconds_exact() + seconds);
}

void ECA_CONTROLLER::set_audio_object_position(double seconds) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_audio_object_rep != 0);
  // --------
  selected_audio_object_rep->seek_position_in_seconds(seconds);
}

void ECA_CONTROLLER::wave_edit_audio_object(void) {
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

void ECA_CONTROLLER::add_chain_operator(const string& chainop_params) {
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  // --------
  selected_chainsetup_rep->interpret_chain_operator(chainop_params);
}

void ECA_CONTROLLER::add_chain_operator(CHAIN_OPERATOR* cotmp) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() == 1);
  assert(cotmp != 0);
  // --------
  selected_chainsetup_rep->add_chain_operator(cotmp);
}

const CHAIN_OPERATOR* ECA_CONTROLLER::get_chain_operator(int chainop_id) {
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
	if (chainop_id - 1 < selected_chainsetup_rep->chains[p]->chainops.size())
	  return(selected_chainsetup_rep->chains[p]->chainops[chainop_id - 1]);
	else
	  return(0);
      }
    }
    ++o;
  }
}

void ECA_CONTROLLER::remove_chain_operator(int chainop_id) { 
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


void ECA_CONTROLLER::set_chain_operator_parameter(int chainop_id,
						  int param,
						  DYNAMIC_PARAMETERS::parameter_type value) {
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
	  if (chainop_id < selected_chainsetup_rep->chains[p]->chainops.size() + 1) {
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

void ECA_CONTROLLER::add_controller(const string& gcontrol_params) { 
  // --------
  // require:
  assert(is_selected() == true);
  assert(connected_chainsetup() != selected_chainsetup());
  assert(selected_chains().size() > 0);
  // --------
  selected_chainsetup_rep->interpret_controller(gcontrol_params);
}

void ECA_CONTROLLER::set_buffersize(int bsize) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  selected_chainsetup_rep->set_buffersize(bsize); 
}

void ECA_CONTROLLER::toggle_raise_priority(bool v) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  selected_chainsetup_rep->toggle_raised_priority(v);
}

void start_normal_thread(ECA_SESSION* param, int retcode, pthread_t*
			 th_ecasound_cqueue, pthread_attr_t* th_attr) {
  retcode = pthread_create(th_ecasound_cqueue, th_attr, start_normal, (void*)param);
  if (retcode != 0)
    throw(new ECA_ERROR("ECA-CONTROLLER", "Unable to create a new thread (start_normal)."));
}

void* start_normal(void* param) {
  ofstream fout(ecasound_lockfile.c_str());
  fout.close();
  ecadebug->msg(1,"(eca-controller) Engine-thread pid: " + kvu_numtostr(getpid()));
  start_normal((ECA_SESSION*)param);
  remove(ecasound_lockfile.c_str());
  return(0);
}

void start_normal(ECA_SESSION* param) {
  try {
    ECA_PROCESSOR epros (param);
    epros.exec();
  }
  catch(ECA_ERROR* e) {
    cerr << "---\n(eca-controller) ERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\n(eca-controller) Caught an unknown exception!\n";
  }
}
