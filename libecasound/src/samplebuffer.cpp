// ------------------------------------------------------------------------
// samplebuffer.cpp: Routines and classes for handling sample buffers
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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


template<class T>
void SAMPLE_BUFFER_BASE<T>::make_silent(void) {
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int s = 0; s < buffersize_rep; s++) {
      buffer[n][s] = SAMPLE_SPECS::silent_value;
      ++s;
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::make_silent_range(long int start_pos,
					      long int end_pos) {
  assert(start_pos >= 0);
  assert(end_pos >= 0);

  for(int n = 0; n < channel_count_rep; n++) {
    for(long int s = start_pos; s < end_pos && s < buffersize_rep; s++) {
      buffer[n][s] = SAMPLE_SPECS::silent_value;
      ++s;
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::limit_values(void) {
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int m = 0; m < buffersize_rep; m++) {
      if (buffer[n][m] > SAMPLE_SPECS::impl_max_value) 
	buffer[n][m] = SAMPLE_SPECS::impl_max_value;
      else if (buffer[n][m] < SAMPLE_SPECS::impl_min_value) 
	buffer[n][m] = SAMPLE_SPECS::impl_min_value;
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::divide_by(SAMPLE_BUFFER_BASE<T>::sample_type dvalue) {
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int m = 0; m < buffersize_rep; m++) {
      buffer[n][m] /= dvalue;
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::add(const SAMPLE_BUFFER_BASE<T>& x) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(int q = 0; q != c_count; q++) {
    //    long int s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(long int t = 0; t != length_in_samples(); t++) {
      buffer[q][t] += x.buffer[q][t];
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::add_with_weight(const SAMPLE_BUFFER_BASE<T>& x, int weight) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(int q = 0; q != c_count; q++) {
    //    long int s_count = (buffer[q].size() <= x.buffer[q].size()) ? buffer[q].size() : x.buffer[q].size();
    for(long int t = 0; t != length_in_samples(); t++) {
      buffer[q][t] += x.buffer[q][t] / weight;
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::copy(const SAMPLE_BUFFER_BASE<T>& x) {
  if (x.length_in_samples() >= length_in_samples()) {
    length_in_samples(x.length_in_samples());
  }
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  for(int q = 0; q != c_count; q++) {
    for(long int t = 0; t != length_in_samples(); t++) {
      buffer[q][t] = x.buffer[q][t];
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::copy_range(const SAMPLE_BUFFER_BASE<T>& x, 
			       long int start_pos,
			       long int end_pos,
			       long int to_pos) {
  int c_count = (channel_count_rep <= x.channel_count_rep) ? channel_count_rep : x.channel_count_rep;
  long int t = to_pos;

  assert(start_pos < end_pos);
  assert(to_pos < length_in_samples());

  if (start_pos >= x.length_in_samples()) start_pos = x.length_in_samples();
  if (end_pos >= x.length_in_samples()) end_pos = x.length_in_samples();

  for(int q = 0; q != c_count; q++) {
    for(long int s = start_pos; s != end_pos && t < length_in_samples(); s++) {
      buffer[q][t] = x.buffer[q][s];
      ++t;
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::copy_from_buffer(unsigned char* target,
				     ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
				     int ch,
				     long int srate) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(target != 0);
  assert(ch > 0);
  assert(srate > 0);
  // --------

  if (ch != channel_count_rep) number_of_channels(ch);
  if (srate != sample_rate_rep) resample_to(srate);

  assert(ch == channel_count_rep);

  long int osize = 0;
  for(long int isize = 0; isize < buffersize_rep; isize++) {
    switch (fmt) {
    case ECA_AUDIO_FORMAT::sfmt_u8:
      {
	for(int c = 0; c < ch; c++) {
	  // --- for debugging signal flow
	  //	  printf("(c %u)(isize %u)(osize %u) converting %.2f, \n",
	  //		   c, isize, osize, buffer[c][isize]);
	  sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  target[osize++] = (unsigned
			     char)((sample_type)(stemp / SAMPLE_SPECS::u8_to_st_constant) + SAMPLE_SPECS::u8_to_st_delta);
	  // --- for debugging signal flow
	  //	  printf("converted to u8 %u (hex:%x)\n", target[osize-1], target[osize-1]);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_le:
      {
	for(int c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int16_t s16temp = (int16_t)(sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant);
	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).

	  target[osize++] = (unsigned char)(s16temp & 0xff);
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_be:
      {
	for(int c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int16_t s16temp = (int16_t)(sample_type)(stemp * SAMPLE_SPECS::s16_to_st_constant);

	  // --- for debugging signal flow
	  // if (isize == 0) 
	  //  printf("converted to s16 %d (hex:%x)", s16temp, (unsigned short int)s16temp);
	  // ------------------------------
	  
	  // little endian: (LSB, MSB) (Intel).
	  // big endian: (MSB, LSB) (Motorola).
	  // ---
	  target[osize++] = (unsigned char)((s16temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s16temp & 0xff);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_le:
      {
	for(int c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = 0;

  	  if (s32temp < 0) target[osize - 2] |=  0x80;
//    	  if (osize == 4) printf("neg.target: %x:%x:%x:%x.\n",target[osize-4],
//    				 target[osize-3],
//    				 target[osize-2],
//    				 target[osize-1])
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_be:
      {
	for(int c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s24_to_st_constant);

	  target[osize++] = 0;
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  
	  if (s32temp < 0) target[osize - 3] |= 0x80;
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s32_le:
      {
	for(int c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant);

	  target[osize++] = (unsigned char)(s32temp & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	}
      }
      break;
	
    case ECA_AUDIO_FORMAT::sfmt_s32_be:
      {
	for(int c = 0; c < ch; c++) {
	  sample_type stemp = buffer[c][isize];
	  if (stemp > SAMPLE_SPECS::impl_max_value) stemp = SAMPLE_SPECS::impl_max_value;
	  else if (stemp < SAMPLE_SPECS::impl_min_value) stemp = SAMPLE_SPECS::impl_min_value;
	  int32_t s32temp = (int32_t)(sample_type)(stemp * SAMPLE_SPECS::s32_to_st_constant);

	  target[osize++] = (unsigned char)((s32temp >> 24) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 16) & 0xff);
	  target[osize++] = (unsigned char)((s32temp >> 8) & 0xff);
	  target[osize++] = (unsigned char)(s32temp & 0xff);
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

template<class T>
void SAMPLE_BUFFER_BASE<T>::copy_to_buffer(unsigned char* source,
					   long int samples_read,
					   ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
					   int ch,
					   long int srate) throw(ECA_ERROR*) {
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
  unsigned char b[4];
  long int isize = 0;

  for(long int osize = 0; osize < buffersize_rep; osize++) {
    switch (fmt) {
    case ECA_AUDIO_FORMAT::sfmt_u8: 
      {
	for(int c = 0; c < ch; c++) {
	  // --- for debugging signal flow
	  //	  if (osize == 0) 
	  // printf("converting to u8 %u\n", source[isize]);

	  buffer[c][osize] = (unsigned char)source[isize++];
	  buffer[c][osize] -= SAMPLE_SPECS::u8_to_st_delta;
	  buffer[c][osize] *= SAMPLE_SPECS::u8_to_st_constant;
	  // --- for debugging signal flow
	  //	  if (osize == 0) 
	  //	    printf("converted to %.2f.\n", buffer[c][osize]);
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s16_le:
      {
	for(int c = 0; c < ch; c++) {
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
	for(int c = 0; c < ch; c++) {
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
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_le:
      {
	for(int c = 0; c < ch; c++) {
	  if (SAMPLE_SPECS::is_system_littleendian) {
	    //	    if (osize == 0) cerr << "sisään:" << (*(int32_t*)(source+isize)) << "|\n";
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
	  //	  if (osize == 0) cerr << "sisään3:" << buffer[c][osize] << "|\n";
	}
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s24_be:
      {
	for(int c = 0; c < ch; c++) {
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
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s32_le:
      {
	for(int c = 0; c < ch; c++) {
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
      }
      break;

    case ECA_AUDIO_FORMAT::sfmt_s32_be:
      {
	for(int c = 0; c < ch; c++) {
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
      }
      break;

    default: 
      { 
	throw(new ECA_ERROR("SAMPLEBUFFER", "Unknown sample format! [c_to_b]."));
      }
    }
  }
  if (srate != sample_rate_rep) resample_from(srate);
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::resample_from(long int from_srate) {
  resample_nofilter(from_srate, sample_rate_rep);
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::resample_to(long int to_srate) {
  resample_nofilter(sample_rate_rep, to_srate);
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::resample_nofilter(long int from, 
				      long int to) {
  double step = (double)to / from;
  long int old_buffer_size = buffersize_rep;
  buffersize_rep = static_cast<long int>(step * buffersize_rep);

  for(int c = 0; c < channel_count_rep; c++) {
    memcpy(old_buffer, buffer[c], old_buffer_size * sizeof(sample_type));

    if (buffersize_rep > reserved_bytes_rep) {
      reserved_bytes_rep = buffersize_rep;
      delete[] buffer[c];
      buffer[c] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    }
    
    double counter = 0.0;
    long int newbuf_size = 0;
    long int last_size = 0;
     
    buffer[c][0] = old_buffer[0];
    for(long int buf_size = 1; buf_size < old_buffer_size; buf_size++) {
      counter += step;
      if (step <= 1) {
	if (counter >= newbuf_size + 1) {
	  newbuf_size++;
	  if (newbuf_size >= buffersize_rep) break;
	  buffer[c][newbuf_size] = old_buffer[buf_size];
	}
      }
      else {
	newbuf_size = static_cast<long int>(ceil(counter));
	if (newbuf_size >= buffersize_rep) newbuf_size = buffersize_rep - 1;
	for(long int t = last_size + 1; t < newbuf_size; t++) {
	  buffer[c][t] = old_buffer[buf_size - 1] + ((old_buffer[buf_size]
						      - old_buffer[buf_size-1])
						     * static_cast<SAMPLE_BUFFER_BASE<T>::sample_type>(t - last_size)
						     / (newbuf_size - last_size));
	}
	buffer[c][newbuf_size] = old_buffer[buf_size];
      }
      last_size = newbuf_size;
    }
  }
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::resample_extfilter(long int from_srate,
					long int to_srate) {}

template<class T>
void SAMPLE_BUFFER_BASE<T>::resample_simplefilter(long int from_srate,
					  long int to_srate) { }

template<class T>
void SAMPLE_BUFFER_BASE<T>::length_in_samples(long int len) {
  if (buffersize_rep != len) resize(len); 
}

template<class T>
void SAMPLE_BUFFER_BASE<T>::number_of_channels(int len) {
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

template<class T>
void SAMPLE_BUFFER_BASE<T>::resize(long int buffersize) {
  if (buffersize > reserved_bytes_rep) {
    reserved_bytes_rep = buffersize;
    for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
      delete[] buffer[n];
      buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    }
    delete[] old_buffer;
    old_buffer = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
  }
  buffersize_rep = buffersize;
}

template<class T>
SAMPLE_BUFFER_BASE<T>::SAMPLE_BUFFER_BASE (long int buffersize, int channels, long int srate) 
  : channel_count_rep(channels),
  buffersize_rep(buffersize),
  sample_rate_rep(srate),
  reserved_bytes_rep(buffersize) {

  buffer.resize(channels);
  for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
    buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
  }
  old_buffer = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
 
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer<>) Buffer created, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}

template<class T>
SAMPLE_BUFFER_BASE<T>::~SAMPLE_BUFFER_BASE (void) { 
  for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
    delete[] buffer[n];
  }
  delete[] old_buffer;
}

template<class T>
SAMPLE_BUFFER_BASE<T>& SAMPLE_BUFFER_BASE<T>::operator=(const SAMPLE_BUFFER_BASE<T>& x) {
  // ---
  // For better performance, doesn't copy IO-buffers nor
  // iterator state.
  // ---
  
  if (this != &x) {
    if (x.buffersize_rep > reserved_bytes_rep) {
      reserved_bytes_rep = x.buffersize_rep;
      for(int n = 0; n < static_cast<int>(buffer.size()); n++) delete[] buffer[n];
      delete[] old_buffer;
      buffer.resize(x.buffer.size());
      for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
	buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
      }
      old_buffer = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
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

template<class T>
SAMPLE_BUFFER_BASE<T>::SAMPLE_BUFFER_BASE (const SAMPLE_BUFFER_BASE<T>& x)
  : channel_count_rep(x.channel_count_rep),
					     buffersize_rep(x.buffersize_rep),
					     sample_rate_rep(x.sample_rate_rep),
					     reserved_bytes_rep(x.reserved_bytes_rep) {
  // ---
  // For better performance, doesn't copy IO-buffers.
  // ---
  buffer.resize(x.buffer.size());
  for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
    buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    memcpy(buffer[n], x.buffer[n], buffersize_rep * sizeof(sample_type));
  }
  old_buffer = new sample_type [reserved_bytes_rep * sizeof(sample_type)];

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(samplebuffer<>) Buffer copy-constructed, channels: " +
		kvu_numtostr(buffer.size()) + ", length-samples: " +
		kvu_numtostr(buffersize_rep) + ", sample rate: " +
		kvu_numtostr(sample_rate_rep) + ".");
}

