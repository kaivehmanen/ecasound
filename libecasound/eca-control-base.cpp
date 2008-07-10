// ------------------------------------------------------------------------
// eca-control-base.cpp: Base class providing basic functionality
//                       for controlling the ecasound library
// Copyright (C) 1999-2004,2006,2008 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3 (see Ecasound Programmer's Guide)
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

#include <string>
#include <vector>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <kvu_dbc.h>
#include <kvu_utils.h>
#include <kvu_numtostr.h>
#include <kvu_message_item.h>
#include <kvu_value_queue.h>

#include "eca-engine.h"
#include "eca-session.h"
#include "eca-chainsetup.h"
#include "eca-control-base.h"
#include "eca-resources.h"

#include "eca-error.h"
#include "eca-logger.h"

/**
 * Import namespaces
 */
using std::list;
using std::string;
using std::vector;

/**
 * Definitions for member functions
 */

/**
 * Helper function for starting the slave thread.
 */
void* ECA_CONTROL_BASE::start_normal_thread(void *ptr)
{
  ECA_CONTROL_BASE* ctrl_base = static_cast<ECA_CONTROL_BASE*>(ptr);
  ctrl_base->engine_pid_rep = getpid();
  DBC_CHECK(ctrl_base->engine_pid_rep >= 0);

  ECA_LOG_MSG(ECA_LOGGER::system_objects, "Engine thread started with pid: " + kvu_numtostr(ctrl_base->engine_pid_rep));

  ctrl_base->run_engine();

  ECA_LOG_MSG(ECA_LOGGER::system_objects,  
	      "Engine thread " + kvu_numtostr(ctrl_base->engine_pid_rep) + " will exit.\n");
  ctrl_base->engine_pid_rep = -1;

  return 0;
}

ECA_CONTROL_BASE::ECA_CONTROL_BASE (ECA_SESSION* psession)
{
  session_repp = psession;
  selected_chainsetup_repp = psession->selected_chainsetup_repp;
  engine_repp = 0;
  engine_pid_rep = -1;
  engine_exited_rep.set(0);
  float_to_string_precision_rep = 3;
  joining_rep = false;
}

ECA_CONTROL_BASE::~ECA_CONTROL_BASE (void)
{
  close_engine();
}

/**
 * Initializes the engine
 *
 * @pre is_connected() == true
 * @pre is_engine_started() != true
 */
void ECA_CONTROL_BASE::engine_start(void)
{
  // --------
  DBC_REQUIRE(is_connected() == true);
  DBC_REQUIRE(is_engine_started() != true);
  // --------

  start_engine_sub(false);
}

/**
 * Start the processing engine
 *
 * @pre is_connected() == true
 * @pre is_running() != true
 * @post is_engine_started() == true
 *
 * @return negative on error, zero on success 
 */
int ECA_CONTROL_BASE::start(void)
{
  // --------
  DBC_REQUIRE(is_connected() == true);
  DBC_REQUIRE(is_running() != true);
  // --------
  
  int result = 0;

  ECA_LOG_MSG(ECA_LOGGER::subsystems, "Controller/Processing started");

  if (is_engine_started() != true) {
    /* request_batchmode=false */
    start_engine_sub(false);
  }

  if (is_engine_started() != true) {
    ECA_LOG_MSG(ECA_LOGGER::info, "Can't start processing: couldn't start engine.");
    result = -1;
  }  
  else {
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);
  }

  // --------
  DBC_ENSURE(result != 0 || is_engine_started() == true);
  // --------

  return result;
}

/**
 * Starts the processing engine and blocks until 
 * processing is finished.
 *
 * @param batchmode if true, runs until finished/stopped state is reached, and
 *        then returns; if false, will run infinitely
 *
 * @pre is_connected() == true
 * @pre is_running() != true
 * @post is_finished() == true || 
 *       processing_started == true && is_running() != true ||
 *       processing_started != true &&
 *       (is_engine_started() != true ||
 *        is_engine_started() == true &&
 * 	  engine_repp->status() != ECA_ENGINE::engine_status_stopped))
 *
 * @return negative on error, zero on success 
 */
