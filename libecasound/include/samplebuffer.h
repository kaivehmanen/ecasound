#ifndef _SAMPLEBUFFER_H
#define _SAMPLEBUFFER_H

#include <config.h>

#include <vector>
#include <sys/types.h>

#include "eca-audio-format.h"
#include "sample-specs.h"

#include "eca-debug.h"
#include "eca-error.h"

/**
 * Represents a buffer of samples. The primary goal of this class is to 
 * provide a reasonably efficient implementation while still hiding the
 * the actual type information.
 */
template<class T>
class SAMPLE_BUFFER_BASE {

  friend class SAMPLE_ITERATOR;
  friend class SAMPLE_ITERATOR_CHANNEL;
  friend class SAMPLE_ITERATOR_CHANNELS;
  friend class SAMPLE_ITERATOR_INTERLEAVED;

 public:

  typedef int channel_sizet_t;
  typedef long int buf_sizet_t;
  typedef long int srate_sizet_t;

  typedef T sample_type;

 private:

  // ---
  // Sample buffer data - only these variables (+ sample data) are processed when copying 
  // and contructing buffer objects
  // ---

  int channel_count_rep;
  long int buffersize_rep;
  long int sample_rate_rep;
  long int reserved_bytes_rep;

 public:

  /**
   * WARNING! Although 'buffer' is a public member, you should only 
   * use it directly for a very, very good reason. All normal 
   * input/output should be done via the SAMPLEBUFFER_ITERATORS 
   * class. Representation of 'buffer' may change at any time, 
   * and this will break all code using direct-access.
   */
  vector<sample_type*> buffer;

 private:

  // ---
  // Other member variables
  // ---
  sample_type* old_buffer; // for resampling

  void resize(long int buffersize);

 public:
    
  void resample_from(long int from_srate);
  void resample_to(long int to_srate);

 private:

  void resample_extfilter(long int from_srate,
			  long int to_srate);
  void resample_simplefilter(long int from_srate,
			     long int to_srate);
  void resample_nofilter(long int from_srate,
			 long int to_srate);

 public:
    
  // ---
  // Public member routines
  // ---
  
  /**
   * Channel-wise addition. Buffer length is increased if necessary.
   */
  void add(const SAMPLE_BUFFER_BASE<T>& x);

  /**
   * Channel-wise, weighted addition. Before addition every sample is 
   * multiplied by '1/weight'. Buffer length is increased if necessary.
   */
  void add_with_weight(const SAMPLE_BUFFER_BASE<T>& x, int weight);

  /**
   * Channel-wise copy. Buffer length is increased if necessary.
   */
  void copy(const SAMPLE_BUFFER_BASE<T>& x);

  /**
   * Ranged channel-wise copy. Copies samples in range 
   * 'start_pos' - 'end_pos' from buffer 'x' to current 
   * buffer and position 'to_pos'. 
   */
  void copy_range(const SAMPLE_BUFFER_BASE<T>& x, long int start_pos, long int end_pos, long int to_pos);

  /**
   * Divide all samples by 'dvalue'.
   */
  void divide_by(sample_type dvalue);

  /**
   * Limit all samples to valid values. 
   */
  void limit_values(void);

  /**
   * Mute the whole buffer.
   */
  void make_silent(void);

  /**
   * Mute a range of samples.
   */
  void make_silent_range(long int start_pos, long int end_pos);

  /**
   * Fill buffer from external buffer source. 
   * Sample data will be converted to internal sample format 
   * using the given arguments (sample rate, sample format 
   * and endianess).
   *
   * ensure:
   *  channels == channel_count_rep
   */
  void copy_to_buffer(unsigned char* source,
		      long int samples,
		      ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
		      int ch,
		      long int srate) throw(ECA_ERROR*);

  /**
   * Copy contents of sample buffer to 'target'. Sample data 
   * will be converted according to the given arguments
   * (sample rate, sample format and endianess).
   *
   * ensure:
   *  channels == channel_count_rep
   */
  void copy_from_buffer(unsigned char* target,
			ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
			int ch,
			long int srate) throw(ECA_ERROR*);
        
