// ------------------------------------------------------------------------
// eca-control-base.cpp: Base class providing basic functionality
//                       for controlling the ecasound library
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
#include <sys/time.h>

#include <kvutils/value_queue.h>
#include <kvutils/message_item.h>

#include "eca-main.h"
#include "eca-session.h"
#include "eca-chainsetup.h"
#include "eca-control-base.h"

#include "eca-error.h"
#include "eca-debug.h"

void* start_normal_thread(void *ptr);

/**
 * Helper function for starting the slave thread.
 */
void* start_normal_thread(void *ptr) {
  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-controller) Engine-thread pid: " + kvu_numtostr(getpid()));
  ECA_CONTROL_BASE* ctrl_base = static_cast<ECA_CONTROL_BASE*>(ptr);
  ctrl_base->run_engine();
}

ECA_CONTROL_BASE::ECA_CONTROL_BASE (ECA_SESSION* psession) {
  session_repp = psession;
  selected_chainsetup_repp = psession->selected_chainsetup_repp;
  engine_started_rep = false;
}

/**
 * Start the processing engine
 *
 * require:
 *  is_connected() == true
 *
 * ensure:
 *  is_engine_started() == true
 */
void ECA_CONTROL_BASE::start(void) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (session_repp->status() == ECA_SESSION::ep_status_running) return;
  ecadebug->control_flow("Controller/Processing started");

//    while(::ecasound_queue.is_empty() == false) ::ecasound_queue.pop_front();
  if (session_repp->status() == ECA_SESSION::ep_status_notready) {
    start_engine();
  }

  if (is_engine_started() == false) {
    ecadebug->msg("(eca-controller) Can't start processing: couldn't start engine.");
    return;
  }  

  session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_start, 0.0);

  // --------
  // ensure:
  assert(is_engine_started() == true);
  // --------
}

/**
 * Start the processing engine and block until 
 * processing is finished.
 *
 * require:
 *  is_connected() == true
 *
 * ensure:
 *  is_finished() == true || (processing_started == true && is_running() != true)
 */
void ECA_CONTROL_BASE::run(void) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (session_repp->status() == ECA_SESSION::ep_status_running) return;

  start();

  struct timespec sleepcount;
  sleepcount.tv_sec = 1;
  sleepcount.tv_nsec = 0;

  bool processing_started = false;
  while(is_finished() == false) {
    ::nanosleep(&sleepcount, NULL);
    if (processing_started != true) {
      if (is_running() == true) processing_started = true;
    }
    else {
      if (is_running() != true) break;
    }
  }      

  ecadebug->control_flow("Controller/Processing finished");

  // --------
  // ensure:
  assert(is_finished() == true || (processing_started == true && is_running() != true));
  // --------
}

/**
 * Stops the processing engine.
 *
 * require:
 *  is_engine_started() == true
 *
 * ensure:
 *   is_running() == false
 */
void ECA_CONTROL_BASE::stop(void) {
  // --------
  // require:
  assert(is_engine_started() == true);
  // --------

  if (session_repp->status() != ECA_SESSION::ep_status_running) return;
  ecadebug->control_flow("Controller/Processing stopped");
  session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_stop, 0.0);

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
 * require:
 *  is_engine_started() == true
 *
 * ensure:
 *   is_running() == false
 */
void ECA_CONTROL_BASE::stop_on_condition(void) {
  // --------
  // require:
  assert(is_engine_started() == true);
  // --------

  if (session_repp->status() != ECA_SESSION::ep_status_running) return;
  ecadebug->control_flow("Controller/Processing stopped");
  session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_stop, 0.0);
  struct timeval now;
  gettimeofday(&now, 0);
  struct timespec sleepcount;
  sleepcount.tv_sec = now.tv_sec + 60;
  sleepcount.tv_nsec = now.tv_usec * 1000;
  ::pthread_mutex_lock(session_repp->ecasound_stop_mutex_repp);
  int res =
    ::pthread_cond_timedwait(session_repp->ecasound_stop_cond_repp, 
			     session_repp->ecasound_stop_mutex_repp, 
			     &sleepcount);
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-controller-base) Received stop-cond");
  ::pthread_mutex_unlock(session_repp->ecasound_stop_mutex_repp);

  // --------
  // ensure:
  assert(is_running() == false); 
  // --------
}

/**
 * Stops the processing engine.
 */
void ECA_CONTROL_BASE::quit(void) { close_engine(); }

/**
 * Starts the processing engine.
 *
 * require:
 *  is_connected() == true
 */
void ECA_CONTROL_BASE::start_engine(void) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (engine_started_rep == true) {
    ecadebug->msg(ECA_DEBUG::info, "(eca-controller) Can't execute; processing engine already running!");
    return;
  }

  pthread_attr_t th_attr;
  pthread_attr_init(&th_attr);
  int retcode_rep = pthread_create(&th_cqueue_rep,
				   &th_attr,
				   start_normal_thread, 
				   static_cast<void *>(this));
  if (retcode_rep != 0) {
    ecadebug->msg(ECA_DEBUG::info, "Warning! Unable to create a new thread for engine.");
    engine_started_rep = false;
  }
  else
    engine_started_rep = true;
}

