// ------------------------------------------------------------------------
// samplebuffer.cpp: Class representing a buffer of audio samples.
// Copyright (C) 1999-2005,2009 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
//
// References:
//     http://www.mega-nerd.com/SRC/
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

#include <iostream>
#include <vector>

#include <cmath>   /* ceil(), floor() */
#include <cstring> /* memcpy */
#include <cstdlib> /* labs() */

#include <sys/types.h>

#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#ifdef ECA_COMPILE_SAMPLERATE
#include <samplerate.h>
#endif

#include "eca-sample-conversion.h"
#include "samplebuffer.h"
#include "samplebuffer_impl.h"
#include "eca-logger.h"

/* Debug resampling operations */ 
// #define DEBUG_RESAMPLING

#ifdef DEBUG_RESAMPLING
#define DEBUG_RESAMPLING_STATEMENT(x) x
#else
#define DEBUG_RESAMPLING_STATEMENT(x) ((void)0)
#endif

#ifdef WORDS_BIGENDIAN
static const bool is_system_littleendian = false;
#else
static const bool is_system_littleendian = true;
#endif

/**
 * Constructs a new sample buffer object.
 */
SAMPLE_BUFFER::SAMPLE_BUFFER (buf_size_t buffersize, channel_size_t channels)
  : channel_count_rep(channels),
    buffersize_rep(buffersize),
    reserved_samples_rep(buffersize) 
{
  // ---
  DBC_REQUIRE(buffersize >= 0);
  DBC_REQUIRE(channels >= 0);
  // ---

  impl_repp = new SAMPLE_BUFFER_impl;

  buffer.resize(channels);
  for(size_t n = 0; n < buffer.size(); n++) {
    buffer[n] = new sample_t [reserved_samples_rep];
  }
  make_silent();

  impl_repp->rt_lock_rep = false;
  impl_repp->lockref_rep = 0;
  impl_repp->old_buffer_repp = 0;
#ifdef ECA_COMPILE_SAMPLERATE
  impl_repp->quality_rep = 50;
  impl_repp->src_state_rep.resize(channels);
#else
  impl_repp->quality_rep = 5;
#endif
 
  ECA_LOG_MSG(ECA_LOGGER::functions, 
		"Buffer created, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ".");

  // ---
  DBC_ENSURE(buffer.size() == static_cast<size_t>(channel_count_rep));
  // ---
}

/**
 * Constructs a new sample buffer object from a reference
 * to an already existing object.
 *
 * For better performance, doesn't copy IO-buffers nor
 * iterator state.
 *
 * ** Note! This function is obsolete!
 */
SAMPLE_BUFFER::SAMPLE_BUFFER (const SAMPLE_BUFFER& x)
  : channel_count_rep(x.channel_count_rep),
    buffersize_rep(x.buffersize_rep),
    reserved_samples_rep(x.reserved_samples_rep)
{
  impl_repp = new SAMPLE_BUFFER_impl;

  buffer.resize(x.buffer.size());

  for(size_t n = 0; n < buffer.size(); n++) {
    buffer[n] = new sample_t [reserved_samples_rep];
    std::memcpy(buffer[n], x.buffer[n], buffersize_rep * sizeof(sample_t));
  }

  impl_repp->old_buffer_repp = 0;

  ECA_LOG_MSG(ECA_LOGGER::functions, 
		"Buffer copy-constructed, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ".");

  // ---
  DBC_ENSURE(buffer.size() == static_cast<size_t>(channel_count_rep));
  DBC_ENSURE(buffersize_rep == reserved_samples_rep);
  // ---
}

/**
 * Assignment operator.
 *
 * For better performance, doesn't copy IO-buffers nor
 * iterator state.
 *
 * ** Note! This function is obsolete!
 */
SAMPLE_BUFFER& SAMPLE_BUFFER::operator=(const SAMPLE_BUFFER& x)
{
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

    for(size_t n = 0; n < buffer.size(); n++) {
      std::memcpy(buffer[n], x.buffer[n], buffersize_rep * sizeof(sample_t));
    }
  }

  return *this;

  // ---
  DBC_ENSURE(buffer.size() == static_cast<size_t>(channel_count_rep));
  DBC_ENSURE(buffersize_rep == reserved_samples_rep);
  // ---
}

/**
 * Destructor.
 */
SAMPLE_BUFFER::~SAMPLE_BUFFER (void)
{
  DBC_CHECK(impl_repp->lockref_rep == 0);

  for(size_t n = 0; n < buffer.size(); n++) {
    if (buffer[n] != 0) {
      delete[] buffer[n];
      buffer[n] = 0;
    }
  }

  if (impl_repp->old_buffer_repp != 0) {
    delete[] impl_repp->old_buffer_repp;
    impl_repp->old_buffer_repp = 0;
  }

#ifdef ECA_COMPILE_SAMPLERATE
  for(size_t n = 0; n < impl_repp->src_state_rep.size(); n++) {
    if (impl_repp->src_state_rep[n] != 0) {
      src_delete(impl_repp->src_state_rep[n]);
      impl_repp->src_state_rep[n] = 0;
    }
  }
#endif

  delete impl_repp;
}

/**
 * Channel-wise addition. Buffer length is increased if necessary.
 *
 * @post length_in_samples() >= x.length_in_samples()
 */
void SAMPLE_BUFFER::add(const SAMPLE_BUFFER& x)
{
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
 * Note: event tags are not copied!
 * 
 * @pre weight != 0
 * @post length_in_samples() >= x.length_in_samples()
 */
void SAMPLE_BUFFER::add_with_weight(const SAMPLE_BUFFER& x, int weight)
{
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
 * Note: event tags are not copied!
 * 
 * @post length_in_samples() == x.length_in_samples()
 */
void SAMPLE_BUFFER::copy(const SAMPLE_BUFFER& x)
{
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
 * Note: event tags are not copied!
 * 
 * @pre start_pos <= end_pos
 * @pre 
 * @pre to_pos < length_in_samples()
 */
void SAMPLE_BUFFER::copy_range(const SAMPLE_BUFFER& src, 
			       buf_size_t src_start_pos,
			       buf_size_t src_end_pos,
			       buf_size_t dst_to_pos) 
{
  // ---
  DBC_REQUIRE(src_start_pos <= src_end_pos);
  DBC_REQUIRE(dst_to_pos < length_in_samples());
  // ---

  int min_c_count = (channel_count_rep <= src.channel_count_rep) ? channel_count_rep : src.channel_count_rep;

  if (src_end_pos > src.length_in_samples())
    src_end_pos = src.length_in_samples();

  for(channel_size_t q = 0; q < min_c_count; q++) {
    buf_size_t dst_i = dst_to_pos;
    for(buf_size_t src_i = src_start_pos; 
	  src_i < src_end_pos && 
	  dst_i < length_in_samples();
	src_i++, dst_i++) {
      buffer[q][dst_i] = src.buffer[q][src_i];
    }
  }
}

/**
 * Divides all samples by 'dvalue'.
 */
void SAMPLE_BUFFER::divide_by(SAMPLE_BUFFER::sample_t dvalue)
{
  for(channel_size_t n = 0; n < channel_count_rep; n++) {
    for(buf_size_t m = 0; m < buffersize_rep; m++) {
      buffer[n][m] /= dvalue;
    }
  }
}

/**
 * Clears the buffer to zero length. Note that this is
 * different from a silent buffer.
 */
void SAMPLE_BUFFER::make_empty(void)
{
  SAMPLE_BUFFER::length_in_samples(0);
}

/**
 * Mutes the whole buffer.
 */
void SAMPLE_BUFFER::make_silent(void)
{
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
void SAMPLE_BUFFER::limit_values(void)
{
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
 * Resamples samplebuffer contents. Resampling
 * changes buffer length by 'to_rate/from_rate'.
 *
 * @post to_rate / from_rate * old_length_in_samples - length_in_samples() >= -1
 */
void SAMPLE_BUFFER::resample(SAMPLE_SPECS::sample_rate_t from_rate,
			     SAMPLE_SPECS::sample_rate_t to_rate)
{
#ifndef ECA_COMPILE_SAMPLERATE
  DBC_DECLARE(buf_size_t old_length_in_samples = length_in_samples());
#endif

#ifdef ECA_COMPILE_SAMPLERATE
  if (impl_repp->quality_rep > 5) {
    resample_secret_rabbit_code(from_rate, to_rate);
  }
  else 
#endif
    {
      DBC_CHECK(impl_repp->quality_rep <= 5);
      resample_with_memory(from_rate, to_rate); 
    }

#ifndef ECA_COMPILE_SAMPLERATE
  /* with libsamplerate, the output sample count can vary from call to call */
  DBC_CHECK((static_cast<double>(to_rate) / from_rate * old_length_in_samples - length_in_samples()) >= -1);
#endif
}

/**
 * Set resampling quality. 
 *
 * Depending on build options, not all quality levels
 * are necessarily supported. If the requested quality level 
 * exceeds the available resamplers, the quality setting 
 * is set to the highest available algorithm. You can use
 * the get_resample_quality() function to query the current level.
 *
 * @param quality value between 0 (lowest) to 100 (highest)
 */
void SAMPLE_BUFFER::resample_set_quality(int quality)
{
#ifdef ECA_COMPILE_SAMPLERATE
  impl_repp->quality_rep = quality;
#else
  if (quality > 10) {
    ECA_LOG_MSG(ECA_LOGGER::info, 
		"WARNING: Libsamplerate is required for high-quality resampling. "
		"Using the internal resampler instead.");
    impl_repp->quality_rep = 5;
  }
#endif
}

/**
 * Returns current resampling quality.
 *
 * @param value between 0 (lowest) to 100 (highest)
 */
int SAMPLE_BUFFER::resample_get_quality(void) const
{
  return impl_repp->quality_rep;
}

void SAMPLE_BUFFER::export_helper(unsigned char* obuffer, 
				  buf_size_t* optr,
				  sample_t value,
				  ECA_AUDIO_FORMAT::Sample_format fmt)

{
  switch (fmt) {
  case ECA_AUDIO_FORMAT::sfmt_u8:
    {
      obuffer[(*optr)++] = eca_sample_convert_float_to_u8(value);
      break;
    }
    
  case ECA_AUDIO_FORMAT::sfmt_s16_le:
    {
      int16_t s16temp = eca_sample_convert_float_to_s16(value);

      // little endian: (LSB, MSB) (Intel).
      obuffer[(*optr)++] = (unsigned char)(s16temp & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s16temp >> 8) & 0xff);
      break;
    }
      
  case ECA_AUDIO_FORMAT::sfmt_s16_be:
    {
      int16_t s16temp = eca_sample_convert_float_to_s16(value);
      
      // big endian: (MSB, LSB) (Motorola).
      obuffer[(*optr)++] = (unsigned char)((s16temp >> 8) & 0xff);
      obuffer[(*optr)++] = (unsigned char)(s16temp & 0xff);
      break;
    }
    
  case ECA_AUDIO_FORMAT::sfmt_s24_le:
    {
      int32_t s32temp = eca_sample_convert_float_to_s32(value);
      
      /* skip the LSB-byte of s32temp (s32temp & 0xff) */
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 8) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 16) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 24) & 0xff);	
      break;
    }
    
  case ECA_AUDIO_FORMAT::sfmt_s24_be:
    {
      int32_t s32temp = eca_sample_convert_float_to_s32(value);
      
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 24) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 16) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 8) & 0xff);
      /* skip the LSB-byte of s32temp (s32temp & 0xff) */
      break;
    }
    
  case ECA_AUDIO_FORMAT::sfmt_s32_le:
    {
      int32_t s32temp = eca_sample_convert_float_to_s32(value);
      
      obuffer[(*optr)++] = (unsigned char)(s32temp & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 8) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 16) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 24) & 0xff);
      break;
    }
    
  case ECA_AUDIO_FORMAT::sfmt_s32_be:
    {
      int32_t s32temp = eca_sample_convert_float_to_s32(value);
      
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 24) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 16) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((s32temp >> 8) & 0xff);
      obuffer[(*optr)++] = (unsigned char)(s32temp & 0xff);
      break;
    }
    
  case ECA_AUDIO_FORMAT::sfmt_f32_le:
    {
      union { int32_t i; float f; } f32temp;
      f32temp.f = (float)value;
      obuffer[(*optr)++] = (unsigned char)(f32temp.i & 0xff);
      obuffer[(*optr)++] = (unsigned char)((f32temp.i >> 8) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((f32temp.i >> 16) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((f32temp.i >> 24) & 0xff);
      break;
    }
    
  case ECA_AUDIO_FORMAT::sfmt_f32_be:
    {
      union { int32_t i; float f; } f32temp;
      f32temp.f = (float)value;
      obuffer[(*optr)++] = (unsigned char)((f32temp.i >> 24) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((f32temp.i >> 16) & 0xff);
      obuffer[(*optr)++] = (unsigned char)((f32temp.i >> 8) & 0xff);
      obuffer[(*optr)++] = (unsigned char)(f32temp.i & 0xff);
      break;
    }
    
  default: 
    { 
      ECA_LOG_MSG(ECA_LOGGER::info, "Unknown sample format! [1].");
    }
  }
}

/**
 * Exports contents of sample buffer to 'target'. Sample data 
 * will be converted according to the given arguments
 * (sample format and endianess). If 'chcount > 1', channels 
 * will be written interleaved.
 *
 * Note! If chcount > number_of_channels(), empty
 *       channels will be automatically added.
 *
 * @pre target != 0
 * @pre chcount > 0
 * @ensure number_of_channels() >= chcount
 */
void SAMPLE_BUFFER::export_interleaved(unsigned char* target,
				       ECA_AUDIO_FORMAT::Sample_format fmt,
				       channel_size_t chcount) 
{
  // --------
  DBC_REQUIRE(target != 0);
  DBC_REQUIRE(chcount > 0);
  // --------

  if (chcount > channel_count_rep) number_of_channels(chcount);

  buf_size_t osize = 0;
  for(buf_size_t isize = 0; isize < buffersize_rep; isize++) {
    for(channel_size_t c = 0; c < chcount; c++) {
      sample_t stemp = buffer[c][isize];
      if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
      else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;

      SAMPLE_BUFFER::export_helper(target, &osize, stemp, fmt);
    }
  }
  
  // -------
  DBC_ENSURE(number_of_channels() >= chcount);
  // -------
}

/**
 * Same as 'export_data()', but 'target' data is 
 * written in non-interleaved format.
 *
 * Note! If chcount > number_of_channels(), empty
 *       channels will be automatically added.
 *
 * @pre target != 0
 * @pre chcount > 0
 * @ensure number_of_channels() >= chcount
 */
void SAMPLE_BUFFER::export_noninterleaved(unsigned char* target,
					  ECA_AUDIO_FORMAT::Sample_format fmt,
					  channel_size_t chcount)
{
  // --------
  DBC_REQUIRE(target != 0);
  DBC_REQUIRE(chcount > 0);
  // --------

  if (chcount > channel_count_rep) number_of_channels(chcount);

  buf_size_t osize = 0;
  for(channel_size_t c = 0; c < chcount; c++) {
    for(buf_size_t isize = 0; isize < buffersize_rep; isize++) {
      sample_t stemp = buffer[c][isize];
      if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
      else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
      
      SAMPLE_BUFFER::export_helper(target, &osize, stemp, fmt);
    }
  }

  // -------
  DBC_ENSURE(number_of_channels() >= chcount);
  // -------
}

void SAMPLE_BUFFER::import_helper(const unsigned char *ibuffer,
				  buf_size_t* iptr,
				  sample_t* obuffer,
				  buf_size_t optr,
				  ECA_AUDIO_FORMAT::Sample_format fmt)
{
  unsigned char a[2];
  unsigned char b[4];

  switch (fmt) {
  case ECA_AUDIO_FORMAT::sfmt_u8: 
    {
      obuffer[optr] = eca_sample_convert_u8_to_float(ibuffer[(*iptr)++]);
    }
    break;
	
  case ECA_AUDIO_FORMAT::sfmt_s16_le:
    {
      // little endian: (LSB, MSB) (Intel)
      // big endian: (MSB, LSB) (Motorola)
      if (is_system_littleendian) {
	a[0] = ibuffer[(*iptr)++];
	a[1] = ibuffer[(*iptr)++];
      }
      else {
	a[1] = ibuffer[(*iptr)++];
	a[0] = ibuffer[(*iptr)++];
      }
      obuffer[optr] = eca_sample_convert_s16_to_float(*(int16_t*)a);
    }
    break;

  case ECA_AUDIO_FORMAT::sfmt_s16_be:
    {
      if (!is_system_littleendian) {
	a[0] = ibuffer[(*iptr)++];
	a[1] = ibuffer[(*iptr)++];
      }
      else {
	a[1] = ibuffer[(*iptr)++];
	a[0] = ibuffer[(*iptr)++];
      }
      obuffer[optr] = eca_sample_convert_s16_to_float(*(int16_t*)a);
    }
    break;

  case ECA_AUDIO_FORMAT::sfmt_s24_le:
    {
      if (is_system_littleendian) {
	b[0] = 0; /* LSB */
	b[1] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[3] = ibuffer[(*iptr)++];
      }
      else {
	b[3] = 0; /* LSB */
	b[2] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[0] = ibuffer[(*iptr)++];
      }
      obuffer[optr] = eca_sample_convert_s32_to_float((*(int32_t*)b));
    }
    break;

  case ECA_AUDIO_FORMAT::sfmt_s24_be:
    {
      if (is_system_littleendian) {
	b[3] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[0] = 0; /* LSB */
      }
      else {
	b[0] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[3] = 0; /* LSB */
      }
      obuffer[optr] = eca_sample_convert_s32_to_float((*(int32_t*)b));
    }
    break;

  case ECA_AUDIO_FORMAT::sfmt_s32_le:
    {
      if (is_system_littleendian) {
	b[0] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[3] = ibuffer[(*iptr)++];
      }
      else {
	b[3] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[0] = ibuffer[(*iptr)++];
      }
      obuffer[optr] = eca_sample_convert_s32_to_float(*(int32_t*)b);
    }
    break;

  case ECA_AUDIO_FORMAT::sfmt_s32_be:
    {
      if (is_system_littleendian) {
	b[3] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[0] = ibuffer[(*iptr)++];
      }
      else {
	b[0] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[3] = ibuffer[(*iptr)++];
      }
      obuffer[optr] = eca_sample_convert_s32_to_float(*(int32_t*)b);
    }
    break;

  case ECA_AUDIO_FORMAT::sfmt_f32_le:
    {
      if (is_system_littleendian) {
	b[0] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[3] = ibuffer[(*iptr)++];
      }
      else {
	b[3] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[0] = ibuffer[(*iptr)++];
      }
      obuffer[optr] = (sample_t)(*(float*)b);
    }
    break;

  case ECA_AUDIO_FORMAT::sfmt_f32_be:
    {
      if (is_system_littleendian) {
	b[3] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[0] = ibuffer[(*iptr)++];
      }
      else {
	b[0] = ibuffer[(*iptr)++];
	b[1] = ibuffer[(*iptr)++];
	b[2] = ibuffer[(*iptr)++];
	b[3] = ibuffer[(*iptr)++];
      }
      obuffer[optr] = (sample_t)(*(float*)b);
    }
    break;

  default: 
    { 
      ECA_LOG_MSG(ECA_LOGGER::info, "Unknown sample format! [4].");
    }
  }
}

/**
 * Import audio from external raw buffer. Sample data 
 * will be converted to internal sample format using the 
 * given arguments (sample format and endianess). 
 * Channels will be read interleaved.
 *
 * @pre source != 0
 * @pre samples_read >= 0
 */
void SAMPLE_BUFFER::import_interleaved(unsigned char* source,
				       buf_size_t samples_read,
				       ECA_AUDIO_FORMAT::Sample_format fmt,
				       channel_size_t chcount)
{
  // --------
  DBC_REQUIRE(source != 0);
  DBC_REQUIRE(samples_read >= 0);
  // --------

  if (channel_count_rep != chcount) number_of_channels(chcount);
  if (buffersize_rep != samples_read) length_in_samples(samples_read);

  buf_size_t isize = 0;

  for(buf_size_t osize = 0; osize < buffersize_rep; osize++) {
    for(channel_size_t c = 0; c < chcount; c++) {
      import_helper(source, &isize, buffer[c], osize, fmt);
    }
  }
}

/**
 * Same as 'import_interleaved()', but 'source' data is 
 * assumed to be in non-interleaved format.
 *
 * @pre source != 0
 * @pre samples_read >= 0
 */
void SAMPLE_BUFFER::import_noninterleaved(unsigned char* source,
					  buf_size_t samples_read,
					  ECA_AUDIO_FORMAT::Sample_format fmt,
					  channel_size_t chcount)
{
  // --------
  DBC_REQUIRE(source != 0);
  DBC_REQUIRE(samples_read >= 0);
  // --------

  if (channel_count_rep != chcount) number_of_channels(chcount);
  if (buffersize_rep != samples_read) length_in_samples(samples_read);

  buf_size_t isize = 0;
  for(channel_size_t c = 0; c < chcount; c++) {
    for(buf_size_t osize = 0; osize < buffersize_rep; osize++) {
      import_helper(source, &isize, buffer[c], osize, fmt);
    }
  }
}

/** 
 * Sets the number of audio channels.
 */
void SAMPLE_BUFFER::number_of_channels(channel_size_t len) 
{
  // std::cerr << "(samplebuffer_impl) ch-count changes from " << channel_count_rep << " to " << len << ".\n";

  if (len > static_cast<channel_size_t>(buffer.size())) {
    DBC_CHECK(impl_repp->rt_lock_rep != true);

    size_t old_size = buffer.size();
    buffer.resize(len);
    for(channel_size_t n = old_size; n < len; n++) {
      buffer[n] = new sample_t [reserved_samples_rep];
    }
    ECA_LOG_MSG(ECA_LOGGER::functions, "Increasing channel-count (1).");    
  }

  /* note! channel_count_rep and buffer.size() necessarily
   *       weren't the same before this call, so we need
   *       to double check for old data
   */
  if (len > channel_count_rep) {
    for(channel_size_t n = channel_count_rep; n < len; n++) {
      for(buf_size_t m = 0; m < reserved_samples_rep; m++) {
	buffer[n][m] = SAMPLE_SPECS::silent_value;
      }
    }
    // ECA_LOG_MSG(ECA_LOGGER::system_objects, "Increasing channel-count (2).");
  }

  channel_count_rep = len;
}

/**
 * Sets the length of buffer in samples.
 *
 * Note: if length is increased, the added samples
 *       are muted.
 */
void SAMPLE_BUFFER::length_in_samples(buf_size_t len)
{
  DBC_REQUIRE(len >= 0);
  DBC_CHECK(buffersize_rep <= reserved_samples_rep);

  if (len > reserved_samples_rep) {

    DBC_CHECK(impl_repp->rt_lock_rep != true);
    DBC_CHECK(impl_repp->lockref_rep == 0);

    reserved_samples_rep = len * 2;
    for(size_t n = 0; n < buffer.size(); n++) {
      sample_t *prev_buffer = buffer[n];
      buffer[n] = new sample_t [reserved_samples_rep];
      for (buf_size_t m = 0; m < buffersize_rep; m++)
	buffer[n][m] = prev_buffer[m];
      delete[] prev_buffer;
    }

    if (impl_repp->old_buffer_repp != 0) {
      delete[] impl_repp->old_buffer_repp;
      impl_repp->old_buffer_repp = new sample_t [reserved_samples_rep];
    }
  }

  if (len > buffersize_rep) {
    for(size_t n = 0; n < buffer.size(); n++) {
      /* note: mute starting from 'buffersize_rep' */
      for(buf_size_t m = buffersize_rep; m < reserved_samples_rep; m++) {
	buffer[n][m] = SAMPLE_SPECS::silent_value;
      }
    }
  }

  buffersize_rep = len;
}

/**
 * Prepares sample buffer object for resampling 
 * operations with params 'from_srate' and 
 * 'to_srate'. This functions is meant for 
 * doing memory allocations and other similar 
 * operations which cannot be performed 
 * with realtime guarantees.
 */
void SAMPLE_BUFFER::resample_init_memory(SAMPLE_SPECS::sample_rate_t from_srate,
					 SAMPLE_SPECS::sample_rate_t to_srate)
{
#ifdef ECA_COMPILE_SAMPLERATE
  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"Resampler selected: libsamplerate (Secret Rabbit Code).");
#else
  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"Resampler selected: internal resampler.");
