// ------------------------------------------------------------------------
// audioio-proxy-server.cpp: Audio i/o engine serving proxy clients.
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

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <kvutils/dbc.h>

#include "sample-specs.h"
#include "audioio-proxy-server.h"
#include "eca-debug.h"

// --
// Select features

#define PROXY_PROFILING

// --
// Macro definitions

#ifdef PROXY_PROFILING
#define PROXY_PROFILING_INC(x)    ++(x)
#else
#define PROXY_PROFILING_INC(x)    (void)0
#endif

// --
// Initialization of static member functions

const int AUDIO_IO_PROXY_SERVER::buffercount_default = 32;
const long int AUDIO_IO_PROXY_SERVER::buffersize_default = 1024;

/**
 * Helper function for starting the slave thread.
 */
void* start_proxy_server_io_thread(void *ptr) {
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_BLOCK, &sigset, 0);

  AUDIO_IO_PROXY_SERVER* pserver =
    static_cast<AUDIO_IO_PROXY_SERVER*>(ptr);
  pserver->io_thread();

  return 0;
}


/**
 * Constructor.
 */
AUDIO_IO_PROXY_SERVER::AUDIO_IO_PROXY_SERVER (void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) constructor");
  buffercount_rep = buffercount_default;
  buffersize_rep = buffersize_default;
  samplerate_rep = SAMPLE_SPECS::sample_rate_default;

  thread_running_rep = false;
  pthread_cond_init(&full_cond_rep, NULL);
  pthread_mutex_init(&full_mutex_rep, NULL);
  pthread_cond_init(&stop_cond_rep, NULL);
  pthread_mutex_init(&stop_mutex_rep, NULL);
  pthread_cond_init(&flush_cond_rep, NULL);
  pthread_mutex_init(&flush_mutex_rep, NULL);

  running_rep.set(0);
  full_rep.set(0);
  stop_request_rep.set(0);
  exit_request_rep.set(0);
  exit_ok_rep.set(0);

  profile_not_full_set_rep = 0;
  profile_pauses_rep = 0;
  profile_full_and_active_rep = 0;
  profile_full_set_rep = 0;
  profile_not_full_rep = 0;
}

/**
 * Destructor. Doesn't delete any client objects.
 */
AUDIO_IO_PROXY_SERVER::~AUDIO_IO_PROXY_SERVER(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) destructor");
  stop_request_rep.set(1);
  exit_request_rep.set(1);
  exit_ok_rep.set(0);
  if (thread_running_rep == true) {
    ::pthread_join(io_thread_rep, 0);
  }
  for(unsigned int p = 0; p < buffers_rep.size(); p++) {
    delete buffers_rep[p];
  }

#ifdef PROXY_PROFILING
  dump_profile_counters();
#endif
}

/**
 * Starts the proxy server.
 */
void AUDIO_IO_PROXY_SERVER::start(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) start");
  if (thread_running_rep != true) {
    int ret = pthread_create(&io_thread_rep,
			     0,
			     start_proxy_server_io_thread,
			     static_cast<void *>(this));
    if (ret != 0) {
      ecadebug->msg("(audio_io_proxy_server) pthread_create failed, exiting");
      exit(1);
    }
    if (sched_getscheduler(0) == SCHED_FIFO) {
      struct sched_param sparam;
      sparam.sched_priority = schedpriority_rep;
      if (::pthread_setschedparam(io_thread_rep, SCHED_FIFO, &sparam) != 0)
	ecadebug->msg("Unable to change scheduling policy to SCHED_FIFO!");
      else 
	ecadebug->msg("Using realtime-scheduling (SCHED_FIFO).");
    }
    thread_running_rep = true;
  }
  stop_request_rep.set(0);
  running_rep.set(1);
  full_rep.set(0);
  for(unsigned int p = 0; p < clients_rep.size(); p++) {
    buffers_rep[p]->finished_rep.set(0);
  }
  ecadebug->msg(ECA_DEBUG::system_objects, "(audio_io_proxy_server) starting processing");
}

/**
 * Stops the proxy server.
 */
void AUDIO_IO_PROXY_SERVER::stop(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) stop");
  stop_request_rep.set(1);
}

/**
 * Whether the proxy server has been started?
 */
bool AUDIO_IO_PROXY_SERVER::is_running(void) const { 
  if (running_rep.get() == 0) return(false); 
  return(true);
}

/**
 * Whether the proxy server buffers are full?
 */
bool AUDIO_IO_PROXY_SERVER::is_full(void) const { 
  if (full_rep.get() == 0) return(false); 
  return(true);
}

/**
 * Function that blocks until the server signals 
 * that all its buffers are full.
 */
void AUDIO_IO_PROXY_SERVER::wait_for_full(void) {
  wait_for_full_debug_rep.set(1);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_full entry");
  ::pthread_mutex_lock(&full_mutex_rep);
  if (full_rep.get() == 0) 
    ::pthread_cond_wait(&full_cond_rep, 
			&full_mutex_rep);
  pthread_mutex_unlock(&full_mutex_rep);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_full ok");
  wait_for_full_debug_rep.set(0);
}

