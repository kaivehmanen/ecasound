// ------------------------------------------------------------------------
// samplebuffer.cpp: Routines and classes for handling sample buffers.
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

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sys/types.h>

#include <kvutils.h>

#include "samplebuffer.h"
#include "eca-debug.h"
#include "eca-error.h"

long int SAMPLE_BUFFER::sample_rate = SAMPLE_BUFFER::sample_rate_default;

void SAMPLE_BUFFER::set_sample_rate(long int srate) {
  SAMPLE_BUFFER::sample_rate = srate;
}

SAMPLE_BUFFER::sample_type SAMPLE_BUFFER::max_value(int channel) {
  return(*max_element(buffer[channel].begin(), buffer[channel].end()));
}

SAMPLE_BUFFER::sample_type SAMPLE_BUFFER::min_value(int channel) {
  return(*min_element(buffer[channel].begin(), buffer[channel].end()));
}

void SAMPLE_BUFFER::limit_values(void) {
  buf_channel_iter_t c = buffer.begin();
  while(c != buffer.end()) {
    buf_sample_iter_t s = c->begin();
    while(s != c->end()) {
      if ((*s) > impl_max_value)
	(*s) = impl_max_value;
      else if ((*s) < impl_min_value) 
	(*s) = impl_min_value;
      ++s;
    }
    ++c;
  }
}

void SAMPLE_BUFFER::copy_from_buffer(unsigned char* target,
				     ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
				     int ch,
				     long int srate) {
  // --------
  // require:
  assert(target != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) number_of_channels(ch);
  if (srate != SAMPLE_BUFFER::sample_rate) resample_to(srate);

  assert(ch == channel_count_rep);

  buf_sample_size_t osize = 0;

  for(buf_sample_size_t isize = 0; isize < buffersize_rep; isize++) {
    switch (fmt) {
    case ECA_AUDIO_FORMAT::sfmt_s16_le:
      {
	for(buf_channel_size_t c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > impl_max_value) stemp = impl_max_value;
	  else if (stemp < impl_min_value) stemp = impl_min_value;
	  int16_t s16temp = (int16_t)(sample_type)(stemp / s16_to_st_constant);
	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	  // ---
	  if (SAMPLE_BUFFER::is_system_littleendian) {
	    target[osize++] = (unsigned char)(s16temp & 0xff);
	    target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  }
	  else {
	    target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	    target[osize++] = (unsigned char)(s16temp & 0xff);
	  }
	}
	break;
      }

    case ECA_AUDIO_FORMAT::sfmt_s16_be:
      {
	for(buf_channel_size_t c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > impl_max_value) stemp = impl_max_value;
	  else if (stemp < impl_min_value) stemp = impl_min_value;
	  int16_t s16temp = (int16_t)(sample_type)(stemp / s16_to_st_constant);

	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	  // ---
	  if (!SAMPLE_BUFFER::is_system_littleendian) {
	    target[osize++] = (unsigned char)(s16temp & 0xff);
	    target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  }
	  else {
	    target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	    target[osize++] = (unsigned char)(s16temp & 0xff);
	  }
	}
	break;
      }

    case ECA_AUDIO_FORMAT::sfmt_u8:
      {
	for(buf_channel_size_t c = 0; c < ch; c++) {
	  // --- for debugging signal flow
	  //	  printf("(c %u)(isize %u)(osize %u) converting %.2f, \n",
	  //		   c, isize, osize, buffer[c][isize]);
	  target[osize++] = (unsigned
			     char)((sample_type)(buffer[c][isize] / u8_to_st_constant) + u8_to_st_delta);
	  // --- for debugging signal flow
	  //	  printf("converted to u8 %u (hex:%x)\n", target[osize-1], target[osize-1]);
	}
      }
      break;
      
    default: 
      { 
	throw(new ECA_ERROR("SAMPLEBUFFER", "Unknown sample format! [c_to_b]."));
   
      }
    }
  }
}

