// ------------------------------------------------------------------------
// audioio-buffered-proxy.cpp: Proxy class providing additional layer
//                             of buffering instances of class AUDIO_IO.
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

#include <unistd.h> /* open(), close() */

#include "eca-debug.h"
#include "audioio-buffered-proxy.h"

/**
 * Constructor. The given client object is registered to 
 * the given proxy server as a client object.
 *
 * Ownership of 'aobject' is transfered to this proxy
 * object if 'transfer_ownership' is true.
 */
AUDIO_IO_BUFFERED_PROXY::AUDIO_IO_BUFFERED_PROXY (AUDIO_IO_PROXY_SERVER *pserver, 
						  AUDIO_IO* aobject,
						  bool transfer_ownership) 
  : pserver_repp(pserver),
    child_repp(aobject),
    free_child_rep(transfer_ownership) 
{
  label(child_repp->label());
  pbuffer_repp = 0;
  xruns_rep = 0;
  finished_rep = false;

  ecadebug->msg(ECA_DEBUG::user_objects, 
		std::string("(audioio-buffered-proxy) Proxy created for ") +
		child_repp->label() +
		".");

  if (child_repp->is_open() == true) {
    // just in case the child object has already been configured
    fetch_child_data();
  }
}

/**
 * Copy attributes from the proxied (child) object.
 */
void AUDIO_IO_BUFFERED_PROXY::fetch_child_data(void) {
  if (child_repp->io_mode() == AUDIO_IO::io_read) 
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_read;
  else
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_write;

  set_audio_format(child_repp->audio_format());

  position_in_samples(child_repp->position_in_samples());
  
  int channels = child_repp->channels();
  int buffersize = child_repp->buffersize();
  for(unsigned int n = 0; n < pbuffer_repp->sbufs_rep.size(); n++) {
    pbuffer_repp->sbufs_rep[n].number_of_channels(channels);
    pbuffer_repp->sbufs_rep[n].length_in_samples(buffersize);
  }
}

/**
 * Desctructor. Unregisters the client from the proxy 
 * server.
 */
AUDIO_IO_BUFFERED_PROXY::~AUDIO_IO_BUFFERED_PROXY(void) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-buffered-proxy) destructor " + label() + ".");
  if (pbuffer_repp != 0) {
    pserver_repp->unregister_client(child_repp);
  }
  
  if (free_child_rep == true) {
    delete child_repp;
  }
  child_repp = 0;
    
  if (xruns_rep > 0) 
    std::cerr << "(audioio-buffered-proxy) There were total " << xruns_rep << " xruns." << std::endl;
}

/**
 * Whether all data has been processed? If opened in mode 'io_read', 
 * this means that end of stream has been reached. If opened in 
 * 'io_write' or 'io_readwrite' modes, finished status usually
 * means that an error has occured (no space left, etc). After 
 * finished() has returned 'true', further calls to read_buffer() 
 * and/or write_buffer() won't process any data.
 */
bool AUDIO_IO_BUFFERED_PROXY::finished(void) const { return(finished_rep); }

/**
 * Reads samples to buffer pointed by 'sbuf'. If necessary, the target 
 * buffer will be resized.
 */
void AUDIO_IO_BUFFERED_PROXY::read_buffer(SAMPLE_BUFFER* sbuf) { 
  if (pbuffer_repp->read_space() > 0) {
    sbuf->operator=(pbuffer_repp->sbufs_rep[pbuffer_repp->readptr_rep.get()]);
    pbuffer_repp->advance_read_pointer();
    position_in_samples_advance(sbuf->length_in_samples());
    // FIXME: make the threshold configurable
    //      if (pbuffer_repp->read_space() < 16) {
    // //        cerr << "Client read " << pbuffer_repp->readptr_rep.get() << ";";
    // //        cerr << " read_space: " << pbuffer_repp->read_space() << "." << endl;
    //        ::sched_yield();
    //      }
  }
  else {
    if (pbuffer_repp->finished_rep.get() == 1) {
      finished_rep = true;
      sbuf->length_in_samples(0);
    }
    else {
      xruns_rep++;
      ecadebug->msg(ECA_DEBUG::user_objects, 
		    std::string("(audioio-buffered-proxy) Warning! Underrun for ") +
		    child_repp->label() +
		    ". Trying to recover.");
      pserver_repp->wait_for_full(); 
      if (pbuffer_repp->read_space() > 0) {
	this->read_buffer(sbuf);
      }
      else {
	std::cerr << "(audioio-buffered-proxy) Serious trouble with the disk-io subsystem! (1)" << std::endl;
	sbuf->length_in_samples(0);
      }
    }
  }
}

/**
 * Writes all data from sample buffer pointed by 'sbuf'. Notes
 * concerning read_buffer() also apply to this routine.
 */
void AUDIO_IO_BUFFERED_PROXY::write_buffer(SAMPLE_BUFFER* sbuf) { 
  if (pbuffer_repp->write_space() > 0) {
    pbuffer_repp->sbufs_rep[pbuffer_repp->writeptr_rep.get()].operator=(*sbuf);
    pbuffer_repp->advance_write_pointer();
    position_in_samples_advance(sbuf->length_in_samples());
    extend_position();
  }
  else {
    if (pbuffer_repp->finished_rep.get() == 1) finished_rep = true;
    else {
      xruns_rep++;
      ecadebug->msg(ECA_DEBUG::user_objects, 
		    std::string("(audioio-buffered-proxy) Warning! Overrun for ") +
		    child_repp->label() +
		    ". Trying to recover.");
      pserver_repp->wait_for_full(); 
      if (pbuffer_repp->write_space() > 0) {
	this->write_buffer(sbuf);
      }
      else {
	std::cerr << "(audioio-buffered-proxy) Serious trouble with the disk-io subsystem! (2)" << std::endl;
      }
    }
  }
}

/**
 * Seeks to the current position.
 */
void AUDIO_IO_BUFFERED_PROXY::seek_position(void) { 
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-buffered-proxy) seek " + label() + ".");
  bool was_running = false;
  if (pserver_repp->is_running() == true) {
    was_running = true;
    pserver_repp->stop();
    pserver_repp->wait_for_stop();
    //  while(pserver_repp->is_running() != true) usleep(50000);
  }
  child_repp->seek_position_in_samples(position_in_samples());
  pserver_repp->wait_for_stop();
  finished_rep = false;
  if (was_running == true) {
    pserver_repp->start();
    pserver_repp->wait_for_full();
    //  while(pserver_repp->is_full() != true) usleep(50000);
  }
}

/**
 * Opens the child audio object (possibly in exclusive mode).
 * This routine is meant for opening files and devices,
 * loading libraries, etc. 
 */
void AUDIO_IO_BUFFERED_PROXY::open(void) throw(AUDIO_IO::SETUP_ERROR&) { 
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-buffered-proxy) open " + label() + ".");
  if (child_repp->is_open() != true) {
    child_repp->open();
  }
  if (pbuffer_repp == 0) {
    pserver_repp->register_client(child_repp);
    pbuffer_repp = pserver_repp->get_client_buffer(child_repp);
  }
  fetch_child_data();
  toggle_open_state(true);
}

/**
 * Closes the child audio object. After calling this routine, 
 * all resources (ie. soundcard) must be freed
 * (they can be used by other processes).
 */
void AUDIO_IO_BUFFERED_PROXY::close(void) { 
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-buffered-proxy) close " + label() + ".");
  child_repp->close();
  toggle_open_state(false);
}
