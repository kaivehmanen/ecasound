#ifndef _SAMPLEBUFFER_H
#define _SAMPLEBUFFER_H

#include <config.h>

#include <vector>
#include <sys/types.h>

#include "eca-audio-format.h"

class ECA_ERROR;

namespace SAMPLE_SPECS {
  typedef float sample_type; // should be a floating-point value!

  static const long int sample_rate_default = 44100;
  static const int channel_count_default = 2;

  static const sample_type silent_value = 0;     // do not change!
  static const sample_type max_amplitude = 1;
  static const sample_type impl_max_value = silent_value + max_amplitude;
  static const sample_type impl_min_value = silent_value - max_amplitude;

  static const int ch_left = 0;
  static const int ch_right = 1;

#ifdef WORDS_BIGENDIAN
  static const bool is_system_littleendian = false;
#else
  static const bool is_system_littleendian = true;
#endif

  static const sample_type s16_to_st_constant = (32768.0 / max_amplitude); // 2^15
  static const sample_type s24_to_st_constant = (8388607.0 / max_amplitude); // 2^23
  static const sample_type s32_to_st_constant = (2147483647.0 / max_amplitude);  // 2^31
  static const sample_type u8_to_st_delta = 128;
  static const sample_type u8_to_st_constant = (max_amplitude / 128);
}
using namespace SAMPLE_SPECS;

/**
 * Represents a buffer of samples. The primary goal of this class is to 
 * provide a reasonably efficient implementation while still hiding the
 * the actual type information.
 */
class SAMPLE_BUFFER {

  friend class SAMPLE_ITERATOR;
  friend class SAMPLE_ITERATOR_CHANNEL;
  friend class SAMPLE_ITERATOR_CHANNELS;
  friend class SAMPLE_ITERATOR_INTERLEAVED;

 public:

  typedef vector<vector<sample_type> >::iterator buf_channel_iter_t;
  typedef vector<vector<sample_type> >::const_iterator buf_channel_citer_t;
  typedef vector<vector<sample_type> >::size_type buf_channel_size_t;

  typedef vector<sample_type>::iterator buf_sample_iter_t;
  typedef vector<sample_type>::const_iterator buf_sample_citer_t;
  typedef vector<sample_type>::size_type buf_sample_size_t;

  // ---
  // Static members
  // ---

  static long int sample_rate;
  static void set_sample_rate(long int srate);

 private:

  // ---
  // Sample buffer data (only these three variables processed when copying and contructing
  // buffer objects)
  // ---

  buf_channel_size_t channel_count_rep;
  buf_sample_size_t buffersize_rep;
  vector<vector<sample_type> > buffer;

  // ---
  // Other member variables
  // ---

  vector<sample_type> old_buffer; // for resampling

  void resize(long int buffersize);


 public:
    
  void resample_from(unsigned int long from_srate);
  void resample_to(unsigned int long to_srate);

 private:

  void resample_extfilter(unsigned int long from_srate,
			  unsigned int long to_srate);
  void resample_simplefilter(unsigned int long from_srate,
			     unsigned int long to_srate);
  void resample_nofilter(unsigned int long from_srate,
			 unsigned int long to_srate);

 public:
    
  // ---
  // Public member routines
  // ---
  
  /**
   * Channel-wise addition. Buffer length is increased if necessary.
   */
  void add(const SAMPLE_BUFFER& x);

  /**
   * Channel-wise, weighted addition. Before addition every sample is 
   * multiplied by '1/weight'. Buffer length is increased if necessary.
   */
  void add_with_weight(const SAMPLE_BUFFER& x, int weight);

  /**
   * Channel-wise copy. Buffer length is increased if necessary.
   */
  void copy(const SAMPLE_BUFFER& x);

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
  void length_in_samples(int len);
  inline int length_in_samples(void) const { return(buffersize_rep); }
  inline double length_in_seconds(void) const { return((double)buffersize_rep / sample_rate); }

  // ---
  // Constructors/destructors
  // ---

  SAMPLE_BUFFER& operator= (const SAMPLE_BUFFER& t);
  SAMPLE_BUFFER (long int buffersize, int channels);
  SAMPLE_BUFFER (void);
  ~SAMPLE_BUFFER (void);
  SAMPLE_BUFFER (const SAMPLE_BUFFER& x) throw(ECA_ERROR*);
};

#endif
