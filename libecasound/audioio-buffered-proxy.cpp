// ------------------------------------------------------------------------
// audioio-buffered-proxy.cpp: Proxy class providing additional layer
//                             of buffering instances of class AUDIO_IO.
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
#include <signal.h>

#include "audioio-buffered-proxy.h"

AUDIO_IO_BUFFERED_PROXY::AUDIO_IO_BUFFERED_PROXY (AUDIO_IO_PROXY_SERVER *pserver, 
						  AUDIO_IO* aobject) 
  : pserver_repp(pserver),
    child_repp(aobject) 
{
  pserver_repp->register_client(child_repp);
  pbuffer_repp = pserver_repp->get_client_buffer(child_repp);
  xruns_rep = 0;
  finished_rep = false;

  // just in case the child object has already been configured
  fetch_child_data();
}

void AUDIO_IO_BUFFERED_PROXY::fetch_child_data(void) {
  if (child_repp->io_mode() == AUDIO_IO::io_read) 
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_read;
  else
    pbuffer_repp->io_mode_rep = AUDIO_IO::io_write;
  
  int channels = child_repp->channels();
  int buffersize = child_repp->buffersize();
  for(unsigned int n = 0; n < pbuffer_repp->sbufs_rep.size(); n++) {
    pbuffer_repp->sbufs_rep[n].number_of_channels(channels);
    pbuffer_repp->sbufs_rep[n].length_in_samples(buffersize);
  }
}

AUDIO_IO_BUFFERED_PROXY::~AUDIO_IO_BUFFERED_PROXY(void) {
  pserver_repp->unregister_client(child_repp);
  if (xruns_rep > 0) 
    cerr << "(audioio-buffered-proxy) There were total " << xruns_rep << " xruns." << endl;
}

bool AUDIO_IO_BUFFERED_PROXY::finished(void) const { return(finished_rep); }

long AUDIO_IO_BUFFERED_PROXY::length_in_samples(void) const { 
  return(child_repp->length_in_samples());
}

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
    if (pbuffer_repp->finished_rep.get() == 1) finished_rep = true;
    else {
      finished_rep = false;
      xruns_rep++;
      cerr << "Underrun! Exiting!" << endl;
      exit(0);
    }
  }
}

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
      finished_rep = false;
      xruns_rep++;
      cerr << "Overrun! Exiting" << endl;
      exit(0);
    }
  }
}

void AUDIO_IO_BUFFERED_PROXY::seek_position(void) { 
  bool was_running = false;
  if (pserver_repp->is_running() == true) {
    was_running = true;
    pserver_repp->stop();
    while(pserver_repp->is_running() != true) usleep(50000);
  }
  child_repp->seek_position_in_samples(position_in_samples());
  finished_rep = false;
  pbuffer_repp->reset();
  if (was_running == true) {
    pserver_repp->start();
    while(pserver_repp->is_full() != true) usleep(50000);
  }
}

void AUDIO_IO_BUFFERED_PROXY::open(void) throw(AUDIO_IO::SETUP_ERROR&) { 
  child_repp->open();
  fetch_child_data();
}

void AUDIO_IO_BUFFERED_PROXY::close(void) { 
  child_repp->close();
}
