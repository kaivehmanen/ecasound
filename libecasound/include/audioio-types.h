#ifndef _AUDIOIO_TYPES_H
#define _AUDIOIO_TYPES_H

#include "audioio.h"

/**
 * Virtual base for direct audio input/output. 
 * Subclasses must implement all buffering.
 * @author Kai Vehmanen
 */
class AUDIO_IO_DIRECT : public AUDIO_IO {

 public:

  void read_buffer(SAMPLE_BUFFER* sbuf) { }
  void write_buffer(SAMPLE_BUFFER* sbuf) { }

  virtual void buffersize(long int samples, long int sample_rate) { }
  virtual long int buffersize(void) const { return(0); }

  /**
   * Low-level routine for reading samples. Number of samples
   * is stored to 'samples_read' and pointer to the actual
   * sample data is returned. This must be implemented by all subclasses.
   */
  virtual unsigned char* read_samples(long int samples, long int* samples_read) = 0;

  /**
   * Low-level routine for writing samples. Pointer to a buffer 
   * that has space for 'samples' samples is returned. This must be 
   * implemented by all subclasses.
   */
  virtual unsigned char* write_samples(long int samples) = 0;

  AUDIO_IO_DIRECT(const string& name, 
		  const SIMODE mode, 
		    const ECA_AUDIO_FORMAT& fmt) { }

  virtual ~AUDIO_IO_DIRECT(void) { }
};

/**
 * Virtual base for buffered audio input/output.
 * Subclasses must implement all buffering.
 */
class AUDIO_IO_BUFFERED : public AUDIO_IO {

 public:

  void read_buffer(SAMPLE_BUFFER* sbuf);
  void write_buffer(SAMPLE_BUFFER* sbuf);

  virtual void buffersize(long int samples, long int sample_rate);
  virtual long int buffersize(void) const { return(buffersize_rep); }

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

  AUDIO_IO_BUFFERED(const string& name, 
		    const SIMODE mode, 
		    const ECA_AUDIO_FORMAT& fmt);

  virtual ~AUDIO_IO_BUFFERED(void);

 protected:

  void reserve_buffer_space(long int bytes);

 private:

  long int buffersize_rep;
  long int target_srate_rep;
  long int target_samples_rep;
  unsigned char* iobuf_uchar;  // buffer for raw-I/O
  size_t iobuf_size;
};

/**
 * Virtual base class for audio files.
 * @author Kai Vehmanen
 */
class AUDIO_IO_FILE : public AUDIO_IO_BUFFERED {

 public:

  virtual bool is_realtime(void) const { return(false); }

  AUDIO_IO_FILE(const string& name, 
		const SIMODE mode, 
		const ECA_AUDIO_FORMAT& fmt) : AUDIO_IO_BUFFERED(name, mode, fmt) { }

  virtual ~AUDIO_IO_FILE(void) { }
};

/**
 * Virtual base class for real-time devices.
 * @author Kai Vehmanen
 */
class AUDIO_IO_DEVICE : public AUDIO_IO_BUFFERED {
 
 public:

  virtual bool is_realtime(void) const { return(true); }
  virtual bool finished(void) const { return(is_open() == false); }
  virtual void seek_position(void) { }
  //  virtual long position_in_samples(void) const;

  AUDIO_IO_DEVICE(const string& name, 
		  const SIMODE mode, 
		  const ECA_AUDIO_FORMAT& fmt) 
    : AUDIO_IO_BUFFERED(name, mode, fmt) { }

  virtual ~AUDIO_IO_DEVICE(void) { }
};

#endif
