#ifndef INCLUDED_SAMPLEBUFFER_H
#define INCLUDED_SAMPLEBUFFER_H

#include <vector>
#include <sys/types.h>

#include <kvutils/kvu_numtostr.h>
#include "eca-audio-format.h"
#include "sample-specs.h"
#include "eca-debug.h"
#include "eca-error.h"

template<class T> class SAMPLE_BUFFER_FUNCTIONS_BASE;

/**
 * Represents a buffer of samples. The primary goal of this class is to 
 * provide a reasonably efficient implementation while still hiding the
 * the actual type information.
 */
template<class T>
class SAMPLE_BUFFER_BASE {

  friend class SAMPLE_BUFFER_FUNCTIONS_BASE<T>;

  friend class SAMPLE_ITERATOR;
  friend class SAMPLE_ITERATOR_CHANNEL;
  friend class SAMPLE_ITERATOR_CHANNELS;
  friend class SAMPLE_ITERATOR_INTERLEAVED;

 public:

  typedef int channel_size_t;
  typedef long int buf_size_t;
  typedef long int srate_size_t;

  typedef T sample_type;

 public:

  /**
   * WARNING! Although 'buffer' is a public member, you should only 
   * use it directly for a very, very good reason. All normal 
   * input/output should be done via the SAMPLEBUFFER_ITERATORS 
   * class. Representation of 'buffer' may change at any time, 
   * and this will break all code using direct-access.
   */
  vector<sample_type*> buffer;

 public:
    
  void resample_from(long int from_srate) { resample_with_memory(from_srate, sample_rate_rep); }
  void resample_to(long int to_srate) { resample_with_memory(sample_rate_rep, to_srate); }

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
		      ECA_AUDIO_FORMAT::Sample_format fmt,
		      int ch,
		      long int srate) throw(ECA_ERROR&);

  /**
   * Copy contents of sample buffer to 'target'. Sample data 
   * will be converted according to the given arguments
   * (sample rate, sample format and endianess).
   *
   * ensure:
   *  channels == channel_count_rep
   */
  void copy_from_buffer(unsigned char* target,
			ECA_AUDIO_FORMAT::Sample_format fmt,
			int ch,
			long int srate) throw(ECA_ERROR&);
        
  // ---
  // Buffer setup
  // ---

 public:

  void number_of_channels(int len);
  inline int number_of_channels(void) const { return(channel_count_rep); }

  void sample_rate(long int srate) { sample_rate_rep = srate; }
  inline long int sample_rate(void) const { return(sample_rate_rep); }
  void length_in_samples(long int len) { if (buffersize_rep != len) resize(len); }
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

 private:

  // ---
  // Sample buffer data - only these variables (+ sample data) are processed when copying 
  // and contructing buffer objects
  // ---

  int channel_count_rep;
  long int buffersize_rep;
  long int sample_rate_rep;
  long int reserved_bytes_rep;

  // ---
  // Other member variables
  // ---
  sample_type* old_buffer_repp; // for resampling
  vector<sample_type> resample_memory_rep;

  void resize(long int buffersize);

 public:

  void resample_extfilter(long int from_srate,
			  long int to_srate);
  void resample_simplefilter(long int from_srate,
			     long int to_srate);
  void resample_nofilter(long int from_srate,
			 long int to_srate);
  void resample_with_memory(long int from_srate,
			    long int to_srate);
};

typedef SAMPLE_BUFFER_BASE<SAMPLE_SPECS::sample_type> SAMPLE_BUFFER;

#include "samplebuffer_impl.h"

#endif
