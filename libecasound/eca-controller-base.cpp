// ------------------------------------------------------------------------
// eca-controller-base.cpp: Base class providing basic functionality
//                          for controlling the ecasound library
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
#include "eca-controller-base.h"

#include "eca-error.h"
#include "eca-debug.h"

string ecasound_lockfile;

ECA_CONTROLLER_BASE::ECA_CONTROLLER_BASE (ECA_SESSION* psession) {
  session_rep = psession;
  selected_chainsetup_rep = psession->selected_chainsetup;
  engine_started = false;
  ::ecasound_lockfile = "/var/lock/ecasound.lck." + kvu_numtostr(getpid());
}

void ECA_CONTROLLER_BASE::start(bool ignore_lock) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (session_rep->status() == ep_status_running) return;
  ecadebug->control_flow("Controller/Processing started");

  while(::ecasound_queue.is_empty() == false) ::ecasound_queue.pop_front();
  if (session_rep->status() == ep_status_notready) {
    start_engine(ignore_lock);
  }

  if (is_engine_started() == false) {
    ecadebug->msg("(eca-controller) Can't start processing: couldn't start engine.");
    return;
  }  

  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_start, 0.0);

  // --------
  // ensure:
  assert(is_engine_started() == true);
  // --------
}

void ECA_CONTROLLER_BASE::run(void) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (session_rep->status() == ep_status_running) return;

  start(true);

  struct timespec sleepcount;
  sleepcount.tv_sec = 1;
  sleepcount.tv_nsec = 0;

  while(is_finished() == false) {
    ::nanosleep(&sleepcount, NULL);
  }      

  ecadebug->control_flow("Controller/Processing finished");

  // --------
  // ensure:
  assert(is_finished() == true);
  // --------
}

void ECA_CONTROLLER_BASE::stop(void) {
  // --------
  // require:
  assert(is_engine_started() == true);
  // --------

  if (session_rep->status() != ep_status_running) return;
  ecadebug->control_flow("Controller/Processing stopped");
  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_stop, 0.0);

  // --------
  // ensure:
  // assert(is_running() == false); 
  // -- there's a small timeout so assertion cannot be checked
  // --------
}

void ECA_CONTROLLER_BASE::stop_on_condition(void) {
  // --------
  // require:
  assert(is_engine_started() == true);
  // --------

  if (session_rep->status() != ep_status_running) return;
  ecadebug->control_flow("Controller/Processing stopped");
  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_stop, 0.0);
  struct timeval now;
  gettimeofday(&now, 0);
  struct timespec sleepcount;
  sleepcount.tv_sec = now.tv_sec + 60;
  sleepcount.tv_nsec = now.tv_usec * 1000;
  ::pthread_mutex_lock(&ecasound_stop_mutex);
  int res = ::pthread_cond_timedwait(&ecasound_stop_cond, &ecasound_stop_mutex, &sleepcount);
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-controller-base) Received stop-cond");
  ::pthread_mutex_unlock(&ecasound_stop_mutex);

  // --------
  // ensure:
  assert(is_running() == false); 
  // --------
}

void ECA_CONTROLLER_BASE::quit(void) {
  close_engine();
  int n = ECA_QUIT;
  throw(n);
}

void ECA_CONTROLLER_BASE::start_engine(bool ignore_lock) {
  // --------
  // require:
  assert(is_connected() == true);
  // --------

  if (engine_started == true) return;

  ifstream fin(ecasound_lockfile.c_str());
  if (!fin || ignore_lock) {
    pthread_attr_t th_attr;
    pthread_attr_init(&th_attr);
    //      pthread_attr_setschedpolicy(&th_attr, SCHED_FIFO);
    //      pthread_attr_setschedparam(&th_attr, &sparam);
    //      ecadebug->msg("(eca-controller) Using realtime-scheduling (SCHED_FIFO/10, engine-thread).");
    start_normal_thread(session_rep, retcode, &th_cqueue, &th_attr);
  }
  else {
    MESSAGE_ITEM mitem;
    mitem << "(eca-controller) Can't execute; processing module already running!" << 'c' << "\n";
    ecadebug->msg(ECA_DEBUG::system_objects,mitem.to_string());
  }
  fin.close();

  engine_started = true;

  // --------
  // ensure:
  assert(is_engine_started() == true);
  // --------
}

