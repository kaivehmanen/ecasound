// ------------------------------------------------------------------------
// samplebuffer.cpp: Class representing a buffer of audio samples.
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

#include <iostream>
#include <vector>
#include <cmath>

#include <sys/types.h>

#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>

#include "samplebuffer.h"
#include "samplebuffer_impl.h"
#include "eca-debug.h"

/* Debug resampling operations */ 
//  #define DEBUG_RESAMPLING

#ifdef DEBUG_RESAMPLING
#define DEBUG_RESAMPLING_STATEMENT(x) (x)
#else
#define DEBUG_RESAMPLING_STATEMENT(x) ((void)0)
#endif

using std::vector;

/**
 * Constructs a new sample buffer object.
 */
SAMPLE_BUFFER::SAMPLE_BUFFER (buf_size_t buffersize, channel_size_t channels, srate_size_t sample_rate) 
  : channel_count_rep(channels),
    buffersize_rep(buffersize),
    sample_rate_rep(sample_rate),
    reserved_samples_rep(buffersize) 
{

  impl_repp = new SAMPLE_BUFFER_impl;

  buffer.resize(channels);
  for(size_t n = 0; n < buffer.size(); n++) {
    buffer[n] = new sample_t [reserved_samples_rep];
    for(buf_size_t m = 0; m < reserved_samples_rep; m++) {
      buffer[n][m] = SAMPLE_SPECS::silent_value;
    }
  }
  impl_repp->old_buffer_repp = 0;
 
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer) Buffer created, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}

/**
 * Constructs a new sample buffer object from a reference
 * to an already existing object.
 *
 * For better performance, doesn't copy IO-buffers nor
 * iterator state.
 */
SAMPLE_BUFFER::SAMPLE_BUFFER (const SAMPLE_BUFFER& x)
  : channel_count_rep(x.channel_count_rep),
    buffersize_rep(x.buffersize_rep),
    sample_rate_rep(x.sample_rate_rep),
    reserved_samples_rep(x.reserved_samples_rep) {

  impl_repp = new SAMPLE_BUFFER_impl;

  buffer.resize(x.buffer.size());

  for(size_t n = 0; n < buffer.size(); n++) {
    buffer[n] = new sample_t [reserved_samples_rep];
    memcpy(buffer[n], x.buffer[n], buffersize_rep * sizeof(sample_t));
  }

  impl_repp->old_buffer_repp = 0;

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer) Buffer copy-constructed, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}

/**
 * Assignment operator.
 *
 * For better performance, doesn't copy IO-buffers nor
 * iterator state.
 */
SAMPLE_BUFFER& SAMPLE_BUFFER::operator=(const SAMPLE_BUFFER& x) {
 
  if (this != &x) {

    impl_repp->resample_memory_rep = x.impl_repp->resample_memory_rep;
    
    if (x.buffersize_rep > reserved_samples_rep ||
	x.buffer.size() != buffer.size()) {

      reserved_samples_rep = x.buffersize_rep;

      for(size_t n = 0; n < buffer.size(); n++) delete[] buffer[n];

      buffer.resize(x.buffer.size());

      for(size_t n = 0; n < buffer.size(); n++) {
	buffer[n] = new sample_t [reserved_samples_rep];
	for(buf_size_t m = 0; m < reserved_samples_rep; m++) {
	  buffer[n][m] = SAMPLE_SPECS::silent_value;
	}
      }
    }
    
    buffersize_rep = x.buffersize_rep;
    channel_count_rep = x.channel_count_rep;
    sample_rate_rep = x.sample_rate_rep;

    for(size_t n = 0; n < buffer.size(); n++) {
      memcpy(buffer[n], x.buffer[n], buffersize_rep * sizeof(sample_t));
    }
  }

  return *this;
}

/**
 * Destructor.
 */
SAMPLE_BUFFER::~SAMPLE_BUFFER (void) { 
  for(size_t n = 0; n < buffer.size(); n++) {
    delete[] buffer[n];
  }

  if (impl_repp->old_buffer_repp != 0) {
    delete[] impl_repp->old_buffer_repp;
    impl_repp->old_buffer_repp = 0;
  }

  delete impl_repp;
}

