#ifndef INCLUDED_AUDIOIO_DB_CLIENT_H
#define INCLUDED_AUDIOIO_DB_CLIENT_H

#include <string>
#include <iostream>

#include "audioio.h"
#include "audioio-db-server.h"

class SAMPLE_BUFFER;

/**
 * Client class for double-buffering providing 
 * additional layer of buffering for objects
 * derived from AUDIO_IO.
 *
 * The buffering subsystem has been optimized for 
 * reliable streaming performance. Because of this some 
 * operations like random seeks are considerably slower 
 * than with direct access.
 *
 * Related design patterns:
 *     - Proxy (GoF207)
 *
 * @author Kai Vehmanen
 */
class AUDIO_IO_DB_CLIENT : public AUDIO_IO {

 public:

  /** @name Public functions */
  /*@{*/

  AUDIO_IO_DB_CLIENT (AUDIO_IO_DB_SERVER *pserver, AUDIO_IO* aobject, bool transfer_ownership); 
  virtual ~AUDIO_IO_DB_CLIENT(void);

  /*@}*/
  
  /** @name Reimplemented functions from ECA_OBJECT */
  /*@{*/

  virtual std::string name(void) const { return(string("DB => ") + child_repp->name()); }
  virtual std::string description(void) const { return(child_repp->description()); }

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_PARAMETERS<string> */
  /*@{*/

  virtual std::string parameter_names(void) const { return(child_repp->parameter_names()); }
  virtual void set_parameter(int param, std::string value) { child_repp->set_parameter(param,value); }
  virtual std::string get_parameter(int param) const { return(child_repp->get_parameter(param)); }

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_OBJECT<string> */
  /*@{*/

  AUDIO_IO_DB_CLIENT* clone(void) const { std::cerr << __FILE__ << ": Not implemented!" << std::endl; return 0; }
  AUDIO_IO_DB_CLIENT* new_expr(void) const  { std::cerr << __FILE__ << ": Not implemented!" << std::endl; return 0; }

  /*@}*/

  /** @name Reimplemented functions from ECA_AUDIO_POSITION */
  /*@{*/

  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const { return(child_repp->position_in_samples()); }
  virtual SAMPLE_SPECS::sample_pos_t length_in_samples(void) const { return(child_repp->length_in_samples()); }
  virtual void set_position_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  virtual void set_length_in_samples(SAMPLE_SPECS::sample_pos_t pos);
  virtual void seek_position(void);

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

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);

  virtual bool finished(void) const;

  /*@}*/

  private:

  AUDIO_IO_DB_SERVER* pserver_repp;
  AUDIO_IO_DB_BUFFER* pbuffer_repp;
  AUDIO_IO* child_repp;

  AUDIO_IO_DB_CLIENT& operator=(const AUDIO_IO_DB_CLIENT& x) { return *this; }
  AUDIO_IO_DB_CLIENT (const AUDIO_IO_DB_CLIENT& x) { }

  int xruns_rep;
  bool finished_rep;
  bool free_child_rep;
  bool recursing_rep;

  void fetch_child_data(void);
};

#endif