#endif

  double step = 1.0;
  if (from_srate != 0) { step = static_cast<double>(to_srate) / from_srate; }

  /* add at least one word of extra space */
  buf_size_t new_buffer_size = static_cast<buf_size_t>((step * buffersize_rep)) + sizeof(buf_size_t);

  if (new_buffer_size > reserved_samples_rep) {
    reserved_samples_rep = new_buffer_size * 2;

#ifdef ECA_DEBUG_MODE
    DBC_CHECK(impl_repp->rt_lock_rep != true);
    DBC_CHECK(impl_repp->lockref_rep == 0);
#endif

    for(int c = 0; c < channel_count_rep; c++) {
      delete[] buffer[c];
      buffer[c] = new sample_t [reserved_samples_rep];
    }
  }

#ifdef ECA_COMPILE_SAMPLERATE
  impl_repp->src_state_rep.resize(channel_count_rep);
  for(int c = 0; c < channel_count_rep; c++) {
    if (impl_repp->src_state_rep[c] == 0) {
      int error;
      impl_repp->src_state_rep[c] = src_new((impl_repp->quality_rep > 75) ? SRC_SINC_BEST_QUALITY : SRC_SINC_MEDIUM_QUALITY, 1, &error);
      DBC_CHECK(impl_repp->src_state_rep[c] != 0);
    }
  }