/**
 * Function that blocks until the server signals 
 * that it has stopped.
 */
void AUDIO_IO_PROXY_SERVER::wait_for_stop(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_stop entry");
  ::pthread_mutex_lock(&stop_mutex_rep);
  if (running_rep.get() == 1)
    ::pthread_cond_wait(&stop_cond_rep, 
			&stop_mutex_rep);
  pthread_mutex_unlock(&stop_mutex_rep);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_stop ok");
}

/**
 * Function that blocks until the server signals 
 * that it has flushed all buffers (after 
 * exit request).
 */
void AUDIO_IO_PROXY_SERVER::wait_for_flush(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_flush entry");
  ::pthread_mutex_lock(&flush_mutex_rep);
  if (exit_ok_rep.get() == 0)
    ::pthread_cond_wait(&flush_cond_rep, 
			&flush_mutex_rep);
  pthread_mutex_unlock(&flush_mutex_rep);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_flush ok");
}

/**
 * Sends a signal notifying that server buffers
 * are fulls.
 */
void AUDIO_IO_PROXY_SERVER::signal_full(void) {
  if (wait_for_full_debug_rep.get() == 1)
    ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) signal_full entry");
  pthread_mutex_lock(&full_mutex_rep);
  pthread_cond_signal(&full_cond_rep);
  pthread_mutex_unlock(&full_mutex_rep);
  if (wait_for_full_debug_rep.get() == 1)
    ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) signal_full ok");
}

/**
 * Sends a signal notifying that server has
 * stopped.
 */
void AUDIO_IO_PROXY_SERVER::signal_stop(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) signal_stop entry");
  pthread_mutex_lock(&stop_mutex_rep);
  pthread_cond_signal(&stop_cond_rep);
  pthread_mutex_unlock(&stop_mutex_rep);
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) signal_stop ok");
}

/**
 * Sends a signal notifying that server has
 * flushed all buffers (after an exit request).
 */
void AUDIO_IO_PROXY_SERVER::signal_flush(void) {
  //  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) signal_flush");
  pthread_mutex_lock(&flush_mutex_rep);
  pthread_cond_signal(&flush_cond_rep);
  pthread_mutex_unlock(&flush_mutex_rep);
}


/**
 * Sets new default values for sample buffers.
 */
void AUDIO_IO_PROXY_SERVER::set_buffer_defaults(int buffers, 
						long int buffersize, 
						long int sample_rate) { 
  buffercount_rep = buffers;
  buffersize_rep = buffersize;
  samplerate_rep = sample_rate;
}

/**
 * Registers a new client object.
 *
 * @pre aobject != 0
 */
void AUDIO_IO_PROXY_SERVER::register_client(AUDIO_IO* aobject) { 
  // --
  DBC_REQUIRE(aobject != 0);
  // --
  
  clients_rep.push_back(aobject);
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-proxy-server) Registering client " +
		kvu_numtostr(clients_rep.size() - 1) +
		". Buffer count " +
		kvu_numtostr(buffercount_rep) +
		", and size " +
		kvu_numtostr(buffersize_rep) +
		".");
  buffers_rep.push_back(new AUDIO_IO_PROXY_BUFFER(buffercount_rep,
						  buffersize_rep,
						  SAMPLE_SPECS::channel_count_default,
						  samplerate_rep));
  client_map_rep[aobject] = clients_rep.size() - 1;
}

/**
 * Unregisters the client object given as the argument. No
 * resources are freed during this call.
 */
void AUDIO_IO_PROXY_SERVER::unregister_client(AUDIO_IO* aobject) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) unregister_client " + aobject->label() + ".");
  if (client_map_rep.find(aobject) != client_map_rep.end()) {
    size_t index = client_map_rep[aobject];
    if (index >= 0 && index < clients_rep.size()) 
      clients_rep[index] = 0;
    else 
      ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) unregister_client failed (1)");
  }
  else 
    ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) unregister_client failed (2)");
      
}

/**
 * Gets a pointer to the client buffer belonging to 
 * the audio object given as parameter. If no
 * buffers are found (client not registered, etc), 
 * null is returned.
 */
AUDIO_IO_PROXY_BUFFER* AUDIO_IO_PROXY_SERVER::get_client_buffer(AUDIO_IO* aobject) {
  if (client_map_rep.find(aobject) == client_map_rep.end() ||
      clients_rep[client_map_rep[aobject]] == 0)
    return(0);

  return(buffers_rep[client_map_rep[aobject]]);
}

/**
 * Slave thread.
 */
