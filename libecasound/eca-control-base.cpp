// ------------------------------------------------------------------------
// eca-control-base.cpp: Base class providing basic functionality
//                       for controlling the ecasound library
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <signal.h>

#include <kvutils/value_queue.h>
#include <kvutils/message_item.h>
#include <kvutils/dbc.h>

#include "eca-engine.h"
#include "eca-session.h"
#include "eca-chainsetup.h"
#include "eca-control-base.h"
#include "eca-resources.h"

#include "eca-error.h"
#include "eca-debug.h"

void* start_normal_thread(void *ptr);

/**
 * Helper function for starting the slave thread.
 */
void* start_normal_thread(void *ptr) {
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_BLOCK, &sigset, 0);

  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-controller) Engine-thread pid: " + kvu_numtostr(getpid()));
  ECA_CONTROL_BASE* ctrl_base = static_cast<ECA_CONTROL_BASE*>(ptr);
  ctrl_base->run_engine();
  return(0);
}

ECA_CONTROL_BASE::ECA_CONTROL_BASE (ECA_SESSION* psession) {
  session_repp = psession;
  selected_chainsetup_repp = psession->selected_chainsetup_repp;
  engine_repp = 0;
}

ECA_CONTROL_BASE::~ECA_CONTROL_BASE (void) {
  close_engine();
}

/**
 * Start the processing engine
 *
 * @pre is_connected() == true
 * @post is_engine_started() == true
 */
void ECA_CONTROL_BASE::start(void) {
  // --------
  DBC_REQUIRE(is_connected() == true);
  // --------

  if (is_engine_started() == true &&
      engine_repp->status() == ECA_ENGINE::engine_status_running) return;

  ecadebug->control_flow("Controller/Processing started");

  if (is_engine_started() != true) {
    start_engine();
  }

  if (is_engine_started() != true) {
    ecadebug->msg("(eca-controller) Can't start processing: couldn't start engine.");
    return;
  }  

  engine_repp->command(ECA_ENGINE::ep_start, 0.0);

  // --------
  DBC_ENSURE(is_engine_started() == true);
  // --------
}

/**
 * Start the processing engine and block until 
 * processing is finished.
 *
 * @pre is_connected() == true
 * @post is_finished() == true || 
 * @post (processing_started == true && is_running() != true) ||
 * @post (processing_started != true && engine_repp->status() != ECA_ENGINE::engine_status_stopped)
 */
void ECA_CONTROL_BASE::run(void) {
  // --------
  DBC_REQUIRE(is_connected() == true);
  // --------

  if (is_engine_started() == true &&
      engine_repp->status() == ECA_ENGINE::engine_status_running) return;

  start();

  struct timespec sleepcount;
  sleepcount.tv_sec = 1;
  sleepcount.tv_nsec = 0;

  bool processing_started = false;
  while(is_finished() == false) {
    ::nanosleep(&sleepcount, NULL);
    if (processing_started != true) {
      if (is_running() == true) processing_started = true;
      else if (engine_repp->status() != ECA_ENGINE::engine_status_stopped) break;
    }
    else {
      if (is_running() != true) break;
    }
  }      

  ecadebug->control_flow("Controller/Processing finished");

  // --------
  DBC_ENSURE(is_finished() == true || 
	     (processing_started == true && is_running() != true) ||
	     (processing_started != true && engine_repp->status() != ECA_ENGINE::engine_status_stopped));
  // --------
}

/**
 * Stops the processing engine.
 *
 * @pre is_engine_started() == true
 * @post is_running() == false
 */
void ECA_CONTROL_BASE::stop(void) {
  // --------
  DBC_REQUIRE(is_engine_started() == true);
  // --------

  if (engine_repp->status() != ECA_ENGINE::engine_status_running) return;
  ecadebug->control_flow("Controller/Processing stopped");
  engine_repp->command(ECA_ENGINE::ep_stop, 0.0);

  // --------
  // ensure:
  // assert(is_running() == false); 
  // -- there's a small timeout so assertion cannot be checked
  // --------
}

/**
 * Stop the processing engine using thread-to-thread condition
 * signaling.
 *
 * @pre is_engine_started() == true
 * @post is_running() == false
 */
void ECA_CONTROL_BASE::stop_on_condition(void) {
  // --------
  DBC_REQUIRE(is_engine_started() == true);
  // --------

  if (engine_repp->status() != ECA_ENGINE::engine_status_running) return;
  ecadebug->control_flow("Controller/Processing stopped (cond)");
  engine_repp->command(ECA_ENGINE::ep_stop, 0.0);
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-controller-base) Received stop-cond");

  // --
  // blocks until engine has stopped (or 60sec has passed);
  engine_repp->wait_for_stop(60);

  // --------
  DBC_ENSURE(is_running() == false); 
  // --------
}

