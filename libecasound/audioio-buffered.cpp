// ------------------------------------------------------------------------
// audioio-buffered.cpp: A lower level interface for audio I/O objects
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <cmath> /* ceil() */
#include <kvutils/dbc.h>

#include "samplebuffer.h"
#include "audioio-buffered.h"

AUDIO_IO_BUFFERED::AUDIO_IO_BUFFERED(void) 
  : buffersize_rep(0),
    iobuf_uchar_repp(0),
    iobuf_size_rep(0)
{
}

AUDIO_IO_BUFFERED::~AUDIO_IO_BUFFERED(void)
{ 
  if (iobuf_uchar_repp != 0) { 
    delete[] iobuf_uchar_repp; 
    iobuf_uchar_repp = 0;
    iobuf_size_rep = 0;
  }
}

void AUDIO_IO_BUFFERED::reserve_buffer_space(long int bytes)
{
  if (bytes > static_cast<long int>(iobuf_size_rep)) {
    if (iobuf_uchar_repp != 0) {
      delete[] iobuf_uchar_repp;
      iobuf_uchar_repp = 0;
    }
    // std::cerr << "Reserving " << bytes << " bytes (" << label() << ").\n";
    iobuf_uchar_repp = new unsigned char [bytes];
    iobuf_size_rep = bytes;
  }
}

void AUDIO_IO_BUFFERED::set_buffersize(long int samples)
{
  if (buffersize_rep != samples ||
      static_cast<long int>(iobuf_size_rep) < buffersize_rep * frame_size()) {
    buffersize_rep = samples;
    reserve_buffer_space(buffersize_rep * frame_size());
  }
}

void AUDIO_IO_BUFFERED::read_buffer(SAMPLE_BUFFER* sbuf)
{
  // --------
  DBC_REQUIRE(iobuf_uchar_repp != 0);
  DBC_REQUIRE(static_cast<long int>(iobuf_size_rep) >= buffersize_rep * frame_size());
  // --------

  set_buffersize(sbuf->length_in_samples());

  if (interleaved_channels() == true) {
    sbuf->import_interleaved(iobuf_uchar_repp,
			     read_samples(iobuf_uchar_repp, buffersize_rep),
			     sample_format(),
			     channels());
  }
  else {
    sbuf->import_noninterleaved(iobuf_uchar_repp,
				read_samples(iobuf_uchar_repp, buffersize_rep),
				sample_format(),
				channels());
  }
  position_in_samples_advance(sbuf->length_in_samples());
}

void AUDIO_IO_BUFFERED::write_buffer(SAMPLE_BUFFER* sbuf)
{
  // --------
  DBC_REQUIRE(iobuf_uchar_repp != 0);
  DBC_REQUIRE(static_cast<long int>(iobuf_size_rep) >= buffersize_rep * frame_size());
  // --------

  set_buffersize(sbuf->length_in_samples());

  if (interleaved_channels() == true) {
    sbuf->export_interleaved(iobuf_uchar_repp,
			     sample_format(),
			     channels());
  }
  else {
    sbuf->export_noninterleaved(iobuf_uchar_repp,
				sample_format(),
				channels());
  }

  write_samples(iobuf_uchar_repp, sbuf->length_in_samples());
  position_in_samples_advance(sbuf->length_in_samples());
  extend_position();
}