  // ---
  // Info about the audio content 
  // ---
  
  sample_type average_volume(void);
  sample_type average_RMS_volume(void);
  sample_type average_volume(int channel, int count_samples = 0);
  sample_type average_RMS_volume(int channel, int count_samples = 0);
  sample_type max_value(int channel);
  sample_type min_value(int channel);
  sample_type amplitude(void);

  // ---
  // Buffer setup
  // ---

 public:

  void number_of_channels(int len);
  inline int number_of_channels(void) const { return(channel_count_rep); }

  void sample_rate(long int srate) { sample_rate_rep = srate; }
  inline long int sample_rate(void) const { return(sample_rate_rep); }
  void length_in_samples(long int len);
  inline long int length_in_samples(void) const { return(buffersize_rep); }
  inline double length_in_seconds(void) const { return((double)buffersize_rep / sample_rate_rep); }

  // ---
  // Constructors/destructors
  // ---
  SAMPLE_BUFFER_BASE<T>& operator= (const SAMPLE_BUFFER_BASE<T>& t);
  SAMPLE_BUFFER_BASE (long int buffersize = 0,
		      int channels = 0,
		      long int sample_rate = 0);
  ~SAMPLE_BUFFER_BASE(void);
  SAMPLE_BUFFER_BASE (const SAMPLE_BUFFER_BASE<T>& x);
};

// typedef SAMPLE_BUFFER_BASE<SAMPLE_SPECS::sample_type> SAMPLE_BUFFER;

/**
 * Represents a buffer of samples. The primary goal of this class is to 
 * provide a reasonably efficient implementation while still hiding the
 * the actual type information.
 */
class SAMPLE_BUFFER_OBSOLETE {

  friend class SAMPLE_ITERATOR;
  friend class SAMPLE_ITERATOR_CHANNEL;
  friend class SAMPLE_ITERATOR_CHANNELS;
  friend class SAMPLE_ITERATOR_INTERLEAVED;

 public:

  typedef int channel_sizet_t;
  typedef long int buf_sizet_t;
  typedef long int srate_sizet_t;

  typedef vector<vector<SAMPLE_SPECS::sample_type> >::iterator buf_channel_iter_t;
  typedef vector<vector<SAMPLE_SPECS::sample_type> >::const_iterator buf_channel_citer_t;

  typedef vector<SAMPLE_SPECS::sample_type>::iterator buf_sample_iter_t;
  typedef vector<SAMPLE_SPECS::sample_type>::const_iterator buf_sample_citer_t;

 private:

  // ---
  // Sample buffer data (only these three variables processed when copying and contructing
  // buffer objects)
  // ---

  int channel_count_rep;
  long int buffersize_rep;
  long int sample_rate_rep;
  vector<vector<SAMPLE_SPECS::sample_type> > buffer;

  // ---
  // Other member variables
  // ---
  vector<SAMPLE_SPECS::sample_type> old_buffer; // for resampling

  void resize(long int buffersize);

 public:
    
  void resample_from(long int from_srate);
  void resample_to(long int to_srate);

 private:

  void resample_extfilter(long int from_srate,
			  long int to_srate);
  void resample_simplefilter(long int from_srate,
			     long int to_srate);
  void resample_nofilter(long int from_srate,
			 long int to_srate);

 public:
    
  // ---
  // Public member routines
  // ---
  
  /**
   * Channel-wise addition. Buffer length is increased if necessary.
   */
  void add(const SAMPLE_BUFFER_OBSOLETE& x);

  /**
   * Channel-wise, weighted addition. Before addition every sample is 
   * multiplied by '1/weight'. Buffer length is increased if necessary.
   */
  void add_with_weight(const SAMPLE_BUFFER_OBSOLETE& x, int weight);

  /**
   * Channel-wise copy. Buffer length is increased if necessary.
   */
  void copy(const SAMPLE_BUFFER_OBSOLETE& x);

