#ifndef _AUDIOIO_H
#define _AUDIOIO_H

#include <string>

#include "eca-audio-position.h"

class SAMPLE_BUFFER;

/**
 * Input/Output mode
 *
 * si_read
 *
 * Device is opened for input. If opening a file, 
 * it must exist.
 *
 * si_write
 *
 * Device is opened for output. If opening a file and
 * and output exists, it is first truncated.
 * 
 * si_readwrite
 *
 * Device is opened for both reading and writing. If
 * opening a file, a new file is created if needed. 
 * When switching from read to write or vica versa,
 * position should be reset before using the device.
 *
 */
enum SIMODE { si_read, si_write, si_readwrite };

/**
 * Virtual base for all audio I/O classes (files, audio devices,
 * sound producing program modules, etc.)
 * @author Kai Vehmanen
 */
class AUDIO_IO : public ECA_AUDIO_POSITION {

 public:

  // ===================================================================
  // Buffering

  /**
   * Set buffersize. Instead of specifying buffer length in seconds,
   * we use 'samples' and 'samples_rate' parameters to determinate the 
   * exact temporal length. When reading, this means that 'buffersize' of 
   * sample data is read. When writing, 'buffersize' is the maximum count of 
   * samples written at a time. If audio object's sampling rate is 
   * changed, buffersize must also be adjusted.
   *
   * Value 0 is allowed for both 'samples' and 'sample_rate' (no change).
   */
  virtual void buffersize(long int samples, long int sample_rate) = 0;

  /**
   * Returns buffersize in samples.
   */
  virtual long int buffersize(void) const = 0;

  // ===================================================================
  // Basic input/output

  /**
   * Read samples to buffer pointed by 'sbuf'. If necessary, the target 
   * buffer will be resized.
   *
   * @see read_samples
   *
   * require:
   *  io_mode() == si_read
   *  readable() == true
   *  sbuf != 0
   */
  virtual void read_buffer(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * Write all data from sample buffer pointer by 'sbuf'.
   *
   * write samples
   *
   * require:
   *  io_mode() == si_write || io_mode() == si_readwrite
   *  writable() == true
   *  sbuf != 0
   */
  virtual void write_buffer(SAMPLE_BUFFER* sbuf) = 0;

  /**
   * Whether all data has been processed? If opened in mode 'si_read', 
   * this means that end of stream has been reached. If opened in 
   * 'si_write' or 'si_readwrite' modes, finished status usually
   * means that an error has occured (no space left, etc).
   *
   * @see SIMODE
   */
  virtual bool finished(void) const = 0;

  // ===================================================================
  // Audio object control

 public: 
    
  /**
   * Open audio object (possibly in exclusive mode).
   * This routine is meant for opening files and devices,
   * loading libraries, etc. Object's audio format can be 
   * adjusted during this call.
   *
   * ensure:
   *  readable() == true || writable() == true
   */
  virtual void open(void) = 0;

  /**
   * Close audio object. After calling this routine, 
   * all resources (ie. soundcard) must be freed
   * (they can be used by other processes).
   *
   * ensure:
   *  readable() == false
   *  writable() == false
   */
  virtual void close(void) = 0;

  /**
   * Start prosessing sample data. Underruns will occur if the 
   * calling program can't handle data at the speed of the 
   * source device. Write_buffer() calls are blocked if necessary.
   *
   * ensure:
   *  (io_mode() == si_read && readable() == true) || writable()
   */
  virtual void start(void) { }

  /**
   * Stop processing. Doesn't usually concern non-realtime devices.
   * I/O is not allowed after this call. This should be used when 
   * audio object is not going to be used for a while.
   *
   * ensure:
   *  readable() == false
   *  writable() == false
   */
  virtual void stop(void) { } 

  // ===================================================================
  // Features (virtual)

  /**
   * With this method, derived class can specify whether it
   * represents a realtime device? Here, a realtime device...
   *
   * - is disabled after device is opened
   *
   * - is enabled with start()
   *
   * - once enabled, will handle I/O at a constant speed
   *   based on the sample format paremeters
   *
   * - is disabled with stop()
   */
  virtual bool is_realtime(void) const = 0;

  /**
   * Estimed processing latency in samples.
   */
   long int latency(void) const { return(0); }

  // ===================================================================
  // Status and info

  /**
   * Optional status string.
   */
  virtual string status(void) const;

  /**
   * Returns info about the I/O mode of this device. 
   * @ref SIMODE
   */
  inline const SIMODE io_mode(void) const { return(si_mode); }

  /**
   * The device name (usually set to device/file name).
   */
  const string& label(void) const { return(id_label); }

  /**
   * Get a string containing info about sample format parameters.
   */
  string format_info(void) const;

  /**
   * Has device been opened (with open_device())?
   */
  bool is_open(void) const { return(open_rep); }

  virtual bool readable(void) const { return(is_open() && io_mode() != si_write); }
  virtual bool writable(void) const { return(is_open() && io_mode() != si_read); }

 protected:

  virtual AUDIO_IO* clone(void) = 0;

  // --
  // Format setup
  // --

  /**
   * Set IO-mode.
   */
  void io_mode(SIMODE newmode) { si_mode = newmode; }

  /**
   * Set device with name.
   */
  void label(const string& newlabel) { id_label = newlabel; }

  /**
   * Set device state to enabled or disabled.
   */
  void toggle_open_state(bool value) { open_rep = value; }

  // ===================================================================
  // Constructors and destructors

 public:

  virtual ~AUDIO_IO(void) { }
  AUDIO_IO(const string& name = "unknown", 
	   const SIMODE mode = si_read, 
	   const ECA_AUDIO_FORMAT& fmt = ECA_AUDIO_FORMAT())
    : ECA_AUDIO_POSITION(fmt)
    {
      label(name);
      io_mode(mode);
      
      position_in_samples(0);

      readable_rep = writable_rep = open_rep = false;
    }

 private:
  
  SIMODE si_mode;
  string id_label;

  bool readable_rep;
  bool writable_rep;
  bool open_rep;
};

#endif
