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
#include <string>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h> /* gettimeofday() */
#include <errno.h> /* ETIMEDOUT */

#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>

#include "sample-specs.h"
#include "samplebuffer.h"
#include "eca-debug.h"
#include "audioio-proxy-server.h"
#include "audioio-proxy-server_impl.h"

using std::cerr;
using std::endl;

// --
// Select features

#define PROXY_PROFILING

// --
// Macro definitions

#ifdef PROXY_PROFILING
#define PROXY_PROFILING_INC(x)          ++(x)
#define PROXY_PROFILING_STATEMENT(x)    x
#else
#define PROXY_PROFILING_INC(x)          (void)0
#define PROXY_PROFILING_STATEMENT(x)    (void)0
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

  impl_repp = new AUDIO_IO_PROXY_SERVER_impl;

  thread_running_rep = false;

  pthread_cond_init(&impl_repp->full_cond_rep, NULL);
  pthread_mutex_init(&impl_repp->full_mutex_rep, NULL);
  pthread_cond_init(&impl_repp->stop_cond_rep, NULL);
  pthread_mutex_init(&impl_repp->stop_mutex_rep, NULL);
  pthread_cond_init(&impl_repp->flush_cond_rep, NULL);
  pthread_mutex_init(&impl_repp->flush_mutex_rep, NULL);

  running_rep.set(0);
  full_rep.set(0);
  full_request_rep.set(0);
  stop_request_rep.set(0);
  exit_request_rep.set(0);
  exit_ok_rep.set(0);

  impl_repp->profile_full_rep = 0;
  impl_repp->profile_no_processing_rep = 0;
  impl_repp->profile_not_full_anymore_rep = 0;
  impl_repp->profile_processing_rep = 0;
  impl_repp->profile_read_xrun_danger_rep = 0;
  impl_repp->profile_write_xrun_danger_rep = 0;
  impl_repp->profile_rounds_total_rep = 0;
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
    pthread_join(impl_repp->io_thread_rep, 0);
  }
  for(unsigned int p = 0; p < buffers_rep.size(); p++) {
    delete buffers_rep[p];
  }

  delete impl_repp;

  PROXY_PROFILING_STATEMENT(dump_profile_counters());
}

/**
 * Starts the proxy server.
 */
void AUDIO_IO_PROXY_SERVER::start(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) start");
  if (thread_running_rep != true) {
    int ret = pthread_create(&impl_repp->io_thread_rep,
			     0,
			     start_proxy_server_io_thread,
			     static_cast<void *>(this));
    if (ret != 0) {
      ecadebug->msg("(audio_io_proxy_server) pthread_create failed, exiting");
      exit(1);
    }
    
    thread_running_rep = true;
  }

  /*
   * Note! As the new recovery code is in place, this is not needed anymore.
   *       ... or is it? - testing again 15.10.2001
   */
#if 1
   if (sched_getscheduler(0) == SCHED_FIFO) {
     struct sched_param sparam;
     sparam.sched_priority = schedpriority_rep;
     if (pthread_setschedparam(impl_repp->io_thread_rep, SCHED_FIFO, &sparam) != 0)
       ecadebug->msg("(audioio-proxy-server) Unable to change scheduling policy to SCHED_FIFO!");
     else 
       ecadebug->msg("(audioio-proxy-server) Using realtime-scheduling (SCHED_FIFO).");
   }
#endif

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

#if 1
  if (thread_running_rep == true) {
    struct sched_param sparam;
    int policy = 0;
    pthread_getschedparam(impl_repp->io_thread_rep, &policy, &sparam);
    if (policy != SCHED_OTHER) {
      sparam.sched_priority = 0;
      if (pthread_setschedparam(impl_repp->io_thread_rep, SCHED_OTHER, &sparam) != 0)
	ecadebug->msg(ECA_DEBUG::info, 
    		      "(audioio-proxy-server) Unable to change scheduling policy SCHED_FIFO->SCHED_OTHER!");
      else 
	ecadebug->msg(ECA_DEBUG::system_objects,
    		      "(audioio-proxy-server) Changed scheduling policy  SCHED_FIFO->SCHED_OTHER.");
        }
  }
#endif
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
  if (is_running() == true &&
      clients_rep.size() > 0) {
    struct timeval now;
    gettimeofday(&now, 0);
    struct timespec sleepcount;
    sleepcount.tv_sec = now.tv_sec + 5;
    sleepcount.tv_nsec = now.tv_usec * 1000;
    int ret = 0;
    
    full_request_rep.set(1);
    pthread_mutex_lock(&impl_repp->full_mutex_rep);
    ret = pthread_cond_timedwait(&impl_repp->full_cond_rep, 
				   &impl_repp->full_mutex_rep,
				   &sleepcount);
    pthread_mutex_unlock(&impl_repp->full_mutex_rep);
    full_request_rep.set(0);

    if (ret != 0) {
      if (ret == -ETIMEDOUT)
	ecadebug->msg(ECA_DEBUG::info, "(audioio-proxy-server) wait_for_full failed; timeout");
      else
	ecadebug->msg(ECA_DEBUG::info, "(audioio-proxy-server) wait_for_full failed");
    }
  }
  else {
    ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_full failed; not running");
  }
}