void SAMPLE_BUFFER::copy_to_buffer(unsigned char* source,
				   long int samples_read,
				   ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
				   int ch,
				   long int srate) {
  // --------
  // require:
  assert(samples_read >= 0);
  assert(source != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) number_of_channels(ch);
  if (buffersize_rep != samples_read) resize(samples_read);

  assert(ch == channel_count_rep);
  assert(buffersize_rep == samples_read);

  unsigned char a[2];
  buf_sample_size_t isize = 0;

  for(buf_sample_size_t osize = 0; osize < buffersize_rep; osize++) {
    switch (fmt) {
    case ECA_AUDIO_FORMAT::sfmt_s16_le:
      {
	for(buf_channel_size_t c = 0; c < ch; c++) {
	  // little endian: (LSB, MSB) (Intel)
	  // big endian: (MSB, LSB) (Motorola)
	  if (SAMPLE_BUFFER::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_type)(*(int16_t*)a) * s16_to_st_constant;
	  // --- for debugging signal flow
	  //  	  if (osize == 0) {
	  //  	    printf(" ... converted to %d (hex:%x)...", (*(int16_t*)a), (*(int16_t*)a)); 
	  //  	    printf(" (a0: %d, a1: %d) ", a[0], a[1]);
	  //  	    printf(" ... and scaled to %.2f.\n", buffer[c][osize]);
	  //  	  }
	  // ------------------------------
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_be:
      {
	for(buf_channel_size_t c = 0; c < ch; c++) {
	  if (!SAMPLE_BUFFER::is_system_littleendian) {
	    a[0] = source[isize++];
	    a[1] = source[isize++];
	  }
	  else {
	    a[1] = source[isize++];
	    a[0] = source[isize++];
	  }
	  buffer[c][osize] = (sample_type)(*(int16_t*)a) * s16_to_st_constant;
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_u8: 
      {
	for(buf_channel_size_t c = 0; c < ch; c++) {
	  // --- for debugging signal flow
	  //	  if (osize == 0) 
	  // printf("converting to u8 %u\n", source[isize]);

	  buffer[c][osize] = (unsigned char)source[isize++];
	  buffer[c][osize] -= u8_to_st_delta;
	  buffer[c][osize] *= u8_to_st_constant;
	  // --- for debugging signal flow
	  //	  if (osize == 0) 
	  //	    printf("converted to %.2f.\n", buffer[c][osize]);
	}
      }
      break;

    default: 
      { 
	throw(new ECA_ERROR("SAMPLEBUFFER", "Unknown sample format! [c_to_b]."));
      }
    }
  }
  if (srate != SAMPLE_BUFFER::sample_rate) resample_from(srate);
}

void SAMPLE_BUFFER::make_silent(void) {
  buf_channel_iter_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_iter_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      *p = SAMPLE_BUFFER::silent_value;
      ++p;
    }
    ++buf_iter;
  }
}

void SAMPLE_BUFFER::divide_by(SAMPLE_BUFFER::sample_type dvalue) {
  buf_channel_iter_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_iter_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      *p /= dvalue;
      ++p;
    }
    ++buf_iter;
  }
}

void SAMPLE_BUFFER::add(const SAMPLE_BUFFER& x) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  buf_channel_size_t c_count = (number_of_channels() <= x.number_of_channels()) ? number_of_channels() : x.number_of_channels();
  for(buf_channel_size_t q = 0; q != c_count; q++) {
    //    buf_sample_size_t s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(buf_sample_size_t t = 0; t != x.buffer[q].size(); t++) {
      buffer[q][t] += x.buffer[q][t];
    }
  }
}

void SAMPLE_BUFFER::add_with_weight(const SAMPLE_BUFFER& x, int weight) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  buf_channel_size_t c_count = (number_of_channels() <= x.number_of_channels()) ? number_of_channels() : x.number_of_channels();
  for(buf_channel_size_t q = 0; q != c_count; q++) {
    //    buf_sample_size_t s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(buf_sample_size_t t = 0; t != x.buffer[q].size(); t++) {
      buffer[q][t] += x.buffer[q][t] / weight;
    }
  }
}

SAMPLE_BUFFER::sample_type SAMPLE_BUFFER::average_volume(void) {
  sample_type temp_avg = 0.0;
  buf_channel_citer_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_citer_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      temp_avg += fabs((*p) - SAMPLE_BUFFER::silent_value);
      ++p;
    }
    ++buf_iter;
  }

  return(temp_avg / (sample_type)number_of_channels());
}

SAMPLE_BUFFER::sample_type SAMPLE_BUFFER::average_RMS_volume(void) {
  sample_type temp_avg = 0.0;
  buf_channel_citer_t buf_iter = buffer.begin();
  while(buf_iter != buffer.end()) {
    buf_sample_citer_t p = buf_iter->begin();
    while(p != buf_iter->end()) {
      temp_avg += (*p) * (*p);
      ++p;
    }
    ++buf_iter;
  }
  return(sqrt(temp_avg / (sample_type)number_of_channels()));
}

SAMPLE_BUFFER::sample_type SAMPLE_BUFFER::average_volume(int channel,
							 int count_samples) 
{
  sample_type temp_avg = 0.0;
  if (count_samples == 0) count_samples = (int)number_of_channels();

  buf_sample_citer_t p = buffer[channel].begin();
  while(p != buffer[channel].end()) {
    temp_avg += fabs(*p - SAMPLE_BUFFER::silent_value);
    ++p;
  }

  return(temp_avg / (sample_type)count_samples);
}

SAMPLE_BUFFER::sample_type SAMPLE_BUFFER::average_RMS_volume(int channel,
							     int count_samples) 
{
  sample_type temp_avg = 0.0;
  if (count_samples == 0) count_samples = (int)number_of_channels();
  buf_sample_citer_t p = buffer[channel].begin();
  while(p != buffer[channel].end()) {
    temp_avg += *p + *p;
    ++p;
  }
}

void SAMPLE_BUFFER::resample_from(unsigned int long from_srate) {
  resample_nofilter(from_srate, SAMPLE_BUFFER::sample_rate);
}

void SAMPLE_BUFFER::resample_to(unsigned int long to_srate) {
  resample_nofilter(SAMPLE_BUFFER::sample_rate, to_srate);
}

