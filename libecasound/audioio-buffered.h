#ifndef INCLUDED_AUDIOIO_BUFFERED_H
#define INCLUDED_AUDIOIO_BUFFERED_H

#include "audioio.h"

class SAMPLE_BUFFER;

/**
 * A lower level interface for audio I/O objects. Derived classes 
 * must implement routines for reading and/or writing buffers of raw data.
 */
class AUDIO_IO_BUFFERED : public AUDIO_IO {

 public:

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  virtual void buffersize(long int samples, long int sample_rate);
  virtual long int buffersize(void) const { return(buffersize_rep); }

  /**
   * Low-level routine for reading samples. Number of read sample
   * frames is returned. This must be implemented by all subclasses.
   */
  virtual long int read_samples(void* target_buffer, long int sample_frames) = 0;

  /**
   * Low-level routine for writing samples. This must be implemented 
   * by all subclasses.
   */
  virtual void write_samples(void* target_buffer, long int sample_frames) = 0;

  virtual ~AUDIO_IO_BUFFERED(void);
  AUDIO_IO_BUFFERED(void);

 protected:

  void reserve_buffer_space(long int bytes);

 private:

  long int buffersize_rep;
  long int buffersize_sig_rep;
  long int target_srate_rep;
  long int target_samples_rep;
  unsigned char* iobuf_uchar_repp;  // buffer for raw-I/O
  size_t iobuf_size_rep;
};

#endif // INCLUDED_AUDIO_IO_BUFFERED