/**
 * Function that blocks until the server signals 
 * that it has stopped.
 */
void AUDIO_IO_PROXY_SERVER::wait_for_stop(void) {
  struct timeval now;
  gettimeofday(&now, 0);
  struct timespec sleepcount;
  sleepcount.tv_sec = now.tv_sec + 5;
  sleepcount.tv_nsec = now.tv_usec * 1000;
  int ret = 0;

  if (running_rep.get() == 1) {
  pthread_mutex_lock(&impl_repp->stop_mutex_rep);
  ret = pthread_cond_timedwait(&impl_repp->stop_cond_rep, 
				 &impl_repp->stop_mutex_rep,
				 &sleepcount);
  }
  pthread_mutex_unlock(&impl_repp->stop_mutex_rep);

  if (ret != 0) {
    if (ret == -ETIMEDOUT)
      ecadebug->msg(ECA_DEBUG::info, "(audioio-proxy-server) wait_for_stop failed; timeout");
    else
      ecadebug->msg(ECA_DEBUG::info, "(audioio-proxy-server) wait_for_stop failed");
  }
}

/**
 * Function that blocks until the server signals 
 * that it has flushed all buffers (after 
 * exit request).
 */
void AUDIO_IO_PROXY_SERVER::wait_for_flush(void) {
  if (is_running() == true) {
    struct timeval now;
    gettimeofday(&now, 0);
    struct timespec sleepcount;
    sleepcount.tv_sec = now.tv_sec + 5;
    sleepcount.tv_nsec = now.tv_usec * 1000;
    int ret = 0;
    
    if (exit_ok_rep.get() == 0) {
      pthread_mutex_lock(&impl_repp->flush_mutex_rep);
      ret = pthread_cond_timedwait(&impl_repp->flush_cond_rep, 
				   &impl_repp->flush_mutex_rep,
				 &sleepcount);
      pthread_mutex_unlock(&impl_repp->flush_mutex_rep);
    }
  
    if (ret != 0) {
      if (ret == -ETIMEDOUT)
	ecadebug->msg(ECA_DEBUG::info, "(audioio-proxy-server) wait_for_flush failed; timeout");
      else
	ecadebug->msg(ECA_DEBUG::info, "(audioio-proxy-server) wait_for_flush failed");
    }
  }
  else {
    ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) wait_for_flush failed; not running");
  }
}

/**
 * Sends a signal notifying that server buffers
 * are fulls.
 */
void AUDIO_IO_PROXY_SERVER::signal_full(void) {
  pthread_mutex_lock(&impl_repp->full_mutex_rep);
  pthread_cond_broadcast(&impl_repp->full_cond_rep);
  pthread_mutex_unlock(&impl_repp->full_mutex_rep);
}

/**
 * Sends a signal notifying that server has
 * stopped.
 */
void AUDIO_IO_PROXY_SERVER::signal_stop(void) {
  pthread_mutex_lock(&impl_repp->stop_mutex_rep);
  pthread_cond_broadcast(&impl_repp->stop_cond_rep);
  pthread_mutex_unlock(&impl_repp->stop_mutex_rep);
}

/**
 * Sends a signal notifying that server has
 * flushed all buffers (after an exit request).
 */