/** 
 * Sets the number of audio channels.
 */
void SAMPLE_BUFFER::number_of_channels(channel_size_t len) 
{
  /*  std::cerr << "(samplebuffer_impl) ch-count changes from " << channel_count_rep << " to " << len << ".\n"; */
  if (len > static_cast<channel_size_t>(buffer.size())) {
    size_t old_size = buffer.size();
    buffer.resize(len);
    for(channel_size_t n = old_size; n < len; n++) {
      buffer[n] = new sample_t [reserved_samples_rep];
    }
    ecadebug->msg(ECA_DEBUG::system_objects, "(samplebuffer<>) Increasing channel-count (1).");    
  }
  if (len > channel_count_rep) {
    for(channel_size_t n = channel_count_rep; n < len; n++) {
      for(buf_size_t m = 0; m < reserved_samples_rep; m++) {
	buffer[n][m] = SAMPLE_SPECS::silent_value;
      }
    }
    ecadebug->msg(ECA_DEBUG::system_objects, "(samplebuffer<>) Increasing channel-count (2).");
  }
  channel_count_rep = len;
}

/**
 * Resizes the buffer to fit 'buffersize' samples. Doesn't
 * change channel count.
 */
void SAMPLE_BUFFER::resize(buf_size_t buffersize) {
  if (buffersize > reserved_samples_rep) {
    reserved_samples_rep = buffersize;
    for(size_t n = 0; n < buffer.size(); n++) {
      delete[] buffer[n];
      buffer[n] = new sample_t [reserved_samples_rep];
    }

    if (impl_repp->old_buffer_repp != 0) {
      delete[] impl_repp->old_buffer_repp;
      impl_repp->old_buffer_repp = 0;
    }
  }

  if (buffersize != buffersize_rep) {
    for(size_t n = 0; n < buffer.size(); n++) {
      for(buf_size_t m = 0; m < buffersize; m++) {
	buffer[n][m] = SAMPLE_SPECS::silent_value;
      }
    }

    buffersize_rep = buffersize;
  }
}


/**
 * Mutes the whole buffer.
 */
void SAMPLE_BUFFER::make_silent(void) {
  for(channel_size_t n = 0; n < channel_count_rep; n++) {
    for(buf_size_t s = 0; s < buffersize_rep; s++) {
      buffer[n][s] = SAMPLE_SPECS::silent_value;
    }
  }
}

/**
 * Mute a range of samples.
 * 
 * @pre start_pos >= 0
 * @pre end_pos >= 0
 */
void SAMPLE_BUFFER::make_silent_range(buf_size_t start_pos,
				      buf_size_t end_pos) {
  // --
  DBC_REQUIRE(start_pos >= 0);
  DBC_REQUIRE(end_pos >= 0);
  // --

  for(channel_size_t n = 0; n < channel_count_rep; n++) {
    for(buf_size_t s = start_pos; s < end_pos && s < buffersize_rep; s++) {
      buffer[n][s] = SAMPLE_SPECS::silent_value;
    }
  }
}

/**
 * Limits all samples to valid values. 
 */
void SAMPLE_BUFFER::limit_values(void) {
  for(channel_size_t n = 0; n < channel_count_rep; n++) {
    for(buf_size_t m = 0; m < buffersize_rep; m++) {
      if (buffer[n][m] > SAMPLE_SPECS::impl_max_value) 
	buffer[n][m] = SAMPLE_SPECS::impl_max_value;
      else if (buffer[n][m] < SAMPLE_SPECS::impl_min_value) 
	buffer[n][m] = SAMPLE_SPECS::impl_min_value;
    }
  }
}

/**
 * Divides all samples by 'dvalue'.
 */
void SAMPLE_BUFFER::divide_by(SAMPLE_BUFFER::sample_t dvalue) {
  for(channel_size_t n = 0; n < channel_count_rep; n++) {
    for(buf_size_t m = 0; m < buffersize_rep; m++) {
      buffer[n][m] /= dvalue;
    }
  }
}

