#ifndef INCLUDED_AUDIOIO_REVERSE_H
#define INCLUDED_AUDIOIO_REVERSE_H

#include <string>
#include <vector>
#include <iostream>

#include "audioio.h"

class SAMPLE_BUFFER;

/**
 * A proxy class that reverts the child 
 * object's data.
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_REVERSE : public AUDIO_IO {

 public:

  /** @name Public functions */
  /*@{*/

  AUDIO_IO_REVERSE (void); 
  virtual ~AUDIO_IO_REVERSE(void);

  /*@}*/
  
  /** @name Reimplemented functions from ECA_OBJECT */
  /*@{*/

  virtual std::string name(void) const { return(string("Reverse => ") + child_repp->name()); }
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

  virtual AUDIO_IO_REVERSE* clone(void) const { std::cerr << "Not implemented!" << std::endl; return 0; }
  virtual AUDIO_IO_REVERSE* new_expr(void) const { return(new AUDIO_IO_REVERSE()); }

  /*@}*/

  /** @name Reimplemented functions from ECA_AUDIO_POSITION */
  /*@{*/

  virtual SAMPLE_SPECS::sample_pos_t length_in_samples(void) const { return(child_repp->length_in_samples()); }
  virtual void seek_position(void);
  
  /* -- not reimplemented 
   * virtual void length_in_samples(long pos) 
   * virtual void position_in_samples(long pos) 
   * virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const
   */

  /*@}*/

  /** @name Reimplemented functions from AUDIO_IO */
  /*@{*/

  virtual int supported_io_modes(void) const { return(io_read); }
  virtual bool supports_nonblocking_mode(void) const { return(child_repp->supports_nonblocking_mode()); }
  virtual bool supports_seeking(void) const { return(child_repp->supports_seeking()); }
  virtual bool finite_length_stream(void) const { return(child_repp->finite_length_stream()); }
  virtual bool locked_audio_format(void) const { return(child_repp->locked_audio_format()); }

  virtual void buffersize(long int samples, long int sample_rate) { child_repp->buffersize(samples, sample_rate); }
  virtual long int buffersize(void) const { return(child_repp->buffersize()); }

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf) { child_repp->write_buffer(sbuf); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);

  virtual bool finished(void) const;

  /*@}*/

 private:

  mutable std::vector<std::string> params_rep;
  AUDIO_IO* child_repp;
  bool init_rep;
  bool finished_rep;
  SAMPLE_SPECS::sample_pos_t curpos;
  SAMPLE_BUFFER* tempbuf_repp;

  AUDIO_IO_REVERSE& operator=(const AUDIO_IO_REVERSE& x) { return *this; }
  AUDIO_IO_REVERSE (const AUDIO_IO_REVERSE& x) { }

};

#endif