/**
 * Routine used for launching the engine.
 */
void ECA_CONTROL_BASE::run_engine(void) {
  try {
    ECA_PROCESSOR epros (session_repp);
    epros.exec();
  }
  catch(ECA_ERROR& e) {
    cerr << "---\n(eca-controller) ERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\n(eca-controller) Caught an unknown exception!\n";
  }
}

/**
 * Closes the processing engine.
 *
 * ensure:
 *  is_engine_started() == false
 */
void ECA_CONTROL_BASE::close_engine(void) {
  if (!engine_started_rep) return;
  session_repp->ecasound_queue_rep.push_back(ECA_PROCESSOR::ep_exit, 0.0);
  ::pthread_join(th_cqueue_rep,NULL);
  engine_started_rep = false;

  // --------
  // ensure:
  assert(is_engine_started() == false);
  // --------
}

/**
 * Is currently selected chainsetup valid?
 *
 * require:
 *  is_selected()
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
  return(session_repp->connected_chainsetup_repp->is_valid());
}

/**
 * Returns true if some chainsetup is selected.
 */
bool ECA_CONTROL_BASE::is_selected(void) const { return(selected_chainsetup_repp != 0); } 

/**
 * Returns true if processing engine is running.
 */
bool ECA_CONTROL_BASE::is_running(void) const { return(session_repp->status() == ECA_SESSION::ep_status_running); } 

/**
 * Returns true if engine has finished processing.
 */
bool ECA_CONTROL_BASE::is_finished(void) const { return(session_repp->status() == ECA_SESSION::ep_status_finished); } 

long ECA_CONTROL_BASE::length_in_samples(void) const { 
  if (is_connected() == true)
      return session_repp->connected_chainsetup_repp->length_in_samples(); 
  else
    return(0);
}

double ECA_CONTROL_BASE::length_in_seconds_exact(void) const { 
  if (is_connected() == true) {
      return session_repp->connected_chainsetup_repp->length_in_seconds_exact(); 
  }
  else
    return(0.0);
}

long ECA_CONTROL_BASE::position_in_samples(void) const { 
  if (is_connected() == true)
      return session_repp->connected_chainsetup_repp->position_in_samples(); 
  else
    return(0);
}
double ECA_CONTROL_BASE::position_in_seconds_exact(void) const {
  if (is_connected() == true) {
      return session_repp->connected_chainsetup_repp->position_in_seconds_exact(); 
  }
  else
    return(0.0);
}

/**
 * Return info about engine status.
 */
string ECA_CONTROL_BASE::engine_status(void) const {
  switch(session_repp->status()) {
  case ECA_SESSION::ep_status_running: 
    {
    return("running"); 
    }
  case ECA_SESSION::ep_status_stopped: 
    {
    return("stopped"); 
    }
  case ECA_SESSION::ep_status_finished:
    {
    return("finished"); 
    }
  case ECA_SESSION::ep_status_notready: 
    {
    return("not ready"); 
    }
  default: 
    {
    return("unknown status"); 
    }
  }
}

/**
 * Get a string containing a comma separated list of all chains 
 * attached to input with index 'aiod'. 
 *
 * require:
 *  is_selected() == true
 */
string ECA_CONTROL_BASE::attached_chains_input(AUDIO_IO* aiod) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  vector<string> t = session_repp->get_attached_chains_to_input(aiod);
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
 * require:
 *  is_selected() == true
 */
string ECA_CONTROL_BASE::attached_chains_output(AUDIO_IO* aiod) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  vector<string> t = session_repp->get_attached_chains_to_output(aiod);
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
 * require:
 *  is_selected() == true
 */
vector<string> ECA_CONTROL_BASE::attached_chains(const string& filename) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  return(selected_chainsetup_repp->get_attached_chains_to_iodev(filename));
}

/**
 * Set the default buffersize (in samples).
 *
 * require:
 *   is_editable() == true
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
 * require:
 *   is_editable() == true
 */
void ECA_CONTROL_BASE::toggle_raise_priority(bool v) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  session_repp->toggle_raised_priority(v);
}

void ECA_CONTROL_BASE::set_last_list_of_strings(const vector<string>& s) { 
  last_los_rep = s; 
  last_type_rep = "S";
}

void ECA_CONTROL_BASE::set_last_string(const string& s) { 
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

const vector<string>& ECA_CONTROL_BASE::last_list_of_strings(void) const { return(last_los_rep); }
const string& ECA_CONTROL_BASE::last_string(void) const { return(last_s_rep); }
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
}
