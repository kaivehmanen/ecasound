// ------------------------------------------------------------------------
// audioio-types.cpp: Top-level classes for audio-I/O.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <cmath>
#include <string>

#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio-types.h"

#include "eca-debug.h"

AUDIO_IO_BUFFERED::AUDIO_IO_BUFFERED(void) 
  : buffersize_rep(0),
    target_srate_rep(0),
    target_samples_rep(0),
    iobuf_uchar(0),
    iobuf_size(0) { }


AUDIO_IO_BUFFERED::~AUDIO_IO_BUFFERED(void) { 
  if (iobuf_uchar != 0) { 
    delete[] iobuf_uchar; 
    iobuf_uchar = 0;
    iobuf_size = 0;
  }
}

void AUDIO_IO_BUFFERED::reserve_buffer_space(long int bytes) {
  if (bytes > static_cast<long int>(iobuf_size)) {
    if (iobuf_uchar != 0) {
      delete[] iobuf_uchar;
      iobuf_uchar = 0;
    }
    //    cerr << "Reserving " << bytes << " bytes (" << label() << ").\n";
    iobuf_uchar = new unsigned char [bytes];
    iobuf_size = bytes;
  }
}

void AUDIO_IO_BUFFERED::buffersize(long int samples, 
				   long int sample_rate) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-types/buffered) Setting buffer size [" +
		   kvu_numtostr(samples) + "," +
		   kvu_numtostr(sample_rate) + "].");

  long int old_bsize = buffersize_rep;
  if (sample_rate != 0) {
    target_srate_rep = sample_rate;
  }

  if (samples != 0) {
    target_samples_rep = samples;
  }

  if (target_srate_rep != samples_per_second()) {
    buffersize_rep = static_cast<long int>(ceil(static_cast<double>(target_samples_rep) *
			  samples_per_second() / target_srate_rep));
  }
  else {
    buffersize_rep = target_samples_rep;
  }

  reserve_buffer_space(buffersize_rep * frame_size());

  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-types/buffered) Set buffer size [" +
		   kvu_numtostr(buffersize_rep) + "].");

}

void AUDIO_IO_BUFFERED::read_buffer(SAMPLE_BUFFER* sbuf) {
  // --------
  // require:
  assert(iobuf_uchar != 0);
  assert(static_cast<long int>(iobuf_size) >= buffersize_rep * frame_size());
  // --------
  sbuf->copy_to_buffer(iobuf_uchar,
		       read_samples(iobuf_uchar, buffersize_rep),
		       sample_format(),
		       channels(),
		       samples_per_second());
  position_in_samples_advance(sbuf->length_in_samples());
}

void AUDIO_IO_BUFFERED::write_buffer(SAMPLE_BUFFER* sbuf) {
  // --------
  // require:
  assert(iobuf_uchar != 0);
  assert(static_cast<long int>(iobuf_size) >= buffersize_rep * frame_size());
  // --------
  if (buffersize_rep != sbuf->length_in_samples()) buffersize(sbuf->length_in_samples(), samples_per_second());
  sbuf->copy_from_buffer(iobuf_uchar,
			 sample_format(),
			 channels(),
			 samples_per_second());
  write_samples(iobuf_uchar, sbuf->length_in_samples());
  position_in_samples_advance(sbuf->length_in_samples());
  extend_position();
}

string AUDIO_IO_DEVICE::status(void) const {
  MESSAGE_ITEM mitem;

  mitem << "realtime-device, processed ";
  mitem << position_in_samples() << " samples.";

  return(mitem.to_string());
}