#endif

  if (impl_repp->old_buffer_repp == 0) {
#ifdef ECA_DEBUG_MODE
    DBC_CHECK(impl_repp->rt_lock_rep != true);
#endif
    impl_repp->old_buffer_repp = new sample_t [reserved_samples_rep];
  }

  if (impl_repp->resample_memory_rep.size() < static_cast<size_t>(channel_count_rep)) {
#ifdef ECA_DEBUG_MODE
    DBC_CHECK(impl_repp->rt_lock_rep != true);
#endif
    impl_repp->resample_memory_rep.resize(channel_count_rep, 0.0f);
  }
}

void SAMPLE_BUFFER::reserve_channels(channel_size_t num)
{
  channel_size_t oldcount = number_of_channels();
  number_of_channels(num);
  number_of_channels(oldcount);
}

void SAMPLE_BUFFER::reserve_length_in_samples(buf_size_t len)
{
  buf_size_t oldlen = length_in_samples();
  length_in_samples(len);
  length_in_samples(oldlen);
}

/**
 * Sets the realtime-lock state. When realtime-lock
 * is enabled, all non-rt-safe operations 
 * like for instance memory allocations are
 * blocked.
 * 
 * @param state true=lock, false=unlock
 */
void SAMPLE_BUFFER::set_rt_lock(bool state)
{
  impl_repp->rt_lock_rep = state;
}

