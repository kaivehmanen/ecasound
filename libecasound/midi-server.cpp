// ------------------------------------------------------------------------
// midi-server.cpp: MIDI i/o engine serving generic clients.
// Copyright (C) 2001-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "midi-parser.h"
#include "midi-server.h"
#include "eca-logger.h"

const unsigned int MIDI_SERVER::max_queue_size_rep = 32768;

/**
 * Helper function for starting the slave thread.
 */
void* start_midi_server_io_thread(void *ptr)
{
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_BLOCK, &sigset, 0);

  MIDI_SERVER* mserver =
    static_cast<MIDI_SERVER*>(ptr);
  mserver->io_thread();
  return(0);
}

/**
 * Slave thread.
 */
void MIDI_SERVER::io_thread(void)
{
  fd_set fds;
  unsigned char buf[3];
  struct timeval tv;
  
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(midi-server) Hey, in the I/O loop!");
  while(true) {
    if (running_rep.get() == 0 ||
	clients_rep[0]->is_open() != true) {
      usleep(50000);
      if (exit_request_rep.get() == 1) break;
      continue;
    }

    DBC_CHECK(clients_rep.size() > 0);
    DBC_CHECK(clients_rep[0]->supports_nonblocking_mode() == true);
    int fd = clients_rep[0]->poll_descriptor();

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int retval = select(fd + 1 , &fds, NULL, NULL, &tv);

    int read_bytes = 0;
    if (retval) {
      if (FD_ISSET(fd, &fds) == true) {
	// FIXME:: should use MIDI_IO::write_bytes() instead,
	//         poll_descriptor is only meant for poll() and select()
	read_bytes = ::read(fd, buf, 1);
      }
    }

    if (read_bytes < 0) {
      std::cerr << "ERROR: Can't read from MIDI-device: " 
	   << clients_rep[0]->label() << "." << std::endl;
      break;
    }
    else {
      // cerr << "(midi-server) read bytes: " << read_bytes << endl;
      for(int n = 0; n < read_bytes; n++) {
	buffer_rep.push_back(buf[n]);
	while(buffer_rep.size() > max_queue_size_rep) {
	  std::cerr << "(eca-midi) dropping midi bytes" << std::endl;
	  buffer_rep.pop_front();
	}
	for(unsigned int m = 0; m < handlers_rep.size(); m++) {
	  MIDI_HANDLER* p = handlers_rep[m];
	  if (p != 0) p->insert(buf[n]);
	}
      }
      parse_receive_queue();
    }
      
    if (stop_request_rep.get() == 1) {
      stop_request_rep.set(0);
      running_rep.set(0);
    }
  }
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(midi-server) exiting MIDI-server thread");
}

/**
 * Constructor.
 */