int ECA_CONTROL_BASE::run(bool batchmode)
{
  // --------
  DBC_REQUIRE(is_connected() == true);
  DBC_REQUIRE(is_running() != true);
  // --------

  ECA_LOG_MSG(ECA_LOGGER::subsystems, "Controller/Starting batch processing");

  bool processing_started = false;
  int result = -1;

  if (is_engine_started() != true) {
    /* request_batchmode=true */
    start_engine_sub(batchmode);
  }

  if (is_engine_started() != true) {
    ECA_LOG_MSG(ECA_LOGGER::info, "Can't start processing: couldn't start the engine. (2)");
  } 
  else { 
    engine_repp->command(ECA_ENGINE::ep_start, 0.0);

    DBC_CHECK(is_finished() != true);
    
    result = 0;

    /* run until processing is finished; in batchmode run forever (or
     * until error occurs) */
    while(is_finished() != true || batchmode != true) {
      
      /* sleep for one second */
      kvu_sleep(1, 0);

      if (processing_started != true) {
	if (is_running() == true ||
	    is_finished() == true ||
	    engine_exited_rep.get() == 1) {
	  /* make a note that engine state changed to 'running' */
	  processing_started = true;
	}
	else if (is_engine_started() == true) {
	  if (engine_repp->status() == ECA_ENGINE::engine_status_error) {
	    /* not running, so status() is either 'not_ready' or 'error' */
	    ECA_LOG_MSG(ECA_LOGGER::info, "Can't start processing: engine startup failed. (3)");
	    result = -2;
	    break;
	  }
	  /* other valid state alternatives: */
	  DBC_CHECK(engine_repp->status() == ECA_ENGINE::engine_status_stopped ||
		    engine_repp->status() == ECA_ENGINE::engine_status_notready);
	}
	else {
	  /* ECA_CONTROL_BASE destructor has been run and 
	   * engine_repp is now 0 (--> is_engine_started() != true) */
	  break;
	}
      }
      else {
	/* engine was started succesfully (processing_started == true) */
	if (is_running() != true) {
	  /* operation succesfully completed, exit from run() unless
	   * infinite operation is requested (batchmode) */
	  if (batchmode == true) break;
	}
      }
    }
  }    

  if (last_exec_res_rep < 0) {
    /* error occured during processing */
    result = -3;
  }

  ECA_LOG_MSG(ECA_LOGGER::subsystems, 
	      std::string("Controller/Batch processing finished (")
	      + kvu_numtostr(result) + ")");
  
  // --------
  DBC_ENSURE(is_finished() == true ||
	     (processing_started == true && is_running()) != true ||
	     (processing_started != true &&
	      (is_engine_started() != true ||
	       (is_engine_started() == true &&
		engine_repp->status() != ECA_ENGINE::engine_status_stopped))));
  // --------

  return result;
}

/**
 * Stops the processing engine.
 *
 * @pre is_engine_started() == true
 * @pre is_running() == true
 * @post is_running() == false
 */
void ECA_CONTROL_BASE::stop(void)
{
  // --------
  DBC_REQUIRE(is_engine_started() == true);
  DBC_REQUIRE(is_running() == true);
  // --------

  ECA_LOG_MSG(ECA_LOGGER::subsystems, "Controller/Processing stopped");
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
void ECA_CONTROL_BASE::stop_on_condition(void)
{
  // --------
  DBC_REQUIRE(is_engine_started() == true);
  // --------

  if (engine_repp->status() != ECA_ENGINE::engine_status_running) return;
  ECA_LOG_MSG(ECA_LOGGER::subsystems, "Controller/Processing stopped (cond)");
  engine_repp->command(ECA_ENGINE::ep_stop, 0.0);
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "Received stop-cond");

  // --
  // blocks until engine has stopped (or 5 sec has passed);
  engine_repp->wait_for_stop(5);

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
void ECA_CONTROL_BASE::start_engine_sub(bool batchmode)
{
  // --------
  DBC_REQUIRE(is_connected() == true);
  DBC_REQUIRE(is_engine_started() != true);
  // --------

  DBC_CHECK(engine_exited_rep.get() != 1);

  unsigned int p = session_repp->connected_chainsetup_repp->first_selected_chain();
  if (p < session_repp->connected_chainsetup_repp->chains.size())
    session_repp->connected_chainsetup_repp->active_chain_index_rep = p;

  engine_repp = new ECA_ENGINE (session_repp->connected_chainsetup_repp);

  /* to relay the batchmode parameter to created new thread */
  req_batchmode_rep = batchmode;

  pthread_attr_t th_attr;
  pthread_attr_init(&th_attr);
  int retcode_rep = pthread_create(&th_cqueue_rep,
				   &th_attr,
				   start_normal_thread, 
				   static_cast<void *>(this));
  if (retcode_rep != 0) {
    ECA_LOG_MSG(ECA_LOGGER::info, "WARNING: Unable to create a new thread for engine.");
    delete engine_repp;
    engine_repp = 0;
  }
}

/**
 * Routine used for launching the engine.
 */
void ECA_CONTROL_BASE::run_engine(void)
{
  last_exec_res_rep = 0;
  last_exec_res_rep = engine_repp->exec(req_batchmode_rep);
  engine_exited_rep.set(1); 
}

/**
 * Closes the processing engine.
 *
 * ensure:
 *  is_engine_started() != true
 */
void ECA_CONTROL_BASE::close_engine(void)
{
  if (is_engine_started() != true) return;

  engine_repp->command(ECA_ENGINE::ep_exit, 0.0);

  ECA_LOG_MSG(ECA_LOGGER::system_objects, "Waiting for engine thread to exit.");
  if (joining_rep != true) {
    joining_rep = true;
    int res = pthread_join(th_cqueue_rep,NULL);
    joining_rep = false;
    ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"pthread_join returned: " 
		+ kvu_numtostr(res));
  }
  else {
    DBC_CHECK(engine_pid_rep >= 0);
    int i;
    for (i = 0; i < 30; i++) { 
      if (engine_exited_rep.get() ==1)
	break;
      usleep(100000);
    }
    ECA_LOG_MSG(ECA_LOGGER::system_objects, "Engine stuck, sending SIGKILL.");
    kill(engine_pid_rep, SIGKILL);
  }

  if (engine_exited_rep.get() == 1) {
    ECA_LOG_MSG(ECA_LOGGER::system_objects, "Engine thread has exited succesfully.");
    delete engine_repp;
    engine_repp = 0;
    engine_exited_rep.set(0);
  }
  else {
    ECA_LOG_MSG(ECA_LOGGER::info, "WARNING: Problems while shutting down the engine!");
  }

  // ---
  DBC_ENSURE(is_engine_started() != true);
  // ---
}