/**
 * Stops the processing engine.
 */
void ECA_CONTROL_BASE::quit(void) { close_engine(); }

/**
 * Starts the processing engine.
 *
 * @pre is_connected() == true
 * @pre is_engine_started() != true
 */
void ECA_CONTROL_BASE::start_engine(void) {
  // --------
  DBC_REQUIRE(is_connected() == true);
  DBC_REQUIRE(is_engine_started() != true);
  // --------

  unsigned int p = session_repp->connected_chainsetup_repp->first_selected_chain();
  if (p < session_repp->connected_chainsetup_repp->chains.size())
    session_repp->connected_chainsetup_repp->active_chain_index_rep = p;

  engine_repp = new ECA_ENGINE (session_repp);

  pthread_attr_t th_attr;
  pthread_attr_init(&th_attr);
  int retcode_rep = pthread_create(&th_cqueue_rep,
				   &th_attr,
				   start_normal_thread, 
				   static_cast<void *>(this));
  if (retcode_rep != 0) {
    ecadebug->msg(ECA_DEBUG::info, "Warning! Unable to create a new thread for engine.");
    engine_repp = 0;
  }
}

/**
 * Routine used for launching the engine.
 */
void ECA_CONTROL_BASE::run_engine(void) {
  // --
  // launch the engine - blocks until processing 
  // has stopped (or exit command is issued)
  engine_repp->exec();

  delete engine_repp;
  engine_repp = 0;
}

/**
 * Closes the processing engine.
 *
 * ensure:
 *  is_engine_started() != true
 */
void ECA_CONTROL_BASE::close_engine(void) {
  if (is_engine_started() != true) return;

  engine_repp->command(ECA_ENGINE::ep_exit, 0.0);

  // --
  // wait until run_engine() is finished
  ::pthread_join(th_cqueue_rep,NULL);

  // ---
  DBC_ENSURE(is_engine_started() != true);
  // ---
}

/**
 * Is currently selected chainsetup valid?
 *
 * @pre is_selected()
 */
bool ECA_CONTROL_BASE::is_valid(void) const {
  // --------
  // require:
  assert(is_selected());
  // --------

  return(selected_chainsetup_repp->is_valid());
}

/**
 * Returns true if active chainsetup exists and is connected.
 */
bool ECA_CONTROL_BASE::is_connected(void) const {
  if (session_repp->connected_chainsetup_repp == 0) return(false);
  return(session_repp->connected_chainsetup_repp->is_valid() &&
	 session_repp->connected_chainsetup_repp->is_enabled());
}

/**
 * Returns true if some chainsetup is selected.
 */
bool ECA_CONTROL_BASE::is_selected(void) const { return(selected_chainsetup_repp != 0); } 

/**
 * Returns true if processing engine is running.
 */
bool ECA_CONTROL_BASE::is_running(void) const { return(is_engine_started() == true && engine_repp->status() == ECA_ENGINE::engine_status_running); } 

/**
 * Returns true if engine has finished processing. Engine state is 
 * either "finished" or "error".
 */
bool ECA_CONTROL_BASE::is_finished(void) const { 
  return(is_engine_started() == true && 
	 (engine_repp->status() == ECA_ENGINE::engine_status_finished ||
	  engine_repp->status() == ECA_ENGINE::engine_status_error)); 
} 

std::string ECA_CONTROL_BASE::resource_value(const std::string& key) const { 
  ECA_RESOURCES ecarc;
  return(ecarc.resource(key)); 
}

void ECA_CONTROL_BASE::toggle_multitrack_mode(bool v) { 
  if (selected_chainsetup_repp != 0)
    selected_chainsetup_repp->multitrack_mode_rep = v; 
}

/**
 * Returns the length of the selected chainsetup (in samples).
 *
 * @pre is_selected() == true
 */
long ECA_CONTROL_BASE::length_in_samples(void) const { 
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  return(selected_chainsetup_repp->length_in_samples());
}

/**
 * Returns the length of the selected chainsetup (in seconds).
 *
 * @pre is_selected() == true
 */
double ECA_CONTROL_BASE::length_in_seconds_exact(void) const { 
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  return(selected_chainsetup_repp->length_in_seconds_exact());
}

/**
 * Returns the current position of the selected chainsetup (in samples).
 *
 * @pre is_selected() == true
 */
long ECA_CONTROL_BASE::position_in_samples(void) const { 
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  return(selected_chainsetup_repp->position_in_samples());
}

