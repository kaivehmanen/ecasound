#ifndef INCLUDED_AUDIOIO_DEVICE_H
#define INCLUDED_AUDIOIO_DEVICE_H

#include "audioio-buffered.h"

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

  /** @name Public static functions */
  /*@{*/

  /**
   * Whether given object is an AUDIO_IO_DEVICE object.
   */
  static bool is_realtime_object(const AUDIO_IO* aobj);

  /*@}*/

  /** @name Constructors and destructors */
  /*@{*/

  AUDIO_IO_DEVICE(void);
  virtual ~AUDIO_IO_DEVICE(void);

  /*@}*/

  /** @name Attribute functions */
  /*@{*/

  virtual bool supports_seeking(void) const { return(false); }

  /**
   * Estimed processing latency in samples.
   */
  virtual long int latency(void) const { return(0); }

  /*@}*/

  /** @name Configuration 
   * 
   * For setting and getting configuration parameters.
   */
  /*@{*/

  /**
   * Whether to ignore possible under- and overrun 
   * situations. If enabled, device should try to
   * recover from these situations, ie. keep on 
   * running. If disabled, processing should be aborted
   * if an xrun occurs. Should be set before opening 
   * the device. Defaults to 'true'.
   */
  virtual void toggle_ignore_xruns(bool v) { ignore_xruns_rep = v; }

  /** 
   * Whether the use of internal buffering is limited. 
   * If disabled, the device should use minimal amount 
   * of internal buffering. The recommended size is 
   * two or three fragments, each buffersize() sample frames
   * in size. Otherwise the device can use all its
   * internal buffering. This toggle is meant for controlling
   * the latency caused by the device. Defaults to 'true'.
   */
  virtual void toggle_max_buffers(bool v) { max_buffers_rep = v; }
  
  virtual bool ignore_xruns(void) const { return(ignore_xruns_rep); }
  virtual bool max_buffers(void) const { return(max_buffers_rep); }

  /*@}*/

  /** @name Main functionality */
  /*@{*/

  /**
   * Prepare device for processing. After this call, device is 
   * ready for input/output (buffer can be pre-filled).
   *
   * require:
   *  is_running() != true 
   *
   * ensure:
   *  (io_mode() == si_read && readable() == true) || writable()
   */
  virtual void prepare(void) { is_prepared_rep = true; }

  /**
   * Start prosessing sample data. Underruns will occur if the 
   * calling program can't handle data at the speed of the 
   * source device. Write_buffer() calls are blocked if necessary.
   *
   * Note! For output devices, at least one buffer of data 
   *       must have been written before issuing start()!
   *
   * require:
   *  is_running() != true
   *  is_prepared() == true 
   *
   * ensure:
   *  is_running() == true
   */
  virtual void start(void) { is_running_rep = true; }

  /**
   * Stop processing. Doesn't usually concern non-realtime devices.
   * I/O is not allowed after this call. This should be used when 
   * audio object is not going to be used for a while.
   *
   * require:
   *  is_running() == true
   * 
   * ensure:
   *  is_running() != true
   *  is_prepared() != true
   *  readable() == false
   *  writable() == false
   */
  virtual void stop(void) { is_running_rep = false; is_prepared_rep = false; }

  /*@}*/

  /** @name Runtime information */
  /*@{*/

  /**
   * Whether device has been started?
   */
  bool is_running(void) const { return(is_running_rep); }

  /**
   * Whether device has been prepared for processing?
   */
  bool is_prepared(void) const { return(is_prepared_rep); }

  virtual bool finished(void) const { return(is_open() == false); }

  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const = 0;

  virtual std::string status(void) const;

  /*@}*/
  
  /**
   * Seeking is impossible with realtime devices.
   */
  virtual void seek_position(void) { }

 private:
  
  bool is_running_rep;
  bool is_prepared_rep;
  bool ignore_xruns_rep;
  bool max_buffers_rep;
};

#endif