/**
 * Is currently selected chainsetup valid?
 *
 * @pre is_selected()
 */
bool ECA_CONTROL_BASE::is_valid(void) const
{
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  /* use is_valid_for_connection() instead of is_valid() to 
   * report any detected errors via the logging subsystem */
  return selected_chainsetup_repp->is_valid_for_connection(true);
}

/**
 * Returns true if active chainsetup exists and is connected.
 */
bool ECA_CONTROL_BASE::is_connected(void) const
{
  if (session_repp->connected_chainsetup_repp == 0) {
    return false;
  }

  return (session_repp->connected_chainsetup_repp->is_valid() &&
	  session_repp->connected_chainsetup_repp->is_enabled());
}

/**
 * Returns true if some chainsetup is selected.
 */
bool ECA_CONTROL_BASE::is_selected(void) const { return selected_chainsetup_repp != 0; } 

/**
 * Returns true if processing engine is running.
 */
bool ECA_CONTROL_BASE::is_running(void) const { return (is_engine_started() == true && engine_repp->status() == ECA_ENGINE::engine_status_running); } 

/**
 * Returns true if engine has finished processing. Engine state is 
 * either "finished" or "error".
 */
bool ECA_CONTROL_BASE::is_finished(void) const
{
  return (is_engine_started() == true && 
	  (engine_repp->status() == ECA_ENGINE::engine_status_finished ||
	   engine_repp->status() == ECA_ENGINE::engine_status_error)); 
} 

string ECA_CONTROL_BASE::resource_value(const string& key) const
{ 
  ECA_RESOURCES ecarc;
  return ecarc.resource(key); 
}

/**
 * Returns the length of the selected chainsetup (in samples).
 *
 * @pre is_selected() == true
 */
SAMPLE_SPECS::sample_pos_t ECA_CONTROL_BASE::length_in_samples(void) const
{
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  SAMPLE_SPECS::sample_pos_t cslen = 0;
  if (selected_chainsetup_repp->length_set() == true) {
    cslen = selected_chainsetup_repp->length_in_samples();
  }
  if (selected_chainsetup_repp->max_length_set() == true) {
    cslen = selected_chainsetup_repp->max_length_in_samples();
  }

  return cslen;
}

/**
 * Returns the length of the selected chainsetup (in seconds).
 *
 * @pre is_selected() == true
 */
double ECA_CONTROL_BASE::length_in_seconds_exact(void) const
{
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  double cslen = 0.0f;
  if (selected_chainsetup_repp->length_set() == true) {
    cslen = selected_chainsetup_repp->length_in_seconds_exact();
  }
  if (selected_chainsetup_repp->max_length_set() == true) {
    cslen = selected_chainsetup_repp->max_length_in_seconds_exact();
  }

  return cslen;
}

/**
 * Returns the current position of the selected chainsetup (in samples).
 *
 * @pre is_selected() == true
 */
SAMPLE_SPECS::sample_pos_t ECA_CONTROL_BASE::position_in_samples(void) const
{
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  return selected_chainsetup_repp->position_in_samples();
}

/**
 * Returns the current position of the selected chainsetup (in seconds).
 *
 * @pre is_selected() == true
 */
double ECA_CONTROL_BASE::position_in_seconds_exact(void) const
{
  // --------
  DBC_REQUIRE(is_selected());
  // --------

  return selected_chainsetup_repp->position_in_seconds_exact();
}