  /**
   * Ranged channel-wise copy. Copies samples in range 
   * 'start_pos' - 'end_pos' from buffer 'x' to current 
   * buffer and position 'to_pos'. 
   */
  void copy_range(const SAMPLE_BUFFER_OBSOLETE& x, long int start_pos, long int end_pos, long int to_pos);

  /**
   * Divide all samples by 'dvalue'.
   */
  void divide_by(SAMPLE_SPECS::sample_type dvalue);

  /**
   * Limit all samples to valid values. 
   */
  void limit_values(void);

  /**
   * Mute the whole buffer.
   */
  void make_silent(void);

  /**
   * Mute a range of samples.
   */
  void make_silent_range(long int start_pos, long int end_pos);

  /**
   * Fill buffer from external buffer source. 
   * Sample data will be converted to internal sample format 
   * using the given arguments (sample rate, sample format 
   * and endianess).
   *
   * ensure:
   *  channels == channel_count_rep
   */
  void copy_to_buffer(unsigned char* source,
		      long int samples,
		      ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
		      int ch,
		      long int srate) throw(ECA_ERROR*);

  /**
   * Copy contents of sample buffer to 'target'. Sample data 
   * will be converted according to the given arguments
   * (sample rate, sample format and endianess).
   *
   * ensure:
   *  channels == channel_count_rep
   */
  void copy_from_buffer(unsigned char* target,
			ECA_AUDIO_FORMAT::SAMPLE_FORMAT fmt,
			int ch,
			long int srate) throw(ECA_ERROR*);
        
  // ---
  // Info about the audio content 
  // ---
  
  SAMPLE_SPECS::sample_type average_volume(void);
  SAMPLE_SPECS::sample_type average_RMS_volume(void);
  SAMPLE_SPECS::sample_type average_volume(int channel, int count_samples = 0);
  SAMPLE_SPECS::sample_type average_RMS_volume(int channel, int count_samples = 0);
  SAMPLE_SPECS::sample_type max_value(int channel);
  SAMPLE_SPECS::sample_type min_value(int channel);
  SAMPLE_SPECS::sample_type amplitude(void);

  // ---
  // Buffer setup
  // ---

 public:

  void number_of_channels(int len);
  inline int number_of_channels(void) const { return(channel_count_rep); }

  void sample_rate(long int srate) { sample_rate_rep = srate; }
  inline long int sample_rate(void) const { return(sample_rate_rep); }
  void length_in_samples(long int len);
  inline long int length_in_samples(void) const { return(buffersize_rep); }
  inline double length_in_seconds(void) const { return((double)buffersize_rep / sample_rate_rep); }

  // ---
  // Constructors/destructors
  // ---

  SAMPLE_BUFFER_OBSOLETE& operator= (const SAMPLE_BUFFER_OBSOLETE& t);
  SAMPLE_BUFFER_OBSOLETE (long int buffersize = 0,
		 int channels = SAMPLE_SPECS::channel_count_default,
		 long int sample_rate = SAMPLE_SPECS::sample_rate_default);
  ~SAMPLE_BUFFER_OBSOLETE (void);
  SAMPLE_BUFFER_OBSOLETE (const SAMPLE_BUFFER_OBSOLETE& x);
};

#ifndef SAMPLEBUFFER_GUARD
typedef SAMPLE_BUFFER_BASE<SAMPLE_SPECS::sample_type> SAMPLE_BUFFER;
// typedef SAMPLE_BUFFER_OBSOLETE SAMPLE_BUFFER;
#endif

template<class T>
SAMPLE_BUFFER_BASE<T>::sample_type SAMPLE_BUFFER_BASE<T>::max_value(int channel) {
  sample_type t = SAMPLE_SPECS::impl_min_value;
  for(long int m = 0; m < buffersize_rep; m++) {
    if (buffer[channel][m] > t) t = buffer[channel][m];
  }
  return(t);
}

