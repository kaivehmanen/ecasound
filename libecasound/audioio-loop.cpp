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
#include "audioio-types.h"
#include "audioio-loop.h"

#include "eca-error.h"
#include "eca-debug.h"

LOOP_DEVICE::LOOP_DEVICE(int id) 
  :  AUDIO_IO("loop," + kvu_numtostr(id), io_readwrite),
     id_rep(id) { 
  registered_inputs_rep = 0;
  writes_rep = 0;
  registered_outputs_rep = 0;
  filled_rep = false;
}

void LOOP_DEVICE::read_buffer(SAMPLE_BUFFER* buffer) {
  if (filled_rep == false) buffer->make_silent();
  else *buffer = sbuf;
}

void LOOP_DEVICE::write_buffer(SAMPLE_BUFFER* buffer) {
  ++writes_rep;
  if (writes_rep == 1) {
    sbuf.number_of_channels(buffer->number_of_channels());
    sbuf.make_silent();
  }
  sbuf.add_with_weight(*buffer, registered_outputs_rep);
  if (writes_rep == registered_outputs_rep) writes_rep = 0;
  filled_rep = true;
}