void ECA_CONTROLLER_BASE::close_engine(void) {
  if (!engine_started) return;
  ::ecasound_queue.push_back(ECA_PROCESSOR::ep_exit, 0.0);
  ::pthread_join(th_cqueue,NULL);
  engine_started = false;

  // --------
  // ensure:
  assert(is_engine_started() == false);
  // --------
}

bool ECA_CONTROLLER_BASE::is_valid(void) const {
  // --------
  // require:
  assert(is_selected());
  // --------

  return(selected_chainsetup_rep->is_valid());
}

bool ECA_CONTROLLER_BASE::is_connected(void) const {
  if (session_rep->connected_chainsetup == 0) return(false);
  return(session_rep->connected_chainsetup->is_valid());
}

bool ECA_CONTROLLER_BASE::is_selected(void) const { return(selected_chainsetup_rep != 0); } 
bool ECA_CONTROLLER_BASE::is_running(void) const { return(session_rep->status() == ep_status_running); } 
bool ECA_CONTROLLER_BASE::is_finished(void) const { return(session_rep->status() == ep_status_finished); } 

long ECA_CONTROLLER_BASE::length_in_samples(void) const { 
  if (is_connected() == true)
      return session_rep->connected_chainsetup->length_in_samples(); 
  else
    return(0);
}

double ECA_CONTROLLER_BASE::length_in_seconds_exact(void) const { 
  if (is_connected() == true) {
      return session_rep->connected_chainsetup->length_in_seconds_exact(); 
  }
  else
    return(0.0);
}

long ECA_CONTROLLER_BASE::position_in_samples(void) const { 
  if (is_connected() == true)
      return session_rep->connected_chainsetup->position_in_samples(); 
  else
    return(0);
}
double ECA_CONTROLLER_BASE::position_in_seconds_exact(void) const {
  if (is_connected() == true) {
      return session_rep->connected_chainsetup->position_in_seconds_exact(); 
  }
  else
    return(0.0);
}

string ECA_CONTROLLER_BASE::engine_status(void) const {
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

string ECA_CONTROLLER_BASE::attached_chains_input(AUDIO_IO* aiod) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  vector<string> t = session_rep->get_attached_chains_to_input(aiod);
  string out = "";
  vector<string>::const_iterator p = t.begin();
  while(p != t.end()) {
    out += *p;
    ++p;
    if (p != t.end()) out += ",";
  }
  return(out);
}

string ECA_CONTROLLER_BASE::attached_chains_output(AUDIO_IO* aiod) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  vector<string> t = session_rep->get_attached_chains_to_output(aiod);
  string out = "";
  vector<string>::const_iterator p = t.begin();
  while(p != t.end()) {
    out += *p;
    ++p;
    if (p != t.end()) out += ",";
  }
  return(out);
}

vector<string> ECA_CONTROLLER_BASE::attached_chains(const string& filename) const {
  // --------
  REQUIRE(is_selected() == true);
  // --------

  return(selected_chainsetup_rep->get_attached_chains_to_iodev(filename));
}

void ECA_CONTROLLER_BASE::set_buffersize(int bsize) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  selected_chainsetup_rep->set_buffersize(bsize); 
}

void ECA_CONTROLLER_BASE::toggle_raise_priority(bool v) { 
  // --------
  // require:
  assert(is_selected() == true);
  // --------
  session_rep->toggle_raised_priority(v);
}

void start_normal_thread(ECA_SESSION* session, int retcode, pthread_t*
			 th_ecasound_cqueue, pthread_attr_t* th_attr) {
  retcode = pthread_create(th_ecasound_cqueue, th_attr, start_normal, (void*)session);
  if (retcode != 0)
    throw(new ECA_ERROR("ECA-CONTROLLER", "Unable to create a new thread (start_normal)."));
}

void* start_normal(void* param) {
  ofstream fout(ecasound_lockfile.c_str());
  fout.close();
  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-controller) Engine-thread pid: " + kvu_numtostr(getpid()));
  start_normal((ECA_SESSION*)param);
  remove(ecasound_lockfile.c_str());
  return(0);
}

void start_normal(ECA_SESSION* session) {
  try {
    ECA_PROCESSOR epros (session);
    epros.exec();
  }
  catch(ECA_ERROR* e) {
    cerr << "---\n(eca-controller) ERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\n(eca-controller) Caught an unknown exception!\n";
  }
}
