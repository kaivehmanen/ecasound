#ifndef INCLUDED_AUDIOIO_TYPES_H
#define INCLUDED_AUDIOIO_TYPES_H

#include "audioio.h"

/**
 * Virtual base for buffered audio devices
 */
class AUDIO_IO_BUFFERED : public AUDIO_IO {

 public:

  void read_buffer(SAMPLE_BUFFER* sbuf);
  void write_buffer(SAMPLE_BUFFER* sbuf);

  void buffersize(long int samples, long int sample_rate);
  long int buffersize(void) const { return(buffersize_rep); }

  /**
   * Low-level routine for reading samples. Number of read samples
   * is returned. This must be implemented by all subclasses.
   */
  virtual long int read_samples(void* target_buffer, long int samples) = 0;

  /**
   * Low-level routine for writing samples. This must be implemented 
   * by all subclasses.
   */
  virtual void write_samples(void* target_buffer, long int samples) = 0;

  virtual ~AUDIO_IO_BUFFERED(void);
  AUDIO_IO_BUFFERED(void);

 protected:

  void reserve_buffer_space(long int bytes);

 private:

  long int buffersize_rep;
  long int target_srate_rep;
  long int target_samples_rep;
  unsigned char* iobuf_uchar_repp;  // buffer for raw-I/O
  size_t iobuf_size_rep;
};

/**
 * Virtual base class for real-time devices.
 *
 * A realtime device...
 *
 * - is disabled after device is opened
 *
 * - is enabled with start()
 *
 * - once enabled, will handle I/O at a constant speed
 *   based on the sample format paremeters
 *
 * - is disabled with stop()
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_DEVICE : public AUDIO_IO_BUFFERED {
 
 public:

  /**
   * Prepare device for processing. After this call, device is 
   * ready for input/output (buffer can be pre-filled).
   *
   * ensure:
   *  (io_mode() == si_read && readable() == true) || writable()
   */
  virtual void prepare(void) = 0;

  /**
   * Start prosessing sample data. Underruns will occur if the 
   * calling program can't handle data at the speed of the 
   * source device. Write_buffer() calls are blocked if necessary.
   */
  virtual void start(void) = 0;

  /**
   * Stop processing. Doesn't usually concern non-realtime devices.
   * I/O is not allowed after this call. This should be used when 
   * audio object is not going to be used for a while.
   *
   * ensure:
   *  readable() == false
   *  writable() == false
   */
  virtual void stop(void) = 0;

  /**
   * Estimed processing latency in samples.
   */
  virtual long int latency(void) const { return(0); }
  
  virtual bool finished(void) const { return(is_open() == false); }

  /**
   * Seeking is impossible with realtime devices.
   */
  virtual void seek_position(void) { }

  virtual long position_in_samples(void) const = 0;
  virtual string status(void) const;
  
  virtual ~AUDIO_IO_DEVICE(void) { }
};

#endif