MIDI_SERVER::MIDI_SERVER (void)
{
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
MIDI_SERVER::~MIDI_SERVER(void)
{
  if (is_enabled() == true) disable();
}

/**
 * Starts the MIDI server.
 *
 * ensure
 *  is_running() == true
 */
void MIDI_SERVER::start(void)
{
  stop_request_rep.set(0);
  running_rep.set(1);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(midi-server) starting processing");
  send_mmc_start();
  if (is_midi_sync_send_enabled() == true) send_midi_start();

  // --------
  DBC_ENSURE(is_running() == true);
  // --------
}

/**
 * Stops the MIDI-server. Note that this routine only 
 * initializes the stop procedure. Processing will
 * stop once the i/o-thread acknowledges the stop request.
 */
void MIDI_SERVER::stop(void)
{
  stop_request_rep.set(1);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(midi-server) stopping processing");
  send_mmc_stop();
  if (is_midi_sync_send_enabled() == true) send_midi_stop();
}

/**
 * Initializes the MIDI-server by resetting 
 * all MIDI-related state info.
 */
void MIDI_SERVER::init(void)
{
  running_status_rep = 0;
  current_ctrl_channel_rep = -1;
  current_ctrl_number = -1;
}

/**
 * Enables the MIDI-server subsystems and prepared them for processing. 
 * 
 * ensure:
 *  is_enabled() == true
 */
void MIDI_SERVER::enable(void)
{
  init();
  running_rep.set(0);
  stop_request_rep.set(0);
  exit_request_rep.set(0);
  if (thread_running_rep != true) {
    ECA_LOG_MSG(ECA_LOGGER::user_objects, "(midi-server) enabling");
    int ret = pthread_create(&io_thread_rep,
			     0,
			     start_midi_server_io_thread,
			     static_cast<void *>(this));
    if (ret != 0) {
      ECA_LOG_MSG(ECA_LOGGER::info, "(midi-server) pthread_create failed, exiting");
      exit(1);
    }
#ifdef HAVE_SCHED_GETSCHEDULER
    if (sched_getscheduler(0) == SCHED_FIFO) {
#ifdef HAVE_PTHREAD_SETSCHEDPARAM
      struct sched_param sparam;
      sparam.sched_priority = schedpriority_rep;
      if (::pthread_setschedparam(io_thread_rep, SCHED_FIFO, &sparam) != 0)
	ECA_LOG_MSG(ECA_LOGGER::info, "Unable to change scheduling policy to SCHED_FIFO!");
      else 
	ECA_LOG_MSG(ECA_LOGGER::info, "Using realtime-scheduling (SCHED_FIFO).");
#endif
    }
#else
	ECA_LOG_MSG(ECA_LOGGER::info, "Warning! Functions to query scheduler settings not available!");
#endif
    thread_running_rep = true;
  }

  // --------
  DBC_ENSURE(is_enabled() == true);
  // --------  
}

/**
 * Whether MIDI-server is enabled?
 */
bool MIDI_SERVER::is_enabled(void) const { return(thread_running_rep); }

/**
 * Disables the MIDI-server subsystems.
 * 
 * require:
 *  is_enabled() == true
 *
 * ensure:
 *  is_running() != true
 *  is_enabled() != true
 */
void MIDI_SERVER::disable(void)
{
  // --------
  DBC_REQUIRE(is_enabled() == true);
  // --------

  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(midi-server) disabling");
  stop_request_rep.set(1);
  exit_request_rep.set(1);
  if (thread_running_rep == true) {
    ::pthread_join(io_thread_rep, 0);
  }
  thread_running_rep = false;

  // --------
  DBC_ENSURE(is_running() != true);
  DBC_ENSURE(is_enabled() != true);
  // --------
}

/**
 * Whether the MIDI server has been started?
 */
bool MIDI_SERVER::is_running(void) const
{
  if (running_rep.get() == 0) return(false); 
  return(true);
}

/**
 * Registers a new client object. Midi server doesn't
 * handle initializing and opening of client objects.
 */
void MIDI_SERVER::register_client(MIDI_IO* mobject)
{
  clients_rep.push_back(mobject);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"Registering client " +
		kvu_numtostr(clients_rep.size() - 1) +
		".");
}

/**
 * Unregisters the client object given as the argument. No
 * resources are freed during this call.
 */
void MIDI_SERVER::unregister_client(MIDI_IO* mobject)
{
  for(unsigned int n = 0; n < clients_rep.size(); n++) {
    if (clients_rep[n] == mobject) {
      clients_rep[n] = 0;
      break;
    }
  }
}

/**
 * Registers a new MIDI-handler. The server will send 
 * all received MIDI-data to the handler.
 */
void MIDI_SERVER::register_handler(MIDI_HANDLER* object)
{
  handlers_rep.push_back(object);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"Registering handler " +
		kvu_numtostr(handlers_rep.size() - 1) +
		".");
}

/**
 * Unregisters the handler object given as the argument. No
 * resources are freed during this call.
 */
void MIDI_SERVER::unregister_handler(MIDI_HANDLER* object)
{ 
  for(unsigned int n = 0; n < handlers_rep.size(); n++) {
    if (handlers_rep[n] == object) {
      handlers_rep[n] = 0;
      break;
    }
  }
}

/**
 * Adds a new client to which MMC-messages are sent
 * during processing. 
 *
 * Note! Id '127' is specified as the all-device 
 *       id-number in the MMC-spec.
 */
void MIDI_SERVER::add_mmc_send_id(int id)
{
  mmc_send_ids_rep.push_back(id);
}

/**
 * Removes a MMC-message client.
 */
void MIDI_SERVER::remove_mmc_send_id(int id)
{
  mmc_send_ids_rep.remove(id);
}


/**
 * Sends MMC-start to all MMC-send client device ids.
 *
 * require:
 *  is_enabled() == true
 */
void MIDI_SERVER::send_midi_bytes(int dev_id, unsigned char* buf, int bytes) {
  // --------
  DBC_REQUIRE(is_enabled() == true);
  // --------
  
  if (clients_rep[dev_id - 1]->is_open() == true) {
    DBC_CHECK(static_cast<int>(clients_rep.size()) >= dev_id);
    DBC_CHECK(clients_rep[dev_id - 1]->supports_nonblocking_mode() == true);

    int fd = clients_rep[dev_id - 1]->poll_descriptor();

    // FIXME:: should use MIDI_IO::write_bytes() instead,
    //         poll_descriptor is only meant for poll() and select()
    int err = ::write(fd, buf, bytes);
    DBC_CHECK(err == bytes);
  }
}