/** 
 * Increases reference count of 'buffer' data
 * area.
 *
 * This should be issued when an object uses
 * direct access to the samplebuffer's 
 * audio data buffer.
 * 
 * Note! release_pointer_reflock() must be 
 * called after caller stops accessing
 * 'buffer'.
 */
void SAMPLE_BUFFER::get_pointer_reflock(void)
{
  impl_repp->lockref_rep++;
}

/** 
 * Increases reference count of 'buffer' data
 * area.
 *
 * @see get_pointer_reflock()
 */
void SAMPLE_BUFFER::release_pointer_reflock(void)
{
  impl_repp->lockref_rep--;
  DBC_ENSURE(impl_repp->lockref_rep >= 0);
}

/**
 * Adds all event tags that are set for 'sbuf' (bitwise-OR).
 */
void SAMPLE_BUFFER::event_tags_add(const SAMPLE_BUFFER& sbuf)
{
  impl_repp->event_tags_rep |= 
    sbuf.impl_repp->event_tags_rep;
}

/**
 * Sets only those event flags that are set for 'sbuf'.
 */
void SAMPLE_BUFFER::event_tags_set(const SAMPLE_BUFFER& sbuf)
{
  impl_repp->event_tags_rep = 
    sbuf.impl_repp->event_tags_rep;
}

