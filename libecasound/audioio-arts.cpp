// ------------------------------------------------------------------------
// audioio-arts.cpp: Interface for communicating with aRts/MCOP.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef COMPILE_ARTS

#include "audioio-types.h"
#include "audioio-arts.h"
#include "eca-error.h"
#include "eca-debug.h"

ARTS_INTERFACE::ARTS_INTERFACE(const string& name) 
{
  label(name);
}

void ARTS_INTERFACE::open(void) throw(ECA_ERROR*)
{
  if (is_open() == true) return;

  int err = ::arts_init();
  if (err < 0) {
    throw(new ECA_ERROR("AUDIOIO-ARTS", "unable to connect to aRts
    server: " + string(arts_error_text(err))));
  }

  if (io_mode() == io_read) {
    stream_rep = ::arts_record_stream(samples_per_second(), channels(), bits(), "ecasound-input");
  }
  else if (io_mode() == io_write) {
    stream_rep = ::arts_play_stream(samples_per_second(), channels(), bits(), "ecasound-output");
  }
  else {
      throw(new ECA_ERROR("AUDIOIO-ARTS", "Simultanious intput/output not supported."));
  }

  ::arts_stream_set(stream_rep, ARTS_P_BUFFER_SIZE, buffersize() * frame_size());
  ::arts_stream_set(stream_rep, ARTS_P_BLOCKING, 1);
  toggle_open_state(true);
}

void ARTS_INTERFACE::stop(void) { }
void ARTS_INTERFACE::close(void)
{
  toggle_open_state(false);
  ::arts_close_stream(stream_rep);
}

void ARTS_INTERFACE::start(void) { }
long ARTS_INTERFACE::position_in_samples(void) const { return(0); }
long int ARTS_INTERFACE::read_samples(void* target_buffer, 
				      long int samples) 
{
  return(::arts_read(stream_rep, target_buffer, frame_size() * samples) / frame_size());
}

void ARTS_INTERFACE::write_samples(void* target_buffer, 
				   long int samples) 
{
  ::arts_write(stream_rep, target_buffer, frame_size() * samples);
}

ARTS_INTERFACE::~ARTS_INTERFACE(void) { ::arts_free(); }

#endif // COMPILE_ARTS