void SAMPLE_BUFFER::resample_nofilter(unsigned int long from, 
				      unsigned int long to) {
  double step = (double)to / from;
  buffersize_rep *= step ;

  for(int c = 0; c < channel_count_rep; c++) {
    old_buffer = buffer[c];
    
    buffer[c].resize(buffersize_rep);
    
    double counter = 0.0;
    size_t newbuf_sizet = 0;
    size_t last_sizet = 0;
    
    if (old_buffer.size() == 0) continue;
    
    buffer[c][0] = old_buffer[0];
    for(buf_sample_size_t buf_sizet = 1; buf_sizet < old_buffer.size(); buf_sizet++) {
      counter += step;
      if (step <= 1) {
	if (counter >= newbuf_sizet + 1) {
	  newbuf_sizet++;
	  if (newbuf_sizet >= buffer[c].size()) break;
	  buffer[c][newbuf_sizet] = old_buffer[buf_sizet];
	  //  	buffer[c][newbuf_sizet] *= 0.5;
	  //  	buffer[c][newbuf_sizet] += (buffer[c][newbuf_sizet-1] * 0.5);
	}
      }
      else {
	newbuf_sizet = (size_t)ceil(counter);
	if (newbuf_sizet >= buffer[c].size()) break;
	for(size_t t = last_sizet + 1; t < newbuf_sizet; t++) {
	  buffer[c][t] = old_buffer[buf_sizet - 1] + ((old_buffer[buf_sizet]
						      - old_buffer[buf_sizet-1])
						     * static_cast<SAMPLE_BUFFER::sample_type>(t - last_sizet)
						     / (newbuf_sizet - last_sizet));
	  //  	buffer[c][t] *= 0.5;
	  //  	buffer[c][t] += (buffer[c][t-1] * 0.5);
	}
	buffer[c][newbuf_sizet] = old_buffer[buf_sizet];
      }
//        buffer[c][newbuf_sizet] *= 0.5;
//        buffer[c][newbuf_sizet] += (buffer[c][newbuf_sizet-1] * 0.5);

      last_sizet = newbuf_sizet;
    }
  }
}

void SAMPLE_BUFFER::resample_extfilter(unsigned int long from_srate,
					unsigned int long to_srate) {}
void SAMPLE_BUFFER::resample_simplefilter(unsigned int long
					  from_srate,
					  unsigned int long to_srate) { }

void SAMPLE_BUFFER::length_in_samples(int len) {
  if (buffersize_rep != len) resize(len); 
}

void SAMPLE_BUFFER::number_of_channels(int len) {
  if (len > number_of_channels()) {
    buffer.resize(len, vector<sample_type> (buffersize_rep,
					    sample_type(0.0)));
  }
  channel_count_rep = len;
}

void SAMPLE_BUFFER::resize(long int buffersize) {
  buf_channel_iter_t p = buffer.begin();
  while(p != buffer.end()) {
    p->resize(buffersize);
    ++p;
  }
  buffersize_rep = buffersize;
}

SAMPLE_BUFFER::SAMPLE_BUFFER (long int buffersize, int channels) 
  : buffer(channels, vector<SAMPLE_BUFFER::sample_type> (buffersize, sample_type(0.0))),
    buffersize_rep(buffersize),
    channel_count_rep(channels) {

  MESSAGE_ITEM mitem;
  mitem << "Created a sample_buffer with " << number_of_channels() << " channels";
  mitem << " that are " << buffersize << " samples long.";
  ecadebug->msg(1, mitem.to_string());
}

SAMPLE_BUFFER::SAMPLE_BUFFER (void) 
  : buffer(0, vector<SAMPLE_BUFFER::sample_type> (0)),
    buffersize_rep(0),
    channel_count_rep(0) {

  ecadebug->msg(1, "Created a empty sample_buffer.");
}

SAMPLE_BUFFER::~SAMPLE_BUFFER (void) {
  buffer.clear();
}

SAMPLE_BUFFER& SAMPLE_BUFFER::operator=(const SAMPLE_BUFFER& x) {
  // ---
  // For better performance, doesn't copy IO-buffers nor
  // iterator state.
  // ---
  
  if (this != &x) {
    //    if (number_of_channels() != x.number_of_channels()) resize(x.number_of_channels());
    buffer = x.buffer;
    buffersize_rep = x.buffersize_rep;
    channel_count_rep = x.channel_count_rep;

    return *this;
  }
}

SAMPLE_BUFFER::SAMPLE_BUFFER (const SAMPLE_BUFFER& x) :
  buffer(x.buffer), 
  buffersize_rep(x.buffersize_rep),
  channel_count_rep(x.channel_count_rep)
{
  // ---
  // For better performance, doesn't copy IO-buffers.
  // ---

  if (x.number_of_channels() == 0)
    throw(new ECA_ERROR("SAMPLEBUFFER", "Tried to construct an empty buffer."));

  MESSAGE_ITEM mitem;
  mitem << "Created a sample_buffer with " << number_of_channels() << " channels";
  mitem << " that are " << buffer[0].size() << " samples long.";
  ecadebug->msg(1, mitem.to_string());
}
