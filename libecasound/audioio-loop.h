#ifndef INCLUDED_AUDIOIO_LOOP_DEVICE_H
#define INCLUDED_AUDIOIO_LOOP_DEVICE_H

#include "audioio.h"
#include "samplebuffer.h"

/**
 * Audio object that routes data from inputs to outputs
 */
class LOOP_DEVICE : public AUDIO_IO {

  int id_rep;
  int writes_rep;
  int registered_inputs_rep;
  int registered_outputs_rep;
  int empty_rounds_rep;

  bool finished_rep;
  bool filled_rep;
    
  SAMPLE_BUFFER sbuf;

 public:

  virtual std::string name(void) const { return("Internal loop device"); }
  virtual std::string description(void) const { return("Loop device that routes data from output to input."); }

  virtual void buffersize(long int samples, long int sample_rate) { };
  virtual long int buffersize(void) const { return(0); };

  virtual void open(void) throw (AUDIO_IO::SETUP_ERROR &) { }
  virtual void close(void) { }

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  virtual bool finished(void) const;
  virtual void seek_position(void) { } 

  virtual std::string parameter_names(void) const { return("label,id_number"); }
  virtual void set_parameter(int param, std::string value);
  virtual std::string get_parameter(int param) const;

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
  LOOP_DEVICE(void) { }
  ~LOOP_DEVICE(void) { }
  LOOP_DEVICE* clone(void) const { return 0; }
  LOOP_DEVICE* new_expr(void) const { return new LOOP_DEVICE(); }
};

#endif