/**
 * Returns the current position of the selected chainsetup (in seconds).
 *
 * @pre is_selected() == true
 */
double ECA_CONTROL_BASE::position_in_seconds_exact(void) const {
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  return(selected_chainsetup_repp->position_in_seconds_exact());
}

/**
 * Return info about engine status.
 */
string ECA_CONTROL_BASE::engine_status(void) const {
  if (is_engine_started() == true) {
    switch(engine_repp->status()) {
    case ECA_ENGINE::engine_status_running: 
      {
	return("running"); 
      }
    case ECA_ENGINE::engine_status_stopped: 
      {
	return("stopped"); 
      }
    case ECA_ENGINE::engine_status_finished:
      {
	return("finished"); 
      }
    case ECA_ENGINE::engine_status_error:
      {
	return("error"); 
      }
    case ECA_ENGINE::engine_status_notready: 
      {
	return("not ready"); 
      }
    default: 
      {
	return("unknown status"); 
      }
    }
  }
  return("not started");
}

/**
 * Get a string containing a comma separated list of all chains 
 * attached to input with index 'aiod'. 
 *
 * @pre is_selected() == true
 */
std::string ECA_CONTROL_BASE::attached_chains_input(AUDIO_IO* aiod) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  vector<string> t = selected_chainsetup_repp->get_attached_chains_to_input(aiod);
  string out = "";
  vector<string>::const_iterator p = t.begin();
  while(p != t.end()) {
    out += *p;
    ++p;
    if (p != t.end()) out += ",";
  }
  return(out);
}

/**
 * Get a string containing a comma separated list of all chains 
 * attached to output with index 'aiod'.
 *
 * @pre is_selected() == true
 */
std::string ECA_CONTROL_BASE::attached_chains_output(AUDIO_IO* aiod) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  vector<string> t = selected_chainsetup_repp->get_attached_chains_to_output(aiod);
  string out = "";
  vector<string>::const_iterator p = t.begin();
  while(p != t.end()) {
    out += *p;
    ++p;
    if (p != t.end()) out += ",";
  }
  return(out);
}

/**
 * Get a string containing a comma separated list of all chains 
 * attached to audio object with name 'filename'. 
 *
 * @pre is_selected() == true
 */
std::vector<std::string> ECA_CONTROL_BASE::attached_chains(const string& filename) const {
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  return(selected_chainsetup_repp->get_attached_chains_to_iodev(filename));
}

/**
 * Set the default buffersize (in samples).
 *
 * @pre is_editable() == true
 */
void ECA_CONTROL_BASE::set_buffersize(int bsize) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  selected_chainsetup_repp->set_buffersize(bsize); 
}

/**
 * Toggle whether raised priority mode is enabled or not.
 *
 * @pre is_editable() == true
 */
void ECA_CONTROL_BASE::toggle_raise_priority(bool v) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  selected_chainsetup_repp->toggle_raised_priority(v);
}

void ECA_CONTROL_BASE::set_last_string_list(const std::vector<std::string>& s) { 
  last_los_rep = s; 
  last_type_rep = "S";
}

void ECA_CONTROL_BASE::set_last_string(const std::string& s) { 
  last_s_rep = s; 
  last_type_rep = "s";
}

void ECA_CONTROL_BASE::set_last_float(double v) { 
  last_f_rep = v; 
  last_type_rep = "f";
}

void ECA_CONTROL_BASE::set_last_integer(int v) { 
  last_i_rep = v; 
  last_type_rep = "i";
}
 
void ECA_CONTROL_BASE::set_last_long_integer(int v) { 
  last_li_rep = v; 
  last_type_rep = "li";
}

void ECA_CONTROL_BASE::set_last_error(const string& s) {
  last_error_rep = s;
}

const std::vector<std::string>& ECA_CONTROL_BASE::last_string_list(void) const { return(last_los_rep); }
const std::string& ECA_CONTROL_BASE::last_string(void) const { return(last_s_rep); }
double ECA_CONTROL_BASE::last_float(void) const { return(last_f_rep); }
int ECA_CONTROL_BASE::last_integer(void) const { return(last_i_rep); } 
long int ECA_CONTROL_BASE::last_long_integer(void) const { return(last_li_rep); }
const string& ECA_CONTROL_BASE::last_error(void) const { return(last_error_rep); }
const string& ECA_CONTROL_BASE::last_type(void) const { return(last_type_rep); }

void ECA_CONTROL_BASE::clear_last_values(void) { 
  last_los_rep.clear();
  last_s_rep = "";
  last_li_rep = 0;
  last_i_rep = 0;
  last_f_rep = 0.0f;
  last_error_rep = "";
  last_type_rep = "";
}