/**
 * Channel-wise addition. Buffer length is increased if necessary.
 *
 * @post length_in_samples() >= x.length_in_samples()
 */
void SAMPLE_BUFFER::add(const SAMPLE_BUFFER& x) {
  if (x.length_in_samples() > length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int min_c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(channel_size_t q = 0; q < min_c_count; q++) {
    for(buf_size_t t = 0; t < x.length_in_samples(); t++) {
      buffer[q][t] += x.buffer[q][t];
    }
  }
}

/**
 * Channel-wise, weighted addition. Before addition every sample is 
 * multiplied by '1/weight'. Buffer length is increased if necessary.
 *
 * @pre weight != 0
 * @post length_in_samples() >= x.length_in_samples()
 */
void SAMPLE_BUFFER::add_with_weight(const SAMPLE_BUFFER& x, int weight) {
  // ---
  DBC_REQUIRE(weight != 0);
  // ---

  if (x.length_in_samples() > length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int min_c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(channel_size_t q = 0; q < min_c_count; q++) {
    for(buf_size_t t = 0; t < x.length_in_samples(); t++) {
      buffer[q][t] += (x.buffer[q][t] / weight);
    }
  }
}

/**
 * Channel-wise copy. Buffer length is adjusted if necessary.
 *
 * @post length_in_samples() == x.length_in_samples()
 */
void SAMPLE_BUFFER::copy(const SAMPLE_BUFFER& x) {
  length_in_samples(x.length_in_samples());
  
  int min_c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(channel_size_t q = 0; q < min_c_count; q++) {
    for(buf_size_t t = 0; t < length_in_samples(); t++) {
      buffer[q][t] = x.buffer[q][t];
    }
  }
}

/**
 * Ranged channel-wise copy. Copies samples in range 
 * 'start_pos' - 'end_pos' from buffer 'x' to current 
 * buffer position 'to_pos'. 
 *
 * @pre start_pos <= end_pos
 * @pre to_pos < length_in_samples()
 */
void SAMPLE_BUFFER::copy_range(const SAMPLE_BUFFER& x, 
			       buf_size_t start_pos,
			       buf_size_t end_pos,
			       buf_size_t to_pos) 
{
  // ---
  DBC_REQUIRE(start_pos <= end_pos);
  DBC_REQUIRE(to_pos < length_in_samples());
  // ---

  int min_c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;

  if (start_pos >= x.length_in_samples()) start_pos = x.length_in_samples();
  if (end_pos >= x.length_in_samples()) end_pos = x.length_in_samples();

  buf_size_t to_index = to_pos;

  for(channel_size_t q = 0; q < min_c_count; q++) {
    for(buf_size_t s = start_pos; 
	  s < end_pos && 
	  s < x.length_in_samples() &&
	  to_index < length_in_samples() &&
	  to_index < x.length_in_samples();
	s++) {
      buffer[q][to_index] = x.buffer[q][s];
      ++to_index;
    }
  }
}

/**
 * Copy contents of sample buffer to 'target'. Sample data 
 * will be converted according to the given arguments
 * (sample rate, sample format and endianess).
 *
 * Note! If chcount > number_of_channels(), empty
 *       channels will be automatically added.
 *
 * @pre target != 0
 * @pre chcount > 0
 * @pre srate > 0
 * @ensure number_of_channels() >= chcount
 */
void SAMPLE_BUFFER::copy_from_buffer(unsigned char* target,
				     ECA_AUDIO_FORMAT::Sample_format fmt,
				     channel_size_t chcount,
				     srate_size_t srate) 
{
  // --------
  DBC_REQUIRE(target != 0);
  DBC_REQUIRE(chcount > 0);
  DBC_REQUIRE(srate > 0);
  // --------

  if (chcount > channel_count_rep) number_of_channels(chcount);
  if (srate != sample_rate_rep) resample_to(srate);

  buf_size_t osize = 0;
  for(buf_size_t isize = 0; isize < buffersize_rep; isize++) {
    for(channel_size_t c = 0; c < chcount; c++) {
      sample_t stemp = buffer[c][isize];
      if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
      else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
      
      switch (fmt) {
      case ECA_AUDIO_FORMAT::sfmt_u8:
	{
	  target[osize++] = (unsigned
			     char)((sample_t)(stemp / SAMPLE_SPECS::u8_to_st_constant) + SAMPLE_SPECS::u8_to_st_delta);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s16_le:
	{
	  int16_t s16temp;
	  if (stemp < 0) 
	    s16temp = (int16_t)(sample_t)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_t)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
  
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	
	  target[osize++] = (unsigned char)(s16temp & 0xff);
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s16_be:
	{
	  int16_t s16temp;
	  if (stemp < 0) 
	    s16temp = (int16_t)(sample_t)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_t)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
	
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	  // ---
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s16temp & 0xff);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s24_le:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = 0;
	
	  if (s32temp < 0) target[osize - 2] |=  0x80;
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s24_be:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

	  target[osize++] = 0;
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	
	  if (s32temp < 0) target[osize - 3] |= 0x80;
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s32_le:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
	
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s32_be:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
  
	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  break;
	}
   
      case ECA_AUDIO_FORMAT::sfmt_f32_le:
	{
	  union { int32_t i; float f; } f32temp;
	  f32temp.f = (float)stemp;
	  target[osize++] = (unsigned char)(f32temp.i & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 8) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 16) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 24) & 0xff);
	  break;
	}

      case ECA_AUDIO_FORMAT::sfmt_f32_be:
	{
	  union { int32_t i; float f; } f32temp;
	  f32temp.f = (float)stemp;
	  target[osize++] = (unsigned char)((f32temp.i >> 24) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 16) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 8) & 0xff);
	  target[osize++] = (unsigned char)(f32temp.i & 0xff);
	  break;
	}
      
      default: 
	{ 
	  ecadebug->msg(ECA_DEBUG::info, "(samplebuffer) Unknown sample format! [1].");
   	}
      }
    }
  }
  
  // -------
  DBC_ENSURE(number_of_channels() >= chcount);
  // -------
}