void AUDIO_IO_PROXY_SERVER::io_thread(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audio_io_proxy_server) Hey, in the I/O loop!");
  //  long int sleep_counter = 0;
  int prev_processed = 0;
  int processed = 0;
  while(true) {
    if (running_rep.get() == 0) {
      usleep(50000);
      if (exit_request_rep.get() == 1) break;
      continue;
    }

    processed = 0;

    for(unsigned int p = 0; p < clients_rep.size(); p++) {
      if (clients_rep[p] == 0 ||
	  buffers_rep[p]->finished_rep.get()) continue;
      if (buffers_rep[p]->io_mode_rep == AUDIO_IO::io_read) {
	if (buffers_rep[p]->write_space() > 0) {
	  clients_rep[p]->read_buffer(&buffers_rep[p]->sbufs_rep[buffers_rep[p]->writeptr_rep.get()]);
	  if (clients_rep[p]->finished() == true) buffers_rep[p]->finished_rep.set(1);

	  //  	  if (buffers_rep[p]->write_space() > 16) {
	  //   	    cerr << "Writing buffer " << buffers_rep[p]->writeptr_rep.get() << " of client " << p << ";";
	  //   	    cerr << " write_space: " << buffers_rep[p]->write_space() << ";";
	  //	    cerr << "Sleepcount " << sleep_counter << "." << endl;
	  //	    sleep_counter = 0;
	  //  	  }

	  buffers_rep[p]->advance_write_pointer();
	  ++processed;
	}
      }
      else {
	if (buffers_rep[p]->read_space() > 0) {

	  //  	  if (buffers_rep[p]->read_space() > 16) {
	  //  	    cerr << "Reading buffer " << buffers_rep[p]->readptr_rep.get() << " of client " << p << ";";
	  //  	    cerr << " read_space: " << buffers_rep[p]->read_space() << "." << endl;
	  //  	  }

	  clients_rep[p]->write_buffer(&buffers_rep[p]->sbufs_rep[buffers_rep[p]->readptr_rep.get()]);
	  if (clients_rep[p]->finished() == true) buffers_rep[p]->finished_rep.set(1);
	  buffers_rep[p]->advance_read_pointer();
	  ++processed;
	}
      }
    }

    if (stop_request_rep.get() == 1) {
      stop_request_rep.set(0);
      running_rep.set(0);
      signal_stop();
    }

    if (full_rep.get() == 1) {
      if (processed != 0) { /*  && prev_processed != 0 */
	/* case 1 - active iteration so there seems to be free space in the buffers */
	full_rep.set(0);
	PROXY_PROFILING_INC(profile_not_full_set_rep);
      }
      else if (processed == 0 && prev_processed == 0) {
	/* case 2 - nothing processed for two iterations -> wait for a moment */
	PROXY_PROFILING_INC(profile_pauses_rep);
	struct timespec sleepcount;
	sleepcount.tv_sec = 0;
	sleepcount.tv_nsec = 100000;
	::nanosleep(&sleepcount, 0);
      }
      else 
	PROXY_PROFILING_INC(profile_full_and_active_rep);
    }
    else {
      if (processed == 0 && prev_processed == 0) {
	/* case 3 - buffers have become full */
	full_rep.set(1);
	signal_full();
	PROXY_PROFILING_INC(profile_full_set_rep);
      }
      else
	PROXY_PROFILING_INC(profile_not_full_rep);
    }
    prev_processed = processed;
  }
  flush();
  exit_ok_rep.set(1);
//    cerr << "Exiting proxy server thread." << endl;
}

void AUDIO_IO_PROXY_SERVER::dump_profile_counters(void) {
  std::cerr << "(audioio-proxy-server) *** profile begin ***" << endl;
  std::cerr << "profile_not_full_set_rep: " << profile_not_full_set_rep << endl;
  std::cerr << "profile_pauses_rep: " << profile_pauses_rep << endl;
  std::cerr << "profile_full_and_active_rep: " << profile_full_and_active_rep << endl;
  std::cerr << "profile_full_set_rep: " << profile_full_set_rep << endl;
  std::cerr << "profile_not_full_rep: " << profile_not_full_rep <<  endl;
  std::cerr << "(audioio-proxy-server) *** profile end   ***" << endl;
}

/**
 * Flushes all data in the buffers to disk.
 */
void AUDIO_IO_PROXY_SERVER::flush(void) {
  int not_finished = 1;
  while(not_finished != 0) {
    not_finished = 0;
    for(unsigned int p = 0; p < clients_rep.size(); p++) {
      if (clients_rep[p] == 0 ||
	  buffers_rep[p]->finished_rep.get()) continue;
      if (buffers_rep[p]->io_mode_rep != AUDIO_IO::io_read) {
	if (buffers_rep[p]->read_space() > 0) {
	  ++not_finished;

	  ecadebug->msg(std::string("(audio_io_proxy_server) ") +
			"Flushing buffer " + 
			kvu_numtostr(buffers_rep[p]->readptr_rep.get()) +
			" of client " +
			kvu_numtostr(p) +
			" read_space: " +
			kvu_numtostr(buffers_rep[p]->read_space()) +
			".");
	  
	  clients_rep[p]->write_buffer(&buffers_rep[p]->sbufs_rep[buffers_rep[p]->readptr_rep.get()]);
	  if (clients_rep[p]->finished() == true) buffers_rep[p]->finished_rep.set(1);
	  buffers_rep[p]->advance_read_pointer();
	}
      }
    }
  }
  for(unsigned int p = 0; p < buffers_rep.size(); p++) {
    buffers_rep[p]->reset();
  }
  signal_flush();
}