template<class T>
SAMPLE_BUFFER_BASE<T>::sample_type SAMPLE_BUFFER_BASE<T>::min_value(int channel) {
  sample_type t = SAMPLE_SPECS::impl_max_value;
  for(long int m = 0; m < buffersize_rep; m++) {
    if (buffer[channel][m] < t) t = buffer[channel][m];
  }
  return(t);
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
SAMPLE_BUFFER_BASE<T>::sample_type SAMPLE_BUFFER_BASE<T>::average_volume(void) {
  sample_type temp_avg = 0.0;
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int m = 0; m < buffersize_rep; m++) {
      temp_avg += fabs(buffer[n][m] - SAMPLE_SPECS::silent_value);
    }
  }

  return(temp_avg / channel_count_rep / buffersize_rep);
}

template<class T>
SAMPLE_BUFFER_BASE<T>::sample_type SAMPLE_BUFFER_BASE<T>::average_RMS_volume(void) {
  sample_type temp_avg = 0.0;
  for(int n = 0; n < channel_count_rep; n++) {
    for(long int m = 0; m < buffersize_rep; m++) {
      temp_avg += buffer[n][m] * buffer[n][m];
    }
  }
  return(sqrt(temp_avg) / channel_count_rep / buffersize_rep);
}

template<class T>
SAMPLE_BUFFER_BASE<T>::sample_type SAMPLE_BUFFER_BASE<T>::average_volume(int channel,
									 int count_samples) 
{
  sample_type temp_avg = 0.0;
  if (count_samples == 0) count_samples = static_cast<int>(channel_count_rep);

  for(long int n = 0; n < buffersize_rep; n++) {
    temp_avg += fabs(buffer[channel][n] - SAMPLE_SPECS::silent_value);
  }

  return(temp_avg / count_samples);
}

template<class T>
SAMPLE_BUFFER_BASE<T>::sample_type SAMPLE_BUFFER_BASE<T>::average_RMS_volume(int channel,
									     int count_samples) 
{
  sample_type temp_avg = 0.0;
  if (count_samples == 0) count_samples = static_cast<int>(channel_count_rep);
  for(long int n = 0; n < buffersize_rep; n++) {
    temp_avg += buffer[channel][n] * buffer[channel][n];
  }
  return(sqrt(temp_avg) / count_samples);
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
    long int newbuf_sizet = 0;
    long int last_sizet = 0;
     
    buffer[c][0] = old_buffer[0];
    for(long int buf_sizet = 1; buf_sizet < old_buffer_size; buf_sizet++) {
      counter += step;
      if (step <= 1) {
	if (counter >= newbuf_sizet + 1) {
	  newbuf_sizet++;
	  if (newbuf_sizet >= buffersize_rep) break;
	  buffer[c][newbuf_sizet] = old_buffer[buf_sizet];
	}
      }
      else {
	newbuf_sizet = static_cast<long int>(ceil(counter));
	if (newbuf_sizet >= buffersize_rep) break;
	for(long int t = last_sizet + 1; t < newbuf_sizet; t++) {
	  buffer[c][t] = old_buffer[buf_sizet - 1] + ((old_buffer[buf_sizet]
						      - old_buffer[buf_sizet-1])
						     * static_cast<SAMPLE_BUFFER_BASE<T>::sample_type>(t - last_sizet)
						     / (newbuf_sizet - last_sizet));
	}
	buffer[c][newbuf_sizet] = old_buffer[buf_sizet];
      }
      last_sizet = newbuf_sizet;
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
    for(int n = 0; n < buffer.size(); n++) {
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
      for(int n = 0; n < buffer.size(); n++) delete[] buffer[n];
      delete[] old_buffer;
      buffer.resize(x.buffer.size());
      for(int n = 0; n < buffer.size(); n++) {
	buffer[n] = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
      }
      old_buffer = new sample_type [reserved_bytes_rep * sizeof(sample_type)];
    }
    buffersize_rep = x.buffersize_rep;
    channel_count_rep = x.channel_count_rep;
    sample_rate_rep = x.sample_rate_rep;
    for(int n = 0; n < buffer.size(); n++) {
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

#endif
