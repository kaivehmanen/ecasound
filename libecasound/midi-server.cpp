// ------------------------------------------------------------------------
// midi-server.cpp: MIDI i/o engine serving proxy clients.
// Copyright (C) 2001 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include <sys/time.h>

#include <kvutils/kvu_numtostr.h>
#include "midi-server.h"
#include "eca-debug.h"

const unsigned int MIDI_SERVER::max_queue_size_rep = 32768;

/**
 * Helper function for starting the slave thread.
 */
void* start_midi_server_io_thread(void *ptr) {
  MIDI_SERVER* mserver =
    static_cast<MIDI_SERVER*>(ptr);
  mserver->io_thread();
}


/**
 * Constructor.
 */
MIDI_SERVER::MIDI_SERVER (void) { 
  running_status_rep = 0;
  current_ctrl_channel_rep = -1;
  current_ctrl_number = -1;
  thread_running_rep = false;
  running_rep.set(0);
  stop_request_rep.set(0);
  exit_request_rep.set(0);
}

/**
 * Destructor. Doesn't delete any client objects.
 */
MIDI_SERVER::~MIDI_SERVER(void) { 
  stop_request_rep.set(1);
  exit_request_rep.set(1);
  if (thread_running_rep == true) {
    ::pthread_join(io_thread_rep, 0);
  }
}

/**
 * Starts the proxy server.
 */
