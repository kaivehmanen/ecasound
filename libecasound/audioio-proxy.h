#ifndef INCLUDED_AUDIO_IO_PROXY_H
#define INCLUDED_AUDIO_IO_PROXY_H

#include <kvu_dbc.h>

#include "audioio.h"

class SAMPLE_BUFFER;

/**
 * Generic interface for objects that act as
 * proxies for other objects of type AUDIO_IO.
 *
 * Related design patterns:
 *     - Proxy (GoF207
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_PROXY : public AUDIO_IO {

 public:

  /** @name Public functions */
  /*@{*/

  AUDIO_IO_PROXY (void); 
  virtual ~AUDIO_IO_PROXY(void);

  /*@}*/

  /** @name Reimplemented functions from ECA_OBJECT */
  /*@{*/

  virtual std::string name(void) const { return(string("Proxy => ") + child_repp->name()); }
  virtual std::string description(void) const { return(child_repp->description()); }

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_OBJECT<string> */
  /*@{*/

  virtual AUDIO_IO_PROXY* clone(void) const { return(new AUDIO_IO_PROXY()); }
  virtual AUDIO_IO_PROXY* new_expr(void) const { return(new AUDIO_IO_PROXY()); }

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

  virtual bool supports_nonblocking_mode(void) const { return(child_repp->supports_nonblocking_mode()); }
  virtual bool locked_audio_format(void) const { return(child_repp->locked_audio_format()); }

  virtual void set_buffersize(long int samples);
  virtual long int buffersize(void) const { return(buffersize_rep); }

  virtual void read_buffer(SAMPLE_BUFFER* sbuf) { child_repp->read_buffer(sbuf); }
  virtual void write_buffer(SAMPLE_BUFFER* sbuf) { child_repp->write_buffer(sbuf); }

  virtual bool finished(void) const { return(child_repp->finished()); }

  /*@}*/

  /** @name Reimplemented functions from ECA_AUDIO_FORMAT */
  /*@{*/

  virtual void set_channels(SAMPLE_SPECS::channel_t v);
  virtual void set_sample_format(Sample_format v) throw(ECA_ERROR&);

  /*@}*/

 protected: 

  void set_child(AUDIO_IO* v);
  AUDIO_IO* child(void) const { return child_repp; }

 private:

  AUDIO_IO* child_repp;
  long int buffersize_rep;
};

#endif // INCLUDED_AUDIO_IO_PROXY
