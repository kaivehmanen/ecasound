// ------------------------------------------------------------------------
// audioio-buffered.cpp: A lower level interface for audio I/O objects
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
    buffersize_sig_rep(0),
    target_srate_rep(0),
    target_samples_rep(0),
    iobuf_uchar_repp(0),
    iobuf_size_rep(0) { }


AUDIO_IO_BUFFERED::~AUDIO_IO_BUFFERED(void) { 
  if (iobuf_uchar_repp != 0) { 
    delete[] iobuf_uchar_repp; 
    iobuf_uchar_repp = 0;
    iobuf_size_rep = 0;
  }
}

void AUDIO_IO_BUFFERED::reserve_buffer_space(long int bytes) {
  if (bytes > static_cast<long int>(iobuf_size_rep)) {
    if (iobuf_uchar_repp != 0) {
      delete[] iobuf_uchar_repp;
      iobuf_uchar_repp = 0;
    }
    // cerr << "Reserving " << bytes << " bytes (" << label() << ").\n";
    iobuf_uchar_repp = new unsigned char [bytes];
    iobuf_size_rep = bytes;
  }
}

void AUDIO_IO_BUFFERED::buffersize(long int samples, 
				   long int sample_rate) {
  //  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-types/buffered) " +
  //  		label() + ": " +
  //  		"Setting public buffer size to [" +
  //  		kvu_numtostr(samples) + "," +
  //  		kvu_numtostr(sample_rate) + "]. Internal srate is " +
  //  		kvu_numtostr(samples_per_second()) + " and buffersize " +
  //  		kvu_numtostr(buffersize_rep) + ".");
  
  if (sample_rate != 0) {
    target_srate_rep = sample_rate;
  }

  if (samples != 0) {
    target_samples_rep = samples;

    if (target_srate_rep != samples_per_second()) {
      buffersize_rep = static_cast<long int>(ceil(static_cast<double>(target_samples_rep) *
						  samples_per_second() / target_srate_rep));
      
    }
    else {
      buffersize_rep = target_samples_rep;
    }

    buffersize_sig_rep = target_samples_rep * target_srate_rep;
    reserve_buffer_space(buffersize_rep * frame_size());
  }
  else {
    buffersize_rep = 0;
    buffersize_sig_rep = 0;
  }

  //    ecadebug->msg(ECA_DEBUG::user_objects, 
  //  		"(audioio-types/buffered) " +
  //  		label() + ": " +
  //  		"Set internal object buffer size to " +
  //  		kvu_numtostr(buffersize_rep) + ".");
}

void AUDIO_IO_BUFFERED::read_buffer(SAMPLE_BUFFER* sbuf) {
  // --------
  DBC_REQUIRE(iobuf_uchar_repp != 0);
  DBC_REQUIRE(static_cast<long int>(iobuf_size_rep) >= buffersize_rep * frame_size());
  // --------

  if (sbuf->sample_rate() != samples_per_second()) {
    /* FIXME: resample_init_memory() may end up allocating memory! */
    sbuf->resample_init_memory(samples_per_second(), sbuf->sample_rate());
  }
  if (interleaved_channels() == true) {
    sbuf->copy_to_buffer(iobuf_uchar_repp,
			 read_samples(iobuf_uchar_repp, buffersize_rep),
			 sample_format(),
			 channels(),
			 samples_per_second());
  }
  else {
    sbuf->copy_to_buffer_vector(iobuf_uchar_repp,
			 read_samples(iobuf_uchar_repp, buffersize_rep),
			 sample_format(),
			 channels(),
			 samples_per_second());
  }
  position_in_samples_advance(sbuf->length_in_samples());
}

void AUDIO_IO_BUFFERED::write_buffer(SAMPLE_BUFFER* sbuf) {
  // --------
  DBC_REQUIRE(iobuf_uchar_repp != 0);
  DBC_REQUIRE(static_cast<long int>(iobuf_size_rep) >= buffersize_rep * frame_size());
  // --------
  if (buffersize_sig_rep != sbuf->length_in_samples() * sbuf->sample_rate()) {
    //  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-types/buffered) buffersize() doesn't match - correcting");
    buffersize(sbuf->length_in_samples(), sbuf->sample_rate());
  }
  if (sbuf->sample_rate() != samples_per_second()) {
    /* FIXME: resample_init_memory() may end up allocating memory! */
    sbuf->resample_init_memory(sbuf->sample_rate(), samples_per_second());
  }
  if (interleaved_channels() == true) {
    sbuf->copy_from_buffer(iobuf_uchar_repp,
			   sample_format(),
			   channels(),
			   samples_per_second());
  }
  else {
    sbuf->copy_from_buffer_vector(iobuf_uchar_repp,
				  sample_format(),
				  channels(),
				  samples_per_second());
  }
  write_samples(iobuf_uchar_repp, sbuf->length_in_samples());
  position_in_samples_advance(sbuf->length_in_samples());
  extend_position();
}