/**
 * Same as 'copy_from_buffer()', but 'target' data is 
 * written in non-interleaved format.
 *
 * Note! If chcount > number_of_channels(), empty
 *       channels will be automatically added.
 *
 * @pre target != 0
 * @pre chcount > 0
 * @pre srate > 0
 * @ensure number_of_channels() >= chcount
 */
void SAMPLE_BUFFER::copy_from_buffer_vector(unsigned char* target,
					    ECA_AUDIO_FORMAT::Sample_format fmt,
					    channel_size_t chcount,
					    srate_size_t srate) 
{
  // --------
  DBC_REQUIRE(target != 0);
  DBC_REQUIRE(chcount > 0);
  DBC_REQUIRE(srate > 0);
  // --------

  if (chcount > channel_count_rep) number_of_channels(chcount);
  if (srate != sample_rate_rep) resample_to(srate);

  buf_size_t osize = 0;
  for(channel_size_t c = 0; c < chcount; c++) {
    for(buf_size_t isize = 0; isize < buffersize_rep; isize++) {
      sample_t stemp = buffer[c][isize];
      if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
      else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
      
      switch (fmt) {
      case ECA_AUDIO_FORMAT::sfmt_u8:
	{
	  target[osize++] = (unsigned
			     char)((sample_t)(stemp / SAMPLE_SPECS::u8_to_st_constant) + SAMPLE_SPECS::u8_to_st_delta);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s16_le:
	{
	  int16_t s16temp;
	  if (stemp < 0) 
	    s16temp = (int16_t)(sample_t)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_t)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
	
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	
	  target[osize++] = (unsigned char)(s16temp & 0xff);
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s16_be:
	{
	  int16_t s16temp;
	  if (stemp < 0) 
	    s16temp = (int16_t)(sample_t)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_t)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
	
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	  // ---
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s16temp & 0xff);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s24_le:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = 0;
	
	  if (s32temp < 0) target[osize - 2] |=  0x80;

	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s24_be:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

	  target[osize++] = 0;
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	
	  if (s32temp < 0) target[osize - 3] |= 0x80;

	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s32_le:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
	
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s32_be:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_t)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_t)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
  
	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  break;
	}
   
      case ECA_AUDIO_FORMAT::sfmt_f32_le:
	{
	  union { int32_t i; float f; } f32temp;
	  f32temp.f = (float)stemp;
	  target[osize++] = (unsigned char)(f32temp.i & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 8) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 16) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 24) & 0xff);
	  break;
	}

      case ECA_AUDIO_FORMAT::sfmt_f32_be:
	{
	  union { int32_t i; float f; } f32temp;
	  f32temp.f = (float)stemp;
	  target[osize++] = (unsigned char)((f32temp.i >> 24) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 16) & 0xff);
	  target[osize++] = (unsigned char)((f32temp.i >> 8) & 0xff);
	  target[osize++] = (unsigned char)(f32temp.i & 0xff);
	  break;
	}
      
      default: 
	{ 
	  ecadebug->msg(ECA_DEBUG::info, "(samplebuffer) Unknown sample format! [2].");
	}
      }
    }
  }

  // -------
  DBC_ENSURE(number_of_channels() >= chcount);
  // -------
}