/**
 * Sends an MMC-command to all MMC-send client device ids.
 */
void MIDI_SERVER::send_mmc_command(unsigned int cmd)
{
  unsigned char buf[6];
  buf[0] = 0xf0;
  buf[1] = 0x7f;
  buf[2] = 0x00; /* dev-id */
  buf[3] = 0x06;
  buf[4] = cmd;
  buf[5] = 0xf7;
  std::list<int>::const_iterator p = mmc_send_ids_rep.begin();
  while(p != mmc_send_ids_rep.end()) {
    ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(midi-server) Sending MMC message " + 
		kvu_numtostr(cmd) + " to device-id " +
		kvu_numtostr(*p) + ".");
    buf[2] = static_cast<unsigned char>(*p);
    send_midi_bytes(1, buf, 6);
    ++p;
  }
}

/**
 * Sends MMC-start to all MMC-send client device ids.
 */
void MIDI_SERVER::send_mmc_start(void)
{ 
  /* FIXME: should this be 0x03 (deferred play)? */
  // send_mmc_command(0x02); 
  send_mmc_command(0x03); 
}

/**
 * Sends MMC-stop to all MMC-send client device ids.
 */
void MIDI_SERVER::send_mmc_stop(void)
{ 
  send_mmc_command(0x01);
}

/**
 * Sends a MIDI-start message.
 */
void MIDI_SERVER::send_midi_start(void)
{ 
  unsigned char byte[1] = { 0xfa };

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
	      "(midi-server) Sending MIDI-start message.");

  send_midi_bytes(1, byte, 1);
}

/**
 * Sends a MIDI-continue message.
 */
void MIDI_SERVER::send_midi_continue(void)
{
  unsigned char byte[1] = { 0xfb };

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
	      "(midi-server) Sending MIDI-continue message.");

  send_midi_bytes(1, byte, 1);
}

/**
 * Sends a MIDI-stop message.
 */
void MIDI_SERVER::send_midi_stop(void)
{
  unsigned char byte[1] = { 0xfc };

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
	      "(midi-server) Sending MIDI-stop message.");

  send_midi_bytes(1, byte, 1);
}

/**
 * Requests that server will follow the latest value of 
 * controller 'ctrl' on channel 'channel'.
 */
void MIDI_SERVER::add_controller_trace(int channel, int ctrl)
{
  controller_values_rep[std::pair<int,int>(channel,ctrl)] = 0;
}

/**
 * Requests that server stops following the latest value of
 * controller 'ctrl' on channel 'channel'.
 */
void MIDI_SERVER::remove_controller_trace(int channel, int controller)
{
  std::map<std::pair<int,int>,int>::iterator p = controller_values_rep.find(std::pair<int,int>(channel,controller));
  if (p != controller_values_rep.end()) {
    controller_values_rep.erase(p);
  }
}

/**
 * Returns the latest traced value of controller 'ctrl' on 
 * channel 'channel'.
 */
int MIDI_SERVER::last_controller_value(int channel, int ctrl) const
{
  std::map<std::pair<int,int>,int>::iterator p = controller_values_rep.find(std::pair<int,int>(channel,ctrl));
  if (p != controller_values_rep.end()) {
    return(controller_values_rep[std::pair<int,int>(channel,ctrl)]);
  }
  return(0);
}

/**
 * Parses the received MIDI date.
 */
void MIDI_SERVER::parse_receive_queue(void)
{
  while(buffer_rep.size() > 0) {
    unsigned char byte = buffer_rep.front();
    buffer_rep.pop_front();

    if (MIDI_PARSER::is_status_byte(byte) == true) {
      if (MIDI_PARSER::is_voice_category_status_byte(byte) == true) {
	running_status_rep = byte;
	if ((running_status_rep & 0xb0) == 0xb0)
	  current_ctrl_channel_rep = static_cast<int>((byte & 15));
      }
      else if (MIDI_PARSER::is_system_common_category_status_byte(byte) == true) {
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
	    // cerr << endl << "C:" << current_ctrl_number << ".";
	  }
	  else {
	    if (controller_values_rep.find(std::pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)) 
		!= controller_values_rep.end()) {
	      controller_values_rep[std::pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)] = static_cast<int>(byte);
	      // cerr << endl << "(midi-server) Value:" 
	      //      << controller_values_rep[pair<int,int>(current_ctrl_channel_rep,current_ctrl_number)] 
	      //      << ", ch:" << current_ctrl_channel_rep 
	      //      << ", ctrl:" << current_ctrl_number << ".";
	    }
	    // else {
	    //   cerr << endl << "E:" << " found an entry we are not following..." << endl;
	    // }
	    current_ctrl_number = -1;
	  }
	}
      }
    }
  }
}
