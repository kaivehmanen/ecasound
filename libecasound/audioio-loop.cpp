// ------------------------------------------------------------------------
// audioio-loop.cpp: Audio object that routes data between reads and writes
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

#include <string>

#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>

#include "samplebuffer.h"
#include "sample-specs.h"
#include "audioio-buffered.h"
#include "audioio-loop.h"

#include "eca-error.h"
#include "eca-debug.h"

using std::string;

LOOP_DEVICE::LOOP_DEVICE(int id) 
  :  AUDIO_IO("loop," + kvu_numtostr(id), io_readwrite),
     id_rep(id),
     sbuf(buffersize(), 0)
{ 
  writes_rep = 0;
  registered_inputs_rep = 0;
  registered_outputs_rep = 0;
  filled_rep = false;
  finished_rep = false;
  empty_rounds_rep = 0;
}

bool LOOP_DEVICE::finished(void) const
{
  return(finished_rep);
}

void LOOP_DEVICE::read_buffer(SAMPLE_BUFFER* buffer)
{
  buffer->number_of_channels(channels());
  if (empty_rounds_rep == 0) {
    if (filled_rep == true) {
      buffer->copy(sbuf);
      DBC_CHECK(writes_rep == registered_outputs_rep);
      writes_rep = 0;
    }
    else {
      buffer->make_silent();
    }
  }
  else {
    finished_rep = true;
    buffer->make_silent();
  }

  DBC_CHECK(buffer->number_of_channels() == channels());
}

void LOOP_DEVICE::write_buffer(SAMPLE_BUFFER* buffer)
{
  ++writes_rep;

  /* first write after an read (or reset) */
  if (writes_rep == 1) {
    position_in_samples_advance(buffer->length_in_samples());
    extend_position();
    sbuf.number_of_channels(channels());
    sbuf.make_silent();
  }

  /* store data from 'buffer' */
  if (buffer->length_in_samples() > 0) {
    empty_rounds_rep = 0;
    sbuf.add_with_weight(*buffer, registered_outputs_rep);
    filled_rep = true;

    if (writes_rep > registered_outputs_rep) {
      ecadebug->msg(ECA_DEBUG::info, 
		    "(audioio-loop) Warning! Multiple writes without reads!");
    }
  }
  /* empty 'buffer' */
  else {
    ++empty_rounds_rep;
  }

  DBC_CHECK(sbuf.number_of_channels() == channels());
}

void LOOP_DEVICE::set_parameter(int param, 
				string value)
{
  switch (param) {
  case 1: 
    set_label(value);
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