/**
 * Fill buffer from external buffer source. Sample data 
 * will be converted to internal sample format using the 
 * given arguments (sample rate, sample format and 
 * endianess).
 *
 * @pre source != 0
 * @pre srate > 0
 * @pre samples_read >= 0
 */
void SAMPLE_BUFFER::copy_to_buffer(unsigned char* source,
				   buf_size_t samples_read,
				   ECA_AUDIO_FORMAT::Sample_format fmt,
				   channel_size_t chcount,
				   srate_size_t srate) {
  // --------
  DBC_REQUIRE(source != 0);
  DBC_REQUIRE(srate > 0);
  DBC_REQUIRE(samples_read >= 0);
  // --------

  if (channel_count_rep != chcount) number_of_channels(chcount);
  if (buffersize_rep != samples_read) resize(samples_read);

  unsigned char a[2];
  unsigned char b[4];
  buf_size_t isize = 0;

  for(buf_size_t osize = 0; osize < buffersize_rep; osize++) {
    for(channel_size_t c = 0; c < chcount; c++) {
      switch (fmt) {
      case ECA_AUDIO_FORMAT::sfmt_u8: 
	{
	  buffer[c][osize] = (unsigned char)source[isize++];
	  buffer[c][osize] -= SAMPLE_SPECS::u8_to_st_delta;
	  buffer[c][osize] *= SAMPLE_SPECS::u8_to_st_constant;
	}
	break;
	
      case ECA_AUDIO_FORMAT::sfmt_s16_le:
	{
	  // little endian: (LSB, MSB) (Intel)
	  // big endian: (MSB, LSB) (Motorola)
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s16_be:
	{
	  if (!SAMPLE_SPECS::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s24_le:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s24_be:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s32_le:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s32_be:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_f32_le:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(float*)b);
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_f32_be:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(float*)b);
	}
	break;

      default: 
	{ 
	  ecadebug->msg(ECA_DEBUG::info, "(samplebuffer) Unknown sample format! [3].");
	}
      }
    }
  }
  if (srate != sample_rate_rep) resample_from(srate);
}

/**
 * Same as 'copy_to_buffer()', but 'source' data is 
 * assumed be in non-interleaved format.
 *
 * @pre source != 0
 * @pre srate > 0
 * @pre samples_read >= 0
 */
