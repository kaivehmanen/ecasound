// ------------------------------------------------------------------------
// samplebuffer.cpp: Class representing a buffer of audio samples.
// Copyright (C) 2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <vector>
#include <cmath>

#include <sys/types.h>

#include <kvutils/kvu_numtostr.h>

#include "samplebuffer.h"
#include "samplebuffer_impl.h"

#include "eca-debug.h"

using std::vector;

SAMPLE_BUFFER::SAMPLE_BUFFER (buf_size_t buffersize, channel_size_t channels, srate_size_t sample_rate) 
  : channel_count_rep(channels),
    buffersize_rep(buffersize),
    sample_rate_rep(sample_rate),
    reserved_bytes_rep(buffersize) 
{

  impl_repp = new SAMPLE_BUFFER_impl;

  buffer.resize(channels);
  for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
    buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
  }
  impl_repp->old_buffer_repp = 0;
 
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer<>) Buffer created, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}


SAMPLE_BUFFER& SAMPLE_BUFFER::operator=(const SAMPLE_BUFFER& x) {
  // ---
  // For better performance, doesn't copy IO-buffers nor
  // iterator state.
  // ---
  
  if (this != &x) {

    impl_repp->resample_memory_rep = x.impl_repp->resample_memory_rep;
    
    if (x.buffersize_rep > reserved_bytes_rep ||
	x.buffer.size() != buffer.size()) {

      reserved_bytes_rep = x.buffersize_rep;

      for(int n = 0; n < static_cast<int>(buffer.size()); n++) delete[] buffer[n];
      buffer.resize(x.buffer.size());
      for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
	buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
      }
    }

    buffersize_rep = x.buffersize_rep;
    channel_count_rep = x.channel_count_rep;
    sample_rate_rep = x.sample_rate_rep;
    for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
      memcpy(buffer[n], x.buffer[n], buffersize_rep * sizeof(sample_type));
    }

  }
  return *this;
}

SAMPLE_BUFFER::SAMPLE_BUFFER (const SAMPLE_BUFFER& x)
  : channel_count_rep(x.channel_count_rep),
    buffersize_rep(x.buffersize_rep),
    sample_rate_rep(x.sample_rate_rep),
    reserved_bytes_rep(x.reserved_bytes_rep) {

  impl_repp = new SAMPLE_BUFFER_impl;

  // ---
  // For better performance, doesn't copy IO-buffers.
  // ---
  buffer.resize(x.buffer.size());
  for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
    buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    memcpy(buffer[n], x.buffer[n], buffersize_rep * sizeof(sample_type));
  }
  impl_repp->old_buffer_repp = 0;

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer<>) Buffer copy-constructed, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}

SAMPLE_BUFFER::~SAMPLE_BUFFER (void) { 
  for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
    delete[] buffer[n];
  }
  if (impl_repp->old_buffer_repp != 0) {
    delete[] impl_repp->old_buffer_repp;
    impl_repp->old_buffer_repp = 0;
  }
  delete impl_repp;
}

void SAMPLE_BUFFER::number_of_channels(int len) {
  /*  std::cerr << "(samplebuffer_impl) ch-count changes from " << channel_count_rep << " to " << len << ".\n"; */
  if (len > static_cast<long int>(buffer.size())) {
    int old_size = static_cast<int>(buffer.size());
    buffer.resize(len);
    for(int n = old_size; n < len; n++) {
      buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    }
    ecadebug->msg(ECA_DEBUG::system_objects, "(samplebuffer<>) Increasing channel-count.");    
  }
  channel_count_rep = len;
}

void SAMPLE_BUFFER::resize(long int buffersize) {
  if (buffersize > reserved_bytes_rep) {
    reserved_bytes_rep = buffersize;
    for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
      delete[] buffer[n];
      buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    }
    if (impl_repp->old_buffer_repp != 0) {
      delete[] impl_repp->old_buffer_repp;
      impl_repp->old_buffer_repp = 0;
    }
  }
  buffersize_rep = buffersize;
}


/**
 * Mute the whole buffer.
 */
void SAMPLE_BUFFER::make_silent(void) {
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int s = 0; s < buffersize_rep; s++) {
      buffer[n][s] = SAMPLE_SPECS::silent_value;
    }
  }
}

/**
 * Mute a range of samples.
 */
void SAMPLE_BUFFER::make_silent_range(long int start_pos,
					      long int end_pos) {
  assert(start_pos >= 0);
  assert(end_pos >= 0);

  for(int n = 0; n < channel_count_rep; n++) {
    for(long int s = start_pos; s < end_pos && s < buffersize_rep; s++) {
      buffer[n][s] = SAMPLE_SPECS::silent_value;
    }
  }
}

/**
 * Limit all samples to valid values. 
 */