void MIDI_SERVER::start(void) { 
  if (thread_running_rep != true) {
    int ret = pthread_create(&io_thread_rep,
			     0,
			     start_midi_server_io_thread,
			     static_cast<void *>(this));
    if (ret != 0) {
      ecadebug->msg("(midi-server) pthread_create failed, exiting");
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
    stop_request_rep.set(0);
    running_rep.set(1);
  }
  ecadebug->msg(ECA_DEBUG::user_objects, "(midi-server) starting processing");
}

/**
 * Stops the proxy server.
 */
void MIDI_SERVER::stop(void) { 
  stop_request_rep.set(1);
  ecadebug->msg(ECA_DEBUG::user_objects, "(midi-server) stopping processing");
}

/**
 * Whether the MIDI server has been started?
 */
bool MIDI_SERVER::is_running(void) const { 
  if (running_rep.get() == 0) return(false); 
  return(true);
}

/**
 * Registers a new client object. Midi server doesn't
 * handle initializing and opening of client objects.
 */
void MIDI_SERVER::register_client(MIDI_IO* mobject) { 
  clients_rep.push_back(mobject);
  ecadebug->msg(ECA_DEBUG::user_objects, 
		"Registering client " +
		kvu_numtostr(clients_rep.size() - 1) +
		".");
}

/**
 * Unregisters the client object given as the argument. No
 * resources are freed during this call.
 */
void MIDI_SERVER::unregister_client(MIDI_IO* mobject) { 
  for(unsigned int n = 0; n < clients_rep.size(); n++) {
    if (clients_rep[n] == mobject) {
      clients_rep[n] = 0;
      break;
    }
  }
}

/**
 * Slave thread.
 */
void MIDI_SERVER::io_thread(void) { 
  fd_set fds;
  unsigned char buf[3];
  int temp;
  
  ecadebug->msg(ECA_DEBUG::user_objects, "(midi-server) Hey, in the I/O loop!");
  while(true) {
    if (running_rep.get() == 0) {
      usleep(50000);
      if (exit_request_rep.get() == 1) break;
      continue;
    }

    assert(clients_rep.size() > 0);
    assert(clients_rep[0]->supports_nonblocking_mode() == true);
    int fd = clients_rep[0]->file_descriptor();

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    int retval = select(fd + 1 , &fds, NULL, NULL, NULL);
    
    if (retval) {
      temp = ::read(fd, buf, 3);
    }

    if (temp < 0) {
      cerr << "ERROR: Can't read from MIDI-device: " 
	   << clients_rep[0]->label() << "." << endl;
      break;
    }
    else {
      for(int n = 0; n < temp; n++) {
	buffer_rep.push_back(buf[n]);
	while(buffer_rep.size() > max_queue_size_rep) {
	  cerr << "(eca-midi) dropping midi bytes" << endl;
	  buffer_rep.pop_front();
	}
      }
      parse_receive_queue();
    }
      
    if (stop_request_rep.get() == 1) {
      stop_request_rep.set(0);
      running_rep.set(0);
    }
  }
  cerr << "Exiting MIDI-server thread." << endl;
}

/**
 * Whether 'byte' belong to Voice Category status messages
 * (ie. 0x80 to 0xef)?
 */
bool MIDI_SERVER::is_voice_category_status_byte(unsigned char byte) {
  if (byte >= 0x80 && byte < 0xf0) return(true);
  return(false);
}

/**
 * Whether 'byte' belong to System Common Category status messages
 * (ie. 0xf0 to 0xf7)?
 */
bool MIDI_SERVER::is_system_common_category_status_byte(unsigned char byte) {
  if (byte >= 0xf0 && byte < 0xf8) return(true);
  return(false);
}

/**
 * Whether 'byte' belong to Realtime Category status messages
 * (ie. 0xf8 to 0xff)?
 */
bool MIDI_SERVER::is_realtime_category_status_byte(unsigned char byte) {
  if (byte > 0xf7) return(true);
  return(false);
}

/**
 * Whether 'byte' is a statuus byte (0x80 to 0xff)?
 */
bool MIDI_SERVER::is_status_byte(unsigned char byte) {
  if (byte & 0x80) return(true);
  return(false);
}

/**
 * Requests that server will follow the latest value of 
 * controller 'ctrl' on channel 'channel'.
 */
void MIDI_SERVER::add_controller_trace(int channel, int ctrl) {
  controller_values_rep[pair<int,int>(channel,ctrl)] = 0;
}

/**
 * Requests that server stops following the latest value of
 * controller 'ctrl' on channel 'channel'.
 */
void MIDI_SERVER::remove_controller_trace(int channel, int controller) {
  map<pair<int,int>,int>::iterator p = controller_values_rep.find(pair<int,int>(channel,controller));
  if (p != controller_values_rep.end()) {
    controller_values_rep.erase(p);
  }
}

/**
 * Returns the latest traced value of controller 'ctrl' on 
 * channel 'channel'.
 */
int MIDI_SERVER::last_controller_value(int channel, int ctrl) const {  
  map<pair<int,int>,int>::iterator p = controller_values_rep.find(pair<int,int>(channel,ctrl));
  if (p != controller_values_rep.end()) {
    return(controller_values_rep[pair<int,int>(channel,ctrl)]);
  }
  return(0);
}

/**
 * Parses the received MIDI date.
 */
void MIDI_SERVER::parse_receive_queue(void) {

  while(buffer_rep.size() > 0) {
    unsigned char byte = buffer_rep.front();
    buffer_rep.pop_front();

    if (is_status_byte(byte) == true) {
      if (is_voice_category_status_byte(byte) == true) {
	running_status_rep = byte;
	if ((running_status_rep & 0xb0) == 0xb0)
	  current_ctrl_channel_rep = static_cast<int>((byte & 15));
      }
      else if (is_system_common_category_status_byte(byte) == true) {
	current_ctrl_channel_rep = -1;
	running_status_rep = 0;
      }
    }
    else { /* non-status bytes */
      /** 
       * Any data bytes are ignored if no running status
       */
      if (running_status_rep != 0) {

	/**
	 * Check for 'controller messages' (status 0xb0 to 0xbf and
	 * two data bytes)
	 */
	if (current_ctrl_channel_rep != -1) {
	  if (current_ctrl_number == -1) {
	    current_ctrl_number = static_cast<int>(byte);
//      	    cerr << endl << "C:" << current_ctrl_number << ".";
	  }
	  else {
	    if (controller_values_rep.find(pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)) 
		!= controller_values_rep.end()) {
	      controller_values_rep[pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)] = static_cast<int>(byte);
//    	      cerr << endl << "(midi-server) Value:" 
//    		   << controller_values_rep[pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)] 
//    		   << ", ch:" << current_ctrl_channel_rep 
//    		   << ", ctrl:" << current_ctrl_number << ".";
	    }
//  	    else {
//  	      cerr << endl << "E:" << " found an entry we are not following..." << endl;
//  	    }
	    current_ctrl_number = -1;
	  }
	}
      }
    }
  }
}