/**
 * Clears all tags matching 'tagmask'
 */
void SAMPLE_BUFFER::event_tags_clear(Tag_name tagmask)
{
  event_tag_set(tagmask, false);
}

/** 
 * Set/clears the event tag 'tag'.
 */
void SAMPLE_BUFFER::event_tag_set(Tag_name tag, bool val)
{
  if (val)
    impl_repp->event_tags_rep |= tag;
  else
    impl_repp->event_tags_rep &= ~tag;
}

bool SAMPLE_BUFFER::event_tag_test(Tag_name tag)
{
  return (impl_repp->event_tags_rep & tag) ? true : false;
}

/**
 * Resamples samplebuffer contents.
 *
 * Note! 'resample_init_memory()' must be called before 
 *       before calling this function.
 */
void SAMPLE_BUFFER::resample_nofilter(SAMPLE_SPECS::sample_rate_t from, 
				      SAMPLE_SPECS::sample_rate_t to)
{
  double step = static_cast<double>(to) / from;
  buf_size_t old_buffer_size = buffersize_rep;

  // truncate, not round, to integer
  length_in_samples(static_cast<buf_size_t>(std::floor(step * buffersize_rep)));

  DEBUG_RESAMPLING_STATEMENT(std::cerr << "resample_no_f from " << from << " to " << to << "." << std::endl);

  DBC_CHECK(impl_repp->old_buffer_repp != 0);

  for(int c = 0; c < channel_count_rep; c++) {
    std::memcpy(impl_repp->old_buffer_repp, buffer[c], old_buffer_size * sizeof(sample_t));

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
	new_buffer_index = static_cast<buf_size_t>(std::ceil(counter));
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
void SAMPLE_BUFFER::resample_with_memory(SAMPLE_SPECS::sample_rate_t from, 
					 SAMPLE_SPECS::sample_rate_t to)
{
  double step = (double)to / from;
  buf_size_t old_buffer_size = buffersize_rep;

  // truncate, not round, to integer
  length_in_samples(static_cast<buf_size_t>(std::floor(step * buffersize_rep)));

  DBC_CHECK(impl_repp->old_buffer_repp != 0);
  DEBUG_RESAMPLING_STATEMENT(std::cerr << "(samplebuffer) resample_w_m from " << from << " to " << to << "." << std::endl); 

  if (impl_repp->resample_memory_rep.size() < static_cast<size_t>(channel_count_rep)) {
    DBC_CHECK(impl_repp->rt_lock_rep != true);
    impl_repp->resample_memory_rep.resize(channel_count_rep, 0.0f);
  }

  length_in_samples(buffersize_rep);

  for(int c = 0; c < channel_count_rep; c++) {
    std::memcpy(impl_repp->old_buffer_repp, buffer[c], old_buffer_size * sizeof(sample_t));

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
	new_buffer_index = static_cast<buf_size_t>(std::ceil(counter));
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

void SAMPLE_BUFFER::resample_secret_rabbit_code(SAMPLE_SPECS::sample_rate_t from_srate,
						SAMPLE_SPECS::sample_rate_t to_srate) 
{
#ifdef ECA_COMPILE_SAMPLERATE
  SRC_DATA params;
  long ch_out_count = -1;
  double step = static_cast<double>(to_srate) / from_srate;
  buf_size_t old_buffer_size = buffersize_rep;

  /* modify buffersize_rep (size of dst buffer) */
  length_in_samples(static_cast<buf_size_t>(std::floor(step * buffersize_rep)));

  DBC_CHECK(impl_repp->old_buffer_repp != 0);
  DEBUG_RESAMPLING_STATEMENT(std::cerr << "(samplebuffer) resample_s_r_c from " << from_srate << " to " << to_srate << "." << std::endl); 

  for(int c = 0; c < channel_count_rep; c++) {
    std::memcpy(impl_repp->old_buffer_repp, buffer[c], old_buffer_size * sizeof(sample_t));

    DBC_CHECK(buffersize_rep <= reserved_samples_rep);

    // A pointer to the input data samples.
    params.data_in = impl_repp->old_buffer_repp; 

    // The number of frames of data pointed to by data_in.
    params.input_frames = old_buffer_size;

    // A pointer to the output data samples.
    params.data_out = buffer[c];

    // Maximum number of frames pointer to by data_out.
    // note: was 'reserved_samples_rep' but this led 
    //       to corrupted output
    params.output_frames = reserved_samples_rep; /* buffersize_rep; */

    // Equal to output_sample_rate / input_sample_rate.
    params.src_ratio = step;
    params.end_of_input = 0;

    // update the step
    int ret = src_set_ratio(impl_repp->src_state_rep[c], step);
    DBC_CHECK(ret == 0);

    // Perform the sample rate conversion
    ret = src_process(impl_repp->src_state_rep[c], &params);
    DBC_CHECK(ret == 0);
    if (ret) {
      /* make sure we avoid segfault in all cases */
      params.output_frames_gen = 0;
      params.input_frames_used = old_buffer_size;
    }
    DBC_CHECK(std::labs(params.input_frames_used - old_buffer_size) == 0);
#ifdef ECA_DEBUG_MODE
    /* make sure all input samples have been used */
    if (old_buffer_size != params.input_frames_used) { std::cerr << "input_frames_over=" << old_buffer_size - params.input_frames_used << ".\n"; }

    /* check that all channels are processed in the same way */
    if (c == 0) ch_out_count = params.output_frames_gen;
    DBC_CHECK(ch_out_count == params.output_frames_gen);
#endif /* ECA_DEBUG_MODE */
  }

  DEBUG_RESAMPLING_STATEMENT(std::cerr << "(samplebuffer) src_src input=" << old_buffer_size 
			     << ", target=" << buffersize_rep 
			     << ", processed=" << params.input_frames_used 
			     << ", space_for=" << reserved_samples_rep 
			     << ", out=" << params.output_frames_gen << std::endl); 

#ifdef ECA_DEBUG_MODE
  if ((params.output_frames_gen - buffersize_rep) > 1) {
    std::cerr << "(samplebuffer) src_src input=" << old_buffer_size 
	      << ", target=" << buffersize_rep 
	      << ", processed=" << params.input_frames_used 
	      << ", space_for=" << reserved_samples_rep 
	      << ", out=" << params.output_frames_gen << std::endl; 
  }
#endif
  if (params.output_frames_gen != buffersize_rep) {
    /* note: we set buffersize_rep directly and bypass
     * length_in_samples(), but in this case it is safe as 
     * we have used 'reserved_samples_rep' as the upper limit */
    buffersize_rep = params.output_frames_gen;
  }
#endif /* ECA_COMPILE_SAMPLERATE */
}

void SAMPLE_BUFFER::resample_extfilter(SAMPLE_SPECS::sample_rate_t from_srate,
					SAMPLE_SPECS::sample_rate_t to_srate) 
{
}

void SAMPLE_BUFFER::resample_simplefilter(SAMPLE_SPECS::sample_rate_t from_srate,
					  SAMPLE_SPECS::sample_rate_t to_srate) 
{ 
}