void SAMPLE_BUFFER::limit_values(void) {
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int m = 0; m < buffersize_rep; m++) {
      if (buffer[n][m] > SAMPLE_SPECS::impl_max_value) 
	buffer[n][m] = SAMPLE_SPECS::impl_max_value;
      else if (buffer[n][m] < SAMPLE_SPECS::impl_min_value) 
	buffer[n][m] = SAMPLE_SPECS::impl_min_value;
    }
  }
}

/**
 * Divide all samples by 'dvalue'.
 */
void SAMPLE_BUFFER::divide_by(SAMPLE_BUFFER::sample_type dvalue) {
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int m = 0; m < buffersize_rep; m++) {
      buffer[n][m] /= dvalue;
    }
  }
}

/**
 * Channel-wise addition. Buffer length is increased if necessary.
 */
void SAMPLE_BUFFER::add(const SAMPLE_BUFFER& x) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(int q = 0; q != c_count; q++) {
    //    long int s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(long int t = 0; t != x.length_in_samples(); t++) {
      buffer[q][t] += x.buffer[q][t];
    }
  }
}

/**
 * Channel-wise, weighted addition. Before addition every sample is 
 * multiplied by '1/weight'. Buffer length is increased if necessary.
 */
void SAMPLE_BUFFER::add_with_weight(const SAMPLE_BUFFER& x, int weight) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(int q = 0; q != c_count; q++) {
    //    long int s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(long int t = 0; t != x.length_in_samples(); t++) {
      buffer[q][t] += x.buffer[q][t] / weight;
    }
  }
}

/**
 * Channel-wise copy. Buffer length is increased if necessary.
 */
void SAMPLE_BUFFER::copy(const SAMPLE_BUFFER& x) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(int q = 0; q != c_count; q++) {
    for(long int t = 0; t != x.length_in_samples(); t++) {
      buffer[q][t] = x.buffer[q][t];
    }
  }
}

/**
 * Ranged channel-wise copy. Copies samples in range 
 * 'start_pos' - 'end_pos' from buffer 'x' to current 
 * buffer and position 'to_pos'. 
 */
void SAMPLE_BUFFER::copy_range(const SAMPLE_BUFFER& x, 
			       long int start_pos,
			       long int end_pos,
			       long int to_pos) {
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  long int t = to_pos;

  assert(start_pos <= end_pos);
  assert(to_pos < length_in_samples());

  if (start_pos >= x.length_in_samples()) start_pos = x.length_in_samples();
  if (end_pos >= x.length_in_samples()) end_pos = x.length_in_samples();

  for(int q = 0; q != c_count; q++) {
    for(long int s = start_pos; s != end_pos && t < x.length_in_samples(); s++) {
      buffer[q][t] = x.buffer[q][s];
      ++t;
    }
  }
}

/**
 * Copy contents of sample buffer to 'target'. Sample data 
 * will be converted according to the given arguments
 * (sample rate, sample format and endianess).
 *
 * ensure:
 *  channels == channel_count_rep
 */
