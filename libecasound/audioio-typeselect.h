#ifndef INCLUDED_AUDIOIO_TYPESELECT_H
#define INCLUDED_AUDIOIO_TYPESELECT_H

#include <string>
#include <vector>
#include <iostream>

#include "audioio.h"

/**
 * A proxy class for overriding default keyword 
 * and filename associations in ecasound's object
 * maps.
 *
 * Related design patterns:
 *     - Proxy (GoF207)
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_TYPESELECT : public AUDIO_IO {

 public:

  /** @name Public functions */
  /*@{*/

  AUDIO_IO_TYPESELECT (void); 
  virtual ~AUDIO_IO_TYPESELECT(void);

  /*@}*/
  
  /** @name Reimplemented functions from ECA_OBJECT */
  /*@{*/

  virtual std::string name(void) const { return(string("Typeselect => ") + child_repp->name()); }
  virtual std::string description(void) const { return(child_repp->description()); }

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_PARAMETERS<string> */
  /*@{*/

  virtual std::string parameter_names(void) const;
  virtual void set_parameter(int param, std::string value);
  virtual std::string get_parameter(int param) const;

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_OBJECT<string> */
  /*@{*/

  virtual AUDIO_IO_TYPESELECT* clone(void) const;
  virtual AUDIO_IO_TYPESELECT* new_expr(void) const { return(new AUDIO_IO_TYPESELECT()); }

  /*@}*/

  /** @name Reimplemented functions from ECA_AUDIO_POSITION */
  /*@{*/

  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const { return(child_repp->position_in_samples()); }
  virtual SAMPLE_SPECS::sample_pos_t length_in_samples(void) const { return(child_repp->length_in_samples()); }
  virtual void set_position_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  virtual void set_length_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  virtual void seek_position(void) { return(child_repp->seek_position()); }

  /*@}*/

  /** @name Reimplemented functions from AUDIO_IO */
  /*@{*/

  virtual int supported_io_modes(void) const { return(child_repp->supported_io_modes()); }
  virtual bool supports_nonblocking_mode(void) const { return(child_repp->supports_nonblocking_mode()); }
  virtual bool supports_seeking(void) const { return(child_repp->supports_seeking()); }
  virtual bool finite_length_stream(void) const { return(child_repp->finite_length_stream()); }
  virtual bool locked_audio_format(void) const { return(child_repp->locked_audio_format()); }

  virtual void set_buffersize(long int samples) { child_repp->set_buffersize(samples); }
  virtual long int buffersize(void) const { return(child_repp->buffersize()); }

  virtual void read_buffer(SAMPLE_BUFFER* sbuf) { child_repp->read_buffer(sbuf); }
  virtual void write_buffer(SAMPLE_BUFFER* sbuf) { child_repp->write_buffer(sbuf); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);

  virtual bool finished(void) const { return(child_repp->finished()); }

  /*@}*/

 private:

  std::string type_rep;
  mutable std::vector<std::string> params_rep;
  AUDIO_IO* child_repp;
  bool init_rep;

  AUDIO_IO_TYPESELECT& operator=(const AUDIO_IO_TYPESELECT& x) { return *this; }
  AUDIO_IO_TYPESELECT (const AUDIO_IO_TYPESELECT& x) { }

};

#endif
