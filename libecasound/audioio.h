#ifndef INCLUDED_AUDIOIO_H
#define INCLUDED_AUDIOIO_H

#include <string>

#include "eca-audio-position.h"
#include "eca-audio-time.h"
#include "dynamic-object.h"
#include "samplebuffer.h"

/**
 * Virtual base for all audio I/O classes (files, audio devices,
 * sound producing program modules, etc.)
 *
 * The class interface is divided into following sections:
 *
 *  - type definitions
 *
 *  - attributes
 *
 *  - configuration (setting and getting configuration parameters)
 *
 *  - functionality (control and runtime information)
 *
 *  - runtime information
 *
 *  - constructors and destructors
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO : public DYNAMIC_OBJECT<string>,
		 public ECA_AUDIO_POSITION {

 public:

  // ===================================================================
  // Type definitions

  /**
   * Input/Output mode
   *
   * @see io_mode()
   *
   * io_read
   *
   * Device is opened for input. If opening a file, 
   * it must exist.
   *
   * io_write
   *
   * Device is opened for output. If opening a file and
   * and output exists, it is first truncated.
   * 
   * io_readwrite
   *
   * Device is opened for both reading and writing. If
   * opening a file, a new file is created if needed. 
   * When switching from read to write or vica versa,
   * position should be reset before using the device.
   **/
  enum Io_mode { io_read = 1, io_write = 2, io_readwrite = 4 };

  class SETUP_ERROR {
   public:
    enum Error_type {
      sample_format,    /* unsupported sample format */
      channels,         /* unsupported channel count */
      sample_rate,      /* unsupported sample_rate */
      interleaving,     /* non-interleaved or interleaved channel organization not supported */
      io_mode,          /* unsupported I/O mode */
      buffersize,       /* unsupported buffersize */
      blockmode,        /* non-blocking or blocking mode not supported */
      dynamic_params,   /* invalid dynamic parameters (for instance invalid label()) */
      unexpected        /* unexpected/unknown error */
    };
    
     const string& message(void) const;
     Error_type type(void) const;
     SETUP_ERROR(Error_type type, const string& message);

   private:
     Error_type type_rep;
     string message_rep;
  };

  // ===================================================================
  // Attributes

  virtual int supported_io_modes(void) const;
  virtual bool supports_nonblocking_mode(void) const;
  virtual bool supports_seeking(void) const;
  virtual bool finite_length_stream(void) const;
  virtual bool locked_audio_format(void) const;
  
  // ===================================================================
  // Configuration (setting and getting configuration parameters)

  /**
   * Sets the sample buffer size in sample frames. Instead of specifying buffer 
   * length in seconds, parameters 'samples' and 'sample_rate' are used to 
   * determinate the length. When reading, this means that 'buffersize' 
   * sample frames of data is read. When writing, 'buffersize' is only used for 
   * initializing devices and data structures. Device should be able 
   * to write all sample data independently from current buffersize value.
   * When audio object's sampling rate is changed, it's often wise to adjust
   * the buffersize. Otherwise the real length of the buffer changes whenever
   * sampling parameters are changed.
   */
  virtual void buffersize(long int samples, long int sample_rate) = 0;

  /**
   * Returns the buffersize in sample frames.
   */
  virtual long int buffersize(void) const = 0;

  int io_mode(void) const;
  const string& label(void) const;
  string format_info(void) const;

  void io_mode(int mode);
  void label(const string& id_label);
  void toggle_nonblocking_mode(bool value);

  virtual string parameter_names(void) const { return("label"); }
  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  // ===================================================================
  // Functionality (control and runtime information)

 public:

  /**
   * Reads samples to buffer pointed by 'sbuf'. If necessary, the target 
   * buffer will be resized.
   *
   * It's important to note that SAMPLE_BUFFER audio format cannot be
   * changed during processing. This means that audio data must be converted
   * from audio object's format to buffer object's format. SAMPLE_BUFFER 
   * class provides tools for all normal conversion operations. If you need
   * direct access to object's data, a lower abstaction level should be used
   * (@see AUDIO_IO_DIRECT).
   *
   * @see read_samples
   *
   * require:
   *  io_mode() == io_read || io_mode() == io_readwrite
   *  readable() == true
   *  sbuf != 0
   *
   * ensure:
   *  sbuf->length_in_samples() <= buffersize()
   */
  virtual void read_buffer(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * Writes all data from sample buffer pointed by 'sbuf'. Notes
   * concerning read_buffer() also apply to this routine.
   *
   * @see write samples
   *
   * require:
   *  io_mode() == io_write || io_mode() == io_readwrite
   *  writable() == true
   *  sbuf != 0
   */
  virtual void write_buffer(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * Opens the audio object (possibly in exclusive mode).
   * This routine is used for initializing external connections 
   * (opening files or devices, loading shared libraries, 
   * opening IPC connections). As it's impossible to know in 
   * advance what might happen, open() may throw an 
   * exception.  This way it becomes possible to provide 
   * more verbose information about the problem that caused 
   * open() to fail.
   *
   * At this point the various audio parameters are used
   * for the first time. Unless locked_audio_format() is 'true', 
   * object tries to use the audio format parameters set prior to 
   * this call. If object doesn't support the given parameter
   * combination, it can either try adjust them to closest
   * matching, or in the worst case, throw an SETUP_ERROR 
   * exception (see above).
   *
   * ensure:
   *  readable() == true || writable() == true || is_open() != true
   */
  virtual void open(void) throw (AUDIO_IO::SETUP_ERROR &) = 0;

  /**
   * Closes audio object. After calling this routine, 
   * all resources (ie. soundcard) must be freed
   * (they can be used by other processes).
   *
   * ensure:
   *  readable() != true
   *  writable() != true
   */
  virtual void close(void) = 0;

  // ===================================================================
  // Runtime information

  /**
   * Effectively this is meant for implementing nonblocking 
   * input and output with devices supporting it. If supports_nonblocking_mode() 
   * == true, this function can be used to check how many samples are 
   * available for reading, or alternatively, how many samples can be 
   * written without blocking.  Note, you should use buffersize() call 
   * for setting how many bytes read_buffer() will ask from the device.
   *
   * require:
   *  supports_nonblocking_mode() == true
   */
  virtual long int samples_available(void) const { return(0); }

  /**
   * Has device been opened (with open())?
   */
  bool is_open(void) const { return(open_rep); }

  /**
   * Whether all data has been processed? If opened in mode 'io_read', 
   * this means that end of stream has been reached. If opened in 
   * 'io_write' or 'io_readwrite' modes, finished status usually
   * means that an error has occured (no space left, etc). After 
   * finished() has returned 'true', further calls to read_buffer() 
   * and/or write_buffer() won't process any data.
   *
   * For inputs for which 'finite_length_stream()' is true, when
   * 'finished()' returns true, that means an error has occured. 
   * Otherwise 'finished()' just tells that further attempts to do 
   * i/o will fail.
   */
  virtual bool finished(void) const = 0;

  virtual bool nonblocking_mode(void) const;
  virtual bool readable(void) const;
  virtual bool writable(void) const;
  virtual string status(void) const;

  ECA_AUDIO_TIME length(void) const;
  ECA_AUDIO_TIME position(void) const;

 protected:

  void position(const ECA_AUDIO_TIME& v);
  void length(const ECA_AUDIO_TIME& v);

  void toggle_open_state(bool value);

  // ===================================================================
  // Constructors and destructors

 public:

  virtual AUDIO_IO* clone(void) = 0;
  virtual AUDIO_IO* new_expr(void) = 0;
  virtual ~AUDIO_IO(void);
  AUDIO_IO(const string& name = "unknown", 
	   int mode = io_read, 
	   const ECA_AUDIO_FORMAT& fmt = ECA_AUDIO_FORMAT());

 private:
  
  int io_mode_rep;
  string id_label_rep;

  bool nonblocking_rep;
  bool readable_rep;
  bool writable_rep;
  bool open_rep;
};

#endif
