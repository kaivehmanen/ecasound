// ------------------------------------------------------------------------
// audioio-proxy-server.cpp: Audio i/o engine serving proxy clients.
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

#include <unistd.h>
#include "sample-specs.h"
#include "audioio-proxy-server.h"
#include "eca-debug.h"

const int AUDIO_IO_PROXY_SERVER::buffercount_default = 32;
const long int AUDIO_IO_PROXY_SERVER::buffersize_default = 1024;

/**
 * Helper function for starting the slave thread.
 */
void* start_proxy_server_io_thread(void *ptr) {
  AUDIO_IO_PROXY_SERVER* pserver =
    static_cast<AUDIO_IO_PROXY_SERVER*>(ptr);
  pserver->io_thread();
}


/**
 * Constructor.
 */
AUDIO_IO_PROXY_SERVER::AUDIO_IO_PROXY_SERVER (void) { 
  buffercount_rep = buffercount_default;
  buffersize_rep = buffersize_default;
  samplerate_rep = SAMPLE_SPECS::sample_rate_default;
  thread_running_rep = false;
  running_rep.set(0);
  full_rep.set(0);
  stop_request_rep.set(0);
  exit_request_rep.set(0);
}

/**
 * Destructor. Doesn't delete any client objects.
 */
AUDIO_IO_PROXY_SERVER::~AUDIO_IO_PROXY_SERVER(void) { 
  stop_request_rep.set(1);
  exit_request_rep.set(1);
  if (thread_running_rep == true) {
    ::pthread_join(io_thread_rep, 0);
  }
  for(unsigned int p = 0; p < buffers_rep.size(); p++) {
    delete buffers_rep[p];
  }
}

/**
 * Starts the proxy server.
 */
void AUDIO_IO_PROXY_SERVER::start(void) { 
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
  ecadebug->msg(ECA_DEBUG::user_objects, "(audio_io_proxy_server) starting processing");
}

/**
 * Stops the proxy server.
 */
void AUDIO_IO_PROXY_SERVER::stop(void) { 
  stop_request_rep.set(1);
  ecadebug->msg(ECA_DEBUG::user_objects, "(audio_io_proxy_server) stopping processing");
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
 */
void AUDIO_IO_PROXY_SERVER::register_client(AUDIO_IO* aobject) { 
  clients_rep.push_back(aobject);
  ecadebug->msg(ECA_DEBUG::user_objects, 
		"Registering client " +
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
  clients_rep[client_map_rep[aobject]] = 0;
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
  ecadebug->msg(ECA_DEBUG::user_objects, "(audio_io_proxy_server) Hey, in the I/O loop!");
  long int sleep_counter = 0;
  while(true) {
    if (running_rep.get() == 0) {
      usleep(50000);
      if (exit_request_rep.get() == 1) break;
      continue;
    }
    int processed = 0;
    for(unsigned int p = 0; p < clients_rep.size(); p++) {
      if (clients_rep[p] == 0 ||
	  buffers_rep[p]->finished_rep.get()) continue;
      if (buffers_rep[p]->io_mode_rep == AUDIO_IO::io_read) {
	if (buffers_rep[p]->write_space() > 0) {
	  clients_rep[p]->read_buffer(&buffers_rep[p]->sbufs_rep[buffers_rep[p]->writeptr_rep.get()]);
	  if (clients_rep[p]->finished() == true) buffers_rep[p]->finished_rep.set(1);
//    	  if (buffers_rep[p]->write_space() > 16) {
//    	    cerr << "Writing buffer " << buffers_rep[p]->writeptr_rep.get() << " of client " << p << ";";
//    	    cerr << " write_space: " << buffers_rep[p]->write_space() << ";";
//  	    cerr << "Sleepcount " << sleep_counter << "." << endl;
//  	    sleep_counter = 0;
//    	  }
	  buffers_rep[p]->advance_write_pointer();
	  ++processed;
	}
      }
      else {
	if (buffers_rep[p]->read_space() > 0) {
//  	  if (buffers_rep[p]->read_space() > 16) {
//  	    cerr << "Reading buffer " << buffers_rep[p]->readptr_rep.get() << " of client " 
//  		 << p << ";";
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
    }
    if (processed != 0) {
      full_rep.set(0);
      continue;
    }
    else {
      full_rep.set(1);
    }
    if (processed == 0) {
      ++sleep_counter;
      if (full_rep.get() == 1) {
	struct timespec sleepcount;
	sleepcount.tv_sec = 0;
	sleepcount.tv_nsec = 100000;
	::nanosleep(&sleepcount, 0);
      }
    }
  }
  flush();
//    cerr << "Exiting proxy server thread." << endl;
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
	  cerr << "Flushing buffer " << buffers_rep[p]->readptr_rep.get() << " of client " 
	       << p << ";";
	  cerr << " read_space: " << buffers_rep[p]->read_space() << "." << endl;
	  
	  clients_rep[p]->write_buffer(&buffers_rep[p]->sbufs_rep[buffers_rep[p]->readptr_rep.get()]);
	  if (clients_rep[p]->finished() == true) buffers_rep[p]->finished_rep.set(1);
	  buffers_rep[p]->advance_read_pointer();
	}
      }
    }
  }
}
