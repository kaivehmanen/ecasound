// ------------------------------------------------------------------------
// audioio-loop.cpp: Audio object that routes data between reads and writes
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

#include <string>

#include <kvutils/kvu_numtostr.h>

#include "samplebuffer.h"
#include "sample-specs.h"
#include "audioio-types.h"
#include "audioio-loop.h"

#include "eca-error.h"
#include "eca-debug.h"

LOOP_DEVICE::LOOP_DEVICE(int id) 
  :  AUDIO_IO("loop," + kvu_numtostr(id), io_readwrite),
     id_rep(id),
     sbuf(buffersize(),
	  SAMPLE_SPECS::channel_count_default,
	  SAMPLE_SPECS::sample_rate_default)
{ 
  writes_rep = 0;
  registered_inputs_rep = 0;
  registered_outputs_rep = 0;
  filled_rep = false;
  finished_rep = false;
  writes_finished_rep = false;
}

bool LOOP_DEVICE::finished(void) const {
  return(finished_rep);
}

void LOOP_DEVICE::read_buffer(SAMPLE_BUFFER* buffer) {
  if (writes_finished_rep != true) {
    if (filled_rep == false) buffer->make_silent();
    else *buffer = sbuf;
  }
  else {
    finished_rep = true;
    buffer->make_silent();
  }
}

void LOOP_DEVICE::write_buffer(SAMPLE_BUFFER* buffer) {
  ++writes_rep;
  if (writes_rep == 1) {
    sbuf.number_of_channels(buffer->number_of_channels());
    sbuf.make_silent();
  }
  if (writes_finished_rep == true ||
      buffer->length_in_samples() > 0) {
    writes_finished_rep = finished_rep = false;
    sbuf.add_with_weight(*buffer, registered_outputs_rep);
    filled_rep = true;
  }
  else {
    writes_finished_rep = true;
  }
  if (writes_rep == registered_outputs_rep) writes_rep = 0;
}

void LOOP_DEVICE::set_parameter(int param, 
				string value) {
  switch (param) {
  case 1: 
    label(value);
    break;

  case 2: 
    id_rep = atoi(value.c_str());
    break;
  }
}

string LOOP_DEVICE::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(kvu_numtostr(id_rep));
  }
  return("");
}