void AUDIO_IO_PROXY_SERVER::signal_flush(void) {
  pthread_mutex_lock(&impl_repp->flush_mutex_rep);
  pthread_cond_broadcast(&impl_repp->flush_cond_rep);
  pthread_mutex_unlock(&impl_repp->flush_mutex_rep);
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
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-proxy-server) unregister_client " + aobject->name() + ".");
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

  int processed = 0;
  int passive_rounds = 0;
  bool one_time_full = false;

  /* set idle timeout to 10% of total buffersize */
  struct timespec sleepcount; /* was: 100ms */
  sleepcount.tv_sec = 0; 
  sleepcount.tv_nsec = buffersize_rep * buffercount_rep * 1000 / samplerate_rep / 10 * 1000000;
  
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audio_io_proxy_server) Using idle timeout of " +
		kvu_numtostr(sleepcount.tv_nsec) + 
		" nsecs.");

  while(true) {
    if (running_rep.get() == 0) {
      nanosleep(&sleepcount, 0);
      if (exit_request_rep.get() == 1) break;
      continue;
    }

    PROXY_PROFILING_INC(impl_repp->profile_rounds_total_rep);

    processed = 0;

    int min_free_space = buffercount_rep;

    PROXY_PROFILING_STATEMENT(impl_repp->looptimer_rep.start());

    for(unsigned int p = 0; p < clients_rep.size(); p++) {
      if (clients_rep[p] == 0 ||
	  buffers_rep[p]->finished_rep.get()) continue;

      int free_space = 0;

      if (buffers_rep[p]->io_mode_rep == AUDIO_IO::io_read) {
	free_space = buffers_rep[p]->write_space();
	if (free_space > 0) {

	  clients_rep[p]->read_buffer(buffers_rep[p]->sbufs_rep[buffers_rep[p]->writeptr_rep.get()]);
	  if (clients_rep[p]->finished() == true) buffers_rep[p]->finished_rep.set(1);
	  buffers_rep[p]->advance_write_pointer();
	  ++processed;

	  PROXY_PROFILING_STATEMENT( if (buffers_rep[p]->write_space() > 16 && one_time_full == true) { )
	  PROXY_PROFILING_INC(impl_repp->profile_read_xrun_danger_rep);
	  PROXY_PROFILING_STATEMENT( } )

	}
      }
      else {
	free_space = buffers_rep[p]->read_space();
	if (free_space > 0) {

	  clients_rep[p]->write_buffer(buffers_rep[p]->sbufs_rep[buffers_rep[p]->readptr_rep.get()]);
	  if (clients_rep[p]->finished() == true) buffers_rep[p]->finished_rep.set(1);
	  buffers_rep[p]->advance_read_pointer();
	  ++processed;

	  PROXY_PROFILING_STATEMENT( if (buffers_rep[p]->read_space() < 16  && one_time_full == true) { )
	  PROXY_PROFILING_INC(impl_repp->profile_write_xrun_danger_rep);
	  PROXY_PROFILING_STATEMENT( } )

	}
      }

      if (free_space < min_free_space) min_free_space = free_space;

    }

    if (stop_request_rep.get() == 1) {
      stop_request_rep.set(0);
      running_rep.set(0);
      signal_stop();
    }

    if (min_free_space == 0) passive_rounds++;
    else passive_rounds = 0;

    PROXY_PROFILING_STATEMENT(impl_repp->looptimer_rep.stop());

    if (processed == 0) {
      if (passive_rounds > 1) {
	/* case 1: nothing processed during x rounds ==> full, sleep */
	PROXY_PROFILING_INC(impl_repp->profile_full_rep);
	full_rep.set(1);
	signal_full();
	if (one_time_full != true) one_time_full = true;
      }
      else {
	/* case 2: nothing processed ==> sleep */
	PROXY_PROFILING_INC(impl_repp->profile_no_processing_rep);
      }
      nanosleep(&sleepcount, 0);
    }
    else {
      if (min_free_space > 4) {
	/* case 3: something processed; room in the buffers ==> not full */
	PROXY_PROFILING_INC(impl_repp->profile_not_full_anymore_rep);
	full_rep.set(0);
      }
      else {
	/* case 4: something processed; business as usual */
	PROXY_PROFILING_INC(impl_repp->profile_processing_rep);
      }
    }
  }
  flush();
  exit_ok_rep.set(1);
//    cerr << "Exiting proxy server thread." << endl;
}

void AUDIO_IO_PROXY_SERVER::dump_profile_counters(void) {
  if (impl_repp->profile_not_full_anymore_rep > 0) {
    std::cerr << "(audioio-proxy-server) *** profile begin ***" << endl;
    std::cerr << "Profile_full_rep: " << impl_repp->profile_full_rep << endl;
    std::cerr << "Profile_no_processing_rep: " << impl_repp->profile_no_processing_rep << endl;
    std::cerr << "Profile_not_full_anymore_rep: " << impl_repp->profile_not_full_anymore_rep << endl;
    std::cerr << "Profile_processing_rep: " << impl_repp->profile_processing_rep << endl;
    std::cerr << "Profile_read_xrun_danger_rep: " << impl_repp->profile_read_xrun_danger_rep << endl;
    std::cerr << "Profile_write_xrun_danger_rep: " << impl_repp->profile_write_xrun_danger_rep << endl;
    std::cerr << "Profile_rounds_total_rep: " << impl_repp->profile_rounds_total_rep << endl;
    std::cerr << "Fastest/slowest/average loop time: ";
    std::cerr << kvu_numtostr(impl_repp->looptimer_rep.min_duration_seconds() * 1000, 1);
    std::cerr << "/";
    std::cerr << kvu_numtostr(impl_repp->looptimer_rep.max_duration_seconds() * 1000, 1);
    std::cerr << "/";
    std::cerr << kvu_numtostr(impl_repp->looptimer_rep.average_duration_seconds() * 1000, 1);
    std::cerr << " msec." << endl;
    std::cerr << "(audioio-proxy-server) *** profile end   ***" << endl;
  }
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
	  
	  clients_rep[p]->write_buffer(buffers_rep[p]->sbufs_rep[buffers_rep[p]->readptr_rep.get()]);
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