void SAMPLE_BUFFER::copy_to_buffer_vector(unsigned char* source,
					  buf_size_t samples_read,
					  ECA_AUDIO_FORMAT::Sample_format fmt,
					  channel_size_t chcount,
					  srate_size_t srate) {
  // --------
  DBC_REQUIRE(source != 0);
  DBC_REQUIRE(srate > 0);
  DBC_REQUIRE(samples_read >= 0);
  // --------

  if (channel_count_rep != chcount) number_of_channels(chcount);
  if (buffersize_rep != samples_read) resize(samples_read);

  unsigned char a[2];
  unsigned char b[4];

  buf_size_t isize = 0;
  for(channel_size_t c = 0; c < chcount; c++) {
    for(buf_size_t osize = 0; osize < buffersize_rep; osize++) {
      switch (fmt) {
      case ECA_AUDIO_FORMAT::sfmt_u8: 
	{
	  buffer[c][osize] = (unsigned char)source[isize++];
	  buffer[c][osize] -= SAMPLE_SPECS::u8_to_st_delta;
	  buffer[c][osize] *= SAMPLE_SPECS::u8_to_st_constant;
	}
	break;
	
      case ECA_AUDIO_FORMAT::sfmt_s16_le:
	{
	  // little endian: (LSB, MSB) (Intel)
	  // big endian: (MSB, LSB) (Motorola)
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s16_be:
	{
	  if (!SAMPLE_SPECS::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s24_le:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s24_be:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s32_le:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_s32_be:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_f32_le:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  else {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(float*)b);
	}
	break;

      case ECA_AUDIO_FORMAT::sfmt_f32_be:
	{
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    b[3] = source[isize++];
	    b[2] = source[isize++];
	    b[1] = source[isize++];
	    b[0] = source[isize++];
	  }
	  else {
	    b[0] = source[isize++];
	    b[1] = source[isize++];
	    b[2] = source[isize++];
	    b[3] = source[isize++];
	  }
	  buffer[c][osize] = (sample_t)(*(float*)b);
	}
	break;

      default: 
	{ 
	  ecadebug->msg(ECA_DEBUG::info, "(samplebuffer) Unknown sample format! [4].");
	}
      }
    }
  }
  if (srate != sample_rate_rep) resample_from(srate);
}

/**
 * Prepares sample buffer object for resampling 
 * operations with params 'from_srate' and 
 * 'to_srate'. This functions is meant for 
 * doing memory allocations and other similar 
 * operations which cannot be performed 
 * with realtime guarantees.
 */
void SAMPLE_BUFFER::resample_init_memory(srate_size_t from_srate,
					 srate_size_t to_srate) {
  double step = 1.0;
  if (from_srate != 0) { step = static_cast<double>(to_srate) / from_srate; }
  buf_size_t new_buffer_size = static_cast<buf_size_t>(step * buffersize_rep);

  if (impl_repp->old_buffer_repp == 0) 
    impl_repp->old_buffer_repp = new sample_t [reserved_samples_rep];

  if (new_buffer_size > reserved_samples_rep) {
    reserved_samples_rep = new_buffer_size;

    for(int c = 0; c < channel_count_rep; c++) {
      delete[] buffer[c];
      buffer[c] = new sample_t [reserved_samples_rep];
    }
  }

  impl_repp->resample_memory_rep.resize(channel_count_rep, SAMPLE_SPECS::silent_value);
}

/**
 * Resamples samplebuffer contents.
 *
 * Note! 'resample_init_memory()' must be called before 
 *       before calling this function.
 */
void SAMPLE_BUFFER::resample_nofilter(srate_size_t from, 
				      srate_size_t to) {
  double step = static_cast<double>(to) / from;
  buf_size_t old_buffer_size = buffersize_rep;
  buffersize_rep = static_cast<buf_size_t>(step * buffersize_rep);

  DEBUG_RESAMPLING_STATEMENT(std::cerr << "(samplebuffer) resample_no_f from " << from << " to " << to << "." std::endl); 

  DBC_CHECK(impl_repp->old_buffer_repp != 0);

  for(int c = 0; c < channel_count_rep; c++) {
    memcpy(impl_repp->old_buffer_repp, buffer[c], old_buffer_size * sizeof(sample_t));

    DBC_CHECK(buffersize_rep <= reserved_samples_rep);
    
    double counter = 0.0;
    buf_size_t new_buffer_index = 0;
    buf_size_t interpolate_index = 0;
     
    buffer[c][0] = impl_repp->old_buffer_repp[0];
    for(buf_size_t old_buffer_index = 1; old_buffer_index < old_buffer_size; old_buffer_index++) {
      counter += step;
      if (step <= 1) {
	if (counter >= new_buffer_index + 1) {
	  new_buffer_index++;
	  if (new_buffer_index >= buffersize_rep) break;
	  buffer[c][new_buffer_index] = impl_repp->old_buffer_repp[old_buffer_index];
	}
      }
      else {
	new_buffer_index = static_cast<buf_size_t>(ceil(counter));
	if (new_buffer_index >= buffersize_rep) new_buffer_index = buffersize_rep - 1;
	for(buf_size_t t = interpolate_index + 1; t < new_buffer_index; t++) {
	  buffer[c][t] = impl_repp->old_buffer_repp[old_buffer_index - 1] + ((impl_repp->old_buffer_repp[old_buffer_index]
									      - impl_repp->old_buffer_repp[old_buffer_index-1])
									     * static_cast<SAMPLE_BUFFER::sample_t>(t - interpolate_index)
									     / (new_buffer_index - interpolate_index));
	}
	buffer[c][new_buffer_index] = impl_repp->old_buffer_repp[old_buffer_index];
      }
      interpolate_index = new_buffer_index;
    }
  }
}

/**
 * Resamples samplebuffer contents.
 *
 * Note! 'resample_init_memory()' must be called before 
 *       before calling this function.
 */
void SAMPLE_BUFFER::resample_with_memory(srate_size_t from, 
					 srate_size_t to) {
  double step = (double)to / from;
  buf_size_t old_buffer_size = buffersize_rep;
  buffersize_rep = static_cast<buf_size_t>(step * buffersize_rep);

  DBC_CHECK(impl_repp->old_buffer_repp != 0);
  DEBUG_RESAMPLING_STATEMENT(std::cerr << "(samplebuffer) resample_w_m from " << from << " to " << to << "." std::endl); 
  /* FIXME: resize() may end up allocating memory! */
  impl_repp->resample_memory_rep.resize(channel_count_rep);

  for(int c = 0; c < channel_count_rep; c++) {
    memcpy(impl_repp->old_buffer_repp, buffer[c], old_buffer_size * sizeof(sample_t));

    DBC_CHECK(buffersize_rep <= reserved_samples_rep);
    
    double counter = 0.0;
    buf_size_t new_buffer_index = 0;
    buf_size_t interpolate_index = -1;
    sample_t from_point;

    for(buf_size_t old_buffer_index = 0; old_buffer_index < old_buffer_size; old_buffer_index++) {
      counter += step;
      if (step <= 1) {
	if (counter >= new_buffer_index + 1) {
	  new_buffer_index++;
	  if (new_buffer_index >= buffersize_rep) break;
	  buffer[c][new_buffer_index] = impl_repp->old_buffer_repp[old_buffer_index];
	}
      }
      else {
	new_buffer_index = static_cast<buf_size_t>(ceil(counter));
	if (old_buffer_index == 0) from_point = impl_repp->resample_memory_rep[c];
	else from_point = impl_repp->old_buffer_repp[old_buffer_index-1];
	if (new_buffer_index >= buffersize_rep) new_buffer_index = buffersize_rep - 1;
	for(buf_size_t t = interpolate_index + 1; t < new_buffer_index; t++) {
	  buffer[c][t] = from_point + ((impl_repp->old_buffer_repp[old_buffer_index]
					- from_point)
				       * static_cast<SAMPLE_BUFFER::sample_t>(t - interpolate_index)
				       / (new_buffer_index - interpolate_index));
	}
	buffer[c][new_buffer_index] = impl_repp->old_buffer_repp[old_buffer_index];
      }
      interpolate_index = new_buffer_index;
    }
    impl_repp->resample_memory_rep[c] = impl_repp->old_buffer_repp[old_buffer_size - 1];
  }
}

void SAMPLE_BUFFER::resample_extfilter(srate_size_t from_srate,
					srate_size_t to_srate) 
{
}

void SAMPLE_BUFFER::resample_simplefilter(srate_size_t from_srate,
					  srate_size_t to_srate) 
{ 
}