/**
 * Returns true if engine has been started. 
 *
 * Note: when in batch mode, status of notready means that
 *       engine has exited and is no longer receiving any commands
 *       and thus false is returned
 */
bool ECA_CONTROL_BASE::is_engine_started(void) const
{
  bool ret = true;

  if (engine_repp == 0) {
    ret = false;
  }
  else {
    if (engine_repp->batch_mode() == true &&
	engine_repp->status() == ECA_ENGINE::engine_status_notready) {
      ret = false;
    }
  }
  
  return ret;
}

/**
 * Return info about engine status.
 */
string ECA_CONTROL_BASE::engine_status(void) const
{
  if (is_engine_started() == true) {
    switch(engine_repp->status()) {
    case ECA_ENGINE::engine_status_running: 
      {
	return "running"; 
      }
    case ECA_ENGINE::engine_status_stopped: 
      {
	return "stopped"; 
      }
    case ECA_ENGINE::engine_status_finished:
      {
	return "finished"; 
      }
    case ECA_ENGINE::engine_status_error:
      {
	return "error"; 
      }
    case ECA_ENGINE::engine_status_notready: 
      {
	return "not ready"; 
      }
    default: 
      {
	return "unknown status"; 
      }
    }
  }
  return "not started";
}

/**
 * Get a string containing a comma separated list of all chains 
 * attached to input with index 'aiod'. 
 *
 * @pre is_selected() == true
 */
string ECA_CONTROL_BASE::attached_chains_input(AUDIO_IO* aiod) const
{
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
  return out;
}

/**
 * Get a string containing a comma separated list of all chains 
 * attached to output with index 'aiod'.
 *
 * @pre is_selected() == true
 */
string ECA_CONTROL_BASE::attached_chains_output(AUDIO_IO* aiod) const
{
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
  return out;
}

/**
 * Get a string containing a comma separated list of all chains 
 * attached to audio object with name 'filename'. 
 *
 * @pre is_selected() == true
 */
vector<string> ECA_CONTROL_BASE::attached_chains(const string& filename) const
{
  // --------
  DBC_REQUIRE(is_selected() == true);
  // --------

  return selected_chainsetup_repp->get_attached_chains_to_iodev(filename);
}

void ECA_CONTROL_BASE::set_last_string(const list<string>& s)
{
  // use resize() instead of clear(); clear() was a late
  // addition to C++ standard and not supported by all
  // compilers (for example egcs-2.91.66) 
  last_s_rep.resize(0);

  DBC_CHECK(last_s_rep.size() == 0);
  list<string>::const_iterator p = s.begin();
  while(p != s.end()) {
    last_s_rep += *p;
    ++p;
    if (p != s.end()) last_s_rep += "\n";
  }
  last_type_rep = "s";
}

void ECA_CONTROL_BASE::set_last_string_list(const vector<string>& s)
{
  last_los_rep = s; 
  last_type_rep = "S";
}

void ECA_CONTROL_BASE::set_last_string(const string& s)
{
  last_s_rep = s; 
  last_type_rep = "s";
}

void ECA_CONTROL_BASE::set_last_float(double v)
{
  last_f_rep = v; 
  last_type_rep = "f";
}

void ECA_CONTROL_BASE::set_last_integer(int v)
{
  last_i_rep = v; 
  last_type_rep = "i";
}
 
void ECA_CONTROL_BASE::set_last_long_integer(long int v)
{
  last_li_rep = v; 
  last_type_rep = "li";
}

void ECA_CONTROL_BASE::set_last_error(const string& s)
{
  last_error_rep = s;
  last_type_rep = "e";
}

const vector<string>& ECA_CONTROL_BASE::last_string_list(void) const { return last_los_rep; }
const string& ECA_CONTROL_BASE::last_string(void) const { return last_s_rep; }
double ECA_CONTROL_BASE::last_float(void) const { return last_f_rep; }
int ECA_CONTROL_BASE::last_integer(void) const { return last_i_rep; } 
long int ECA_CONTROL_BASE::last_long_integer(void) const { return last_li_rep; }
const string& ECA_CONTROL_BASE::last_error(void) const { return last_error_rep; }
const string& ECA_CONTROL_BASE::last_type(void) const { return last_type_rep; }

void ECA_CONTROL_BASE::clear_last_values(void)
{ 
  last_los_rep.clear();
  last_s_rep = "";
  last_li_rep = 0;
  last_i_rep = 0;
  last_f_rep = 0.0f;
  last_error_rep = "";
  last_type_rep = "-";
}

void ECA_CONTROL_BASE::set_float_to_string_precision(int precision)
{
  float_to_string_precision_rep = precision;
}
std::string ECA_CONTROL_BASE::float_to_string(double n) const
{
  return kvu_numtostr(n, float_to_string_precision_rep);
}
