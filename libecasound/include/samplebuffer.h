#ifndef _SAMPLEBUFFER_H
#define _SAMPLEBUFFER_H

#include <config.h>

#include <vector>
#include <sys/types.h>

#include "eca-audio-format.h"
#include "sample-specs.h"

class ECA_ERROR;

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

  typedef int channel_sizet_t;
  typedef long int buf_sizet_t;
  typedef long int srate_sizet_t;

  typedef vector<vector<sample_type> >::iterator buf_channel_iter_t;
  typedef vector<vector<sample_type> >::const_iterator buf_channel_citer_t;

  typedef vector<sample_type>::iterator buf_sample_iter_t;
  typedef vector<sample_type>::const_iterator buf_sample_citer_t;

 private:

  // ---
  // Sample buffer data (only these three variables processed when copying and contructing
  // buffer objects)
  // ---

  int channel_count_rep;
  long int buffersize_rep;
  long int sample_rate_rep;
  vector<vector<sample_type> > buffer;

  // ---
  // Other member variables
  // ---
  vector<sample_type> old_buffer; // for resampling

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
   * Ranged channel-wise copy. Copies samples in range 
   * 'start_pos' - 'end_pos' from buffer 'x' to current 
   * buffer and position 'to_pos'. 
   */
  void copy_range(const SAMPLE_BUFFER& x, long int start_pos, long int end_pos, long int to_pos);

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

  SAMPLE_BUFFER& operator= (const SAMPLE_BUFFER& t);
  SAMPLE_BUFFER (long int buffersize = 0,
		 int channels = SAMPLE_SPECS::channel_count_default,
		 long int sample_rate = SAMPLE_SPECS::sample_rate_default);
  ~SAMPLE_BUFFER (void);
  SAMPLE_BUFFER (const SAMPLE_BUFFER& x);
};

#endif