void SAMPLE_BUFFER::copy_from_buffer(unsigned char* target,
				     ECA_AUDIO_FORMAT::Sample_format fmt,
				     int ch,
				     long int srate) 
{
  // --------
  // require:
  assert(target != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) {
    /*  std::cerr << "(samplebuffer_impl) chcount changes from " << channel_count_rep << " to " << ch << ".\n"; */
    number_of_channels(ch);
  }
  if (srate != sample_rate_rep) resample_to(srate);

  assert(ch == channel_count_rep);

  long int osize = 0;
  for(long int isize = 0; isize < buffersize_rep; isize++) {
    for(int c = 0; c < ch; c++) {
      sample_type stemp = buffer[c][isize];
      if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
      else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
      
      switch (fmt) {
      case ECA_AUDIO_FORMAT::sfmt_u8:
	{
	  // --- for debugging signal flow
	  //	  printf("(c %u)(isize %u)(osize %u) converting %.2f, \n",
	  //		   c, isize, osize, buffer[c][isize]);
	  target[osize++] = (unsigned
			     char)((sample_type)(stemp / SAMPLE_SPECS::u8_to_st_constant) + SAMPLE_SPECS::u8_to_st_delta);
	  // --- for debugging signal flow
	  //	  printf("converted to u8 %u (hex:%x)\n", target[osize-1], target[osize-1]);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s16_le:
	{
	  int16_t s16temp;
	  if (stemp < 0) 
	    s16temp = (int16_t)(sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_type)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
	
	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
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
	    s16temp = (int16_t)(sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_type)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
	
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
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = 0;
	
	  if (s32temp < 0) target[osize - 2] |=  0x80;
	  //    	  if (osize == 4) printf("neg.target: %x:%x:%x:%x.\n",target[osize-4],
	  //    				 target[osize-3],
	  //    				 target[osize-2],
	  //    				 target[osize-1])
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s24_be:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

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
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
	
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
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
  
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
}

/**
 * Same as 'copy_from_buffer()', but 'target' data is 
 * written in non-interleaved format.
 *
 * ensure:
 *  channels == channel_count_rep
 */
void SAMPLE_BUFFER::copy_from_buffer_vector(unsigned char* target,
						    ECA_AUDIO_FORMAT::Sample_format fmt,
						    int ch,
						    long int srate) 
{
  // --------
  // require:
  assert(target != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) {
    /*  std::cerr << "(samplebuffer_impl) chcount changes from " << channel_count_rep << " to " << ch << ".\n"; */
    number_of_channels(ch);
  }
  if (srate != sample_rate_rep) resample_to(srate);

  assert(ch == channel_count_rep);

  long int osize = 0;
  for(int c = 0; c < ch; c++) {
    for(long int isize = 0; isize < buffersize_rep; isize++) {
      sample_type stemp = buffer[c][isize];
      if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
      else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
      
      switch (fmt) {
      case ECA_AUDIO_FORMAT::sfmt_u8:
	{
	  // --- for debugging signal flow
	  //	  printf("(c %u)(isize %u)(osize %u) converting %.2f, \n",
	  //		   c, isize, osize, buffer[c][isize]);
	  target[osize++] = (unsigned
			     char)((sample_type)(stemp / SAMPLE_SPECS::u8_to_st_constant) + SAMPLE_SPECS::u8_to_st_delta);
	  // --- for debugging signal flow
	  //	  printf("converted to u8 %u (hex:%x)\n", target[osize-1], target[osize-1]);
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s16_le:
	{
	  int16_t s16temp;
	  if (stemp < 0) 
	    s16temp = (int16_t)(sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_type)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
	
	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
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
	    s16temp = (int16_t)(sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant - 0.5);
	  else 
	    s16temp = (int16_t)(sample_type)(stemp * (SAMPLE_SPECS::s16_to_st_constant - 1) + 0.5);
	
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
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = 0;
	
	  if (s32temp < 0) target[osize - 2] |=  0x80;
	  //    	  if (osize == 4) printf("neg.target: %x:%x:%x:%x.\n",target[osize-4],
	  //    				 target[osize-3],
	  //    				 target[osize-2],
	  //    				 target[osize-1])
	  break;
	}
      
      case ECA_AUDIO_FORMAT::sfmt_s24_be:
	{
	  int32_t s32temp;
	  if (stemp < 0) 
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s24_to_st_constant - 1) + 0.5);

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
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
	
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
	    s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant - 0.5);
	  else 
	    s32temp = (int32_t)(sample_type)(stemp * (SAMPLE_SPECS::s32_to_st_constant - 1) + 0.5);
  
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
}

/**
 * Fill buffer from external buffer source. 
 * Sample data will be converted to internal sample format 
 * using the given arguments (sample rate, sample format 
 * and endianess).
 *
 * ensure:
 *  channels == channel_count_rep
 */
void SAMPLE_BUFFER::copy_to_buffer(unsigned char* source,
					long int samples_read,
					ECA_AUDIO_FORMAT::Sample_format fmt,
					int ch,
					long int srate) {
  // --------
  // require:
  assert(samples_read >= 0);
  assert(source != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) {
    /*  std::cerr << "(samplebuffer_impl) chcount changes from " << channel_count_rep << " to " << ch << ".\n"; */
    number_of_channels(ch);
  }
  if (buffersize_rep != samples_read) resize(samples_read);

  assert(ch == channel_count_rep);
  assert(buffersize_rep == samples_read);

  unsigned char a[2];
  unsigned char b[4];
  long int isize = 0;

  for(long int osize = 0; osize < buffersize_rep; osize++) {
    for(int c = 0; c < ch; c++) {
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
	  buffer[c][osize] = (sample_type)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
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
	  buffer[c][osize] = (sample_type)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(float*)b);
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
	  buffer[c][osize] = (sample_type)(*(float*)b);
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
 * ensure:
 *  channels == channel_count_rep
 */
void SAMPLE_BUFFER::copy_to_buffer_vector(unsigned char* source,
						  long int samples_read,
						  ECA_AUDIO_FORMAT::Sample_format fmt,
						  int ch,
						  long int srate) {
  // --------
  // require:
  assert(samples_read >= 0);
  assert(source != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) {
    /*  std::cerr << "(samplebuffer_impl) chcount changes from " << channel_count_rep << " to " << ch << ".\n"; */
    number_of_channels(ch);
  }
  if (buffersize_rep != samples_read) resize(samples_read);

  assert(ch == channel_count_rep);
  assert(buffersize_rep == samples_read);

  unsigned char a[2];
  unsigned char b[4];

  long int isize = 0;
  for(int c = 0; c < ch; c++) {
    for(long int osize = 0; osize < buffersize_rep; osize++) {
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
	  buffer[c][osize] = (sample_type)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(int16_t*)a) / SAMPLE_SPECS::s16_to_st_constant;
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
	  buffer[c][osize] = (sample_type)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)((*(int32_t*)b) << 8) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(int32_t*)b) / SAMPLE_SPECS::s32_to_st_constant;
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
	  buffer[c][osize] = (sample_type)(*(float*)b);
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
	  buffer[c][osize] = (sample_type)(*(float*)b);
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

void SAMPLE_BUFFER::resample_nofilter(long int from, 
				      long int to) {
  double step = (double)to / from;
  long int old_buffer_size = buffersize_rep;
  buffersize_rep = static_cast<long int>(step * buffersize_rep);

  if (impl_repp->old_buffer_repp == 0) 
    impl_repp->old_buffer_repp = new sample_type [reserved_bytes_rep * sizeof(sample_type)];

  for(int c = 0; c < channel_count_rep; c++) {
    memcpy(impl_repp->old_buffer_repp, buffer[c], old_buffer_size * sizeof(sample_type));

    if (buffersize_rep > reserved_bytes_rep) {
      reserved_bytes_rep = buffersize_rep;
      delete[] buffer[c];
      buffer[c] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    }
    
    double counter = 0.0;
    long int new_buffer_index = 0;
    long int interpolate_index = 0;
     
    buffer[c][0] = impl_repp->old_buffer_repp[0];
    for(long int old_buffer_index = 1; old_buffer_index < old_buffer_size; old_buffer_index++) {
      counter += step;
      if (step <= 1) {
	if (counter >= new_buffer_index + 1) {
	  new_buffer_index++;
	  if (new_buffer_index >= buffersize_rep) break;
	  buffer[c][new_buffer_index] = impl_repp->old_buffer_repp[old_buffer_index];
	}
      }
      else {
	new_buffer_index = static_cast<long int>(ceil(counter));
	if (new_buffer_index >= buffersize_rep) new_buffer_index = buffersize_rep - 1;
	for(long int t = interpolate_index + 1; t < new_buffer_index; t++) {
	  buffer[c][t] = impl_repp->old_buffer_repp[old_buffer_index - 1] + ((impl_repp->old_buffer_repp[old_buffer_index]
									      - impl_repp->old_buffer_repp[old_buffer_index-1])
									     * static_cast<SAMPLE_BUFFER::sample_type>(t - interpolate_index)
									     / (new_buffer_index - interpolate_index));
	}
	buffer[c][new_buffer_index] = impl_repp->old_buffer_repp[old_buffer_index];
      }
      interpolate_index = new_buffer_index;
    }
  }
}

void SAMPLE_BUFFER::resample_with_memory(long int from, 
						 long int to) {
  double step = (double)to / from;
  long int old_buffer_size = buffersize_rep;
  buffersize_rep = static_cast<long int>(step * buffersize_rep);
  impl_repp->resample_memory_rep.resize(channel_count_rep, SAMPLE_SPECS::silent_value);

  if (impl_repp->old_buffer_repp == 0) 
    impl_repp->old_buffer_repp = new sample_type [reserved_bytes_rep * sizeof(sample_type)];

  for(int c = 0; c < channel_count_rep; c++) {
    memcpy(impl_repp->old_buffer_repp, buffer[c], old_buffer_size * sizeof(sample_type));

    if (buffersize_rep > reserved_bytes_rep) {
      reserved_bytes_rep = buffersize_rep;
      delete[] buffer[c];
      buffer[c] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    }
    
    double counter = 0.0;
    long int new_buffer_index = 0;
    long int interpolate_index = -1;
    sample_type from_point;

    for(long int old_buffer_index = 0; old_buffer_index < old_buffer_size; old_buffer_index++) {
      counter += step;
      if (step <= 1) {
	if (counter >= new_buffer_index + 1) {
	  new_buffer_index++;
	  if (new_buffer_index >= buffersize_rep) break;
	  buffer[c][new_buffer_index] = impl_repp->old_buffer_repp[old_buffer_index];
	}
      }
      else {
	new_buffer_index = static_cast<long int>(ceil(counter));
	if (old_buffer_index == 0) from_point = impl_repp->resample_memory_rep[c];
	else from_point = impl_repp->old_buffer_repp[old_buffer_index-1];
	if (new_buffer_index >= buffersize_rep) new_buffer_index = buffersize_rep - 1;
	for(long int t = interpolate_index + 1; t < new_buffer_index; t++) {
	  buffer[c][t] = from_point + ((impl_repp->old_buffer_repp[old_buffer_index]
					- from_point)
				       * static_cast<SAMPLE_BUFFER::sample_type>(t - interpolate_index)
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
