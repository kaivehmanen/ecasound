#ifndef _AUDIOIO_LOOP_DEVICE_H
#define _AUDIOIO_LOOP_DEVICE_H

#include "audioio.h"
#include "samplebuffer.h"

/**
 * Audio object that routes data from inputs to outputs
 */
class LOOP_DEVICE : public AUDIO_IO {

  int registered_inputs_rep;

  int id_rep;
  int writes_rep;
  int registered_outputs_rep;

  bool filled_rep;
    
  SAMPLE_BUFFER sbuf;

 public:

  void buffersize(long int samples, long int sample_rate) { };
  long int buffersize(void) const { return(0); };

  void open(void) { }
  void close(void) { }

  void read_buffer(SAMPLE_BUFFER* sbuf);
  void write_buffer(SAMPLE_BUFFER* sbuf);

  bool finished(void) const { return(true); }
  void seek_position(void) { } 

  /**
   * Register a new input client
   */
  void register_input(void) { ++registered_inputs_rep; }
  
  /**
   * Register a new output client
   */
  void register_output(void) { ++registered_outputs_rep; }

  int id(void) const { return(id_rep); }

  LOOP_DEVICE(int id);

  ~LOOP_DEVICE(void) { }
  LOOP_DEVICE* clone(void) { return new LOOP_DEVICE(*this); }
};

#endif
