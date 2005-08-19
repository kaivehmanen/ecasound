// ------------------------------------------------------------------------
// midiio-aseq.cpp: Input and output of MIDI streams using 
//                  ALSA Sequencer
// Copyright (C) 2005 Pedro Lopez-Cabanillas
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

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include "midiio-aseq.h"

#include "eca-logger.h"

MIDI_IO_ASEQ::MIDI_IO_ASEQ(const std::string& name) { label("alsaseq"); device_name_rep = name; }

MIDI_IO_ASEQ::~MIDI_IO_ASEQ(void) { if (is_open()) close(); }

void MIDI_IO_ASEQ::open(void) { 
  int open_flags = 0, port_flags = 0;

  switch(io_mode()) {
  case io_read:
    {
      open_flags = SND_SEQ_OPEN_INPUT;
      port_flags = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
      break;
    }
  case io_write: 
    {
      open_flags = SND_SEQ_OPEN_OUTPUT;
      port_flags = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
      break;
    }
  case io_readwrite: 
    {
      open_flags = SND_SEQ_OPEN_DUPLEX;
      port_flags = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE |
                   SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
      break;
    }
  }
  
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(midiio-aseq) Opening ALSA sequencer");
  int err = snd_seq_open(&seq_handle_rep, "default", open_flags, SND_SEQ_NONBLOCK);
  if (err < 0) {
    toggle_open_state(false);
  }
  else {
    toggle_open_state(true);
  }

  // Set client name.
  snd_seq_set_client_name(seq_handle_rep, "ecasound");
  // Create a simple port
  port_rep = snd_seq_create_simple_port( seq_handle_rep, "ecasound",  
                                         port_flags, 
                                         SND_SEQ_PORT_TYPE_MIDI_GENERIC);
  // Parse the device name, and connect it to the port when successful
  snd_seq_addr_t subs;
  err = snd_seq_parse_address(seq_handle_rep, &subs, device_name_rep.c_str());
  if( err == 0) {
    switch(io_mode()) {
    case io_read:
      snd_seq_connect_to(seq_handle_rep, port_rep, subs.client, subs.port);
      break;
    case io_write: 
      snd_seq_connect_from(seq_handle_rep, port_rep, subs.client, subs.port);
      break;
    case io_readwrite: 
      snd_seq_connect_to(seq_handle_rep, port_rep, subs.client, subs.port);
      snd_seq_connect_from(seq_handle_rep, port_rep, subs.client, subs.port);
      break;
    }
  }
  // Create the encoder/decoder instance
  err = snd_midi_event_new( buffer_size_rep = 16, &coder_rep );
  // ...
  finished_rep = false;
}

void MIDI_IO_ASEQ::close(void) {
  // Release the xxcoder instance
  snd_midi_event_free( coder_rep );
  // Delete the port
  snd_seq_delete_port( seq_handle_rep, port_rep );
  // Close the sequencer client
  snd_seq_close( seq_handle_rep );
  toggle_open_state(false);
}

bool MIDI_IO_ASEQ::finished(void) const { return(finished_rep); }

long int MIDI_IO_ASEQ::read_bytes(void* target_buffer, long int bytes) {
  snd_seq_event_t *event;
  int err = 0, position = 0;
  if ( bytes > buffer_size_rep ) {
     snd_midi_event_resize_buffer ( coder_rep, bytes );	
     buffer_size_rep = bytes;
  }
  while (true) {
    if (snd_seq_event_input_pending(seq_handle_rep, 1) == 0) {
    	return position;
    }
    err = snd_seq_event_input(seq_handle_rep, &event);
    if (err < 0) {
    	break;
    }
    if ( event->type == SND_SEQ_EVENT_CONTROLLER ||
         event->type == SND_SEQ_EVENT_CONTROL14 ||
         event->type == SND_SEQ_EVENT_NONREGPARAM ||
         event->type == SND_SEQ_EVENT_REGPARAM ||
         event->type == SND_SEQ_EVENT_SYSEX ) {
      err = snd_midi_event_decode( coder_rep, 
                                   ((unsigned char *)target_buffer) + position, 
                                   bytes - position, 
                                   event );
      if (err < 0) {
      	break;
      }
      position += err;
      if ( position >= bytes) return position;
    }
  }
  finished_rep = true;
  return err;
}

long int MIDI_IO_ASEQ::write_bytes(void* target_buffer, long int bytes) {
  snd_seq_event_t ev;
  int err = 0;
  if ( bytes > buffer_size_rep ) {
     snd_midi_event_resize_buffer ( coder_rep, bytes );	
     buffer_size_rep = bytes;
  }
  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_source(&ev, port_rep);
  snd_seq_ev_set_subs(&ev);
  snd_seq_ev_set_direct(&ev);
  err = snd_midi_event_encode( coder_rep, 
                    	       (unsigned char *)target_buffer, 
                               bytes, &ev );
  if (err == bytes) {  
     snd_seq_event_output(seq_handle_rep, &ev);
     snd_seq_drain_output(seq_handle_rep);
     return err;
  }
  finished_rep = true;
  return err;
}

void MIDI_IO_ASEQ::set_parameter(int param, 
				std::string value) {
  switch (param) {
  case 1: 
    label(value);
    break;

  case 2: 
    device_name_rep = value;
    break;
  }
}

std::string MIDI_IO_ASEQ::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(device_name_rep);
  }
  return("");
}

bool MIDI_IO_ASEQ::pending_messages(unsigned long timeout) const { 
  struct pollfd *pfds;
  int result = 0;
  int npfds = snd_seq_poll_descriptors_count(seq_handle_rep, POLLIN);
  pfds = (struct pollfd *)alloca(sizeof(*pfds) * npfds);
  snd_seq_poll_descriptors(seq_handle_rep, pfds, npfds, POLLIN);
  result = poll(pfds, npfds, timeout);
  return (result > 0);
}
