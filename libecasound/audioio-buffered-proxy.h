#ifndef INCLUDED_AUDIOIO_BUFFERED_PROXY_H
#define INCLUDED_AUDIOIO_BUFFERED_PROXY_H

#include <string>
#include <iostream>

#include "audioio.h"
#include "samplebuffer.h"
#include "audioio-proxy-server.h"

/**
 * Proxy class providing additional layer of buffering
 * to instances of class AUDIO_IO. Buffering is handled
 * by a separate i/o engine thread, which is common to all
 * proxy objects.
 * @author Kai Vehmanen
 */
class AUDIO_IO_BUFFERED_PROXY : public AUDIO_IO {

  AUDIO_IO_PROXY_SERVER* pserver_repp;
  AUDIO_IO_PROXY_BUFFER* pbuffer_repp;
  AUDIO_IO* child_repp;

  AUDIO_IO_BUFFERED_PROXY& operator=(const AUDIO_IO_BUFFERED_PROXY& x) { return *this; }
  AUDIO_IO_BUFFERED_PROXY (const AUDIO_IO_BUFFERED_PROXY& x) { }

  int xruns_rep;
  bool finished_rep;

  void fetch_child_data(void);

 public:

  // --
  // public functions


  AUDIO_IO_BUFFERED_PROXY (AUDIO_IO_PROXY_SERVER *pserver, AUDIO_IO* aobject); 
  virtual ~AUDIO_IO_BUFFERED_PROXY(void);
  
  // --
  // reimplemented functions from ECA_OBJECT
  
  virtual string name(void) const { return(string("Buffering proxy => ") + child_repp->name()); }
  virtual string description(void) const { return(child_repp->description()); }

  // --
  // reimplemented functions from DYNAMIC_PARAMETERS<string>

  virtual string parameter_names(void) const { return(child_repp->parameter_names()); }
  virtual void set_parameter(int param, string value) { child_repp->set_parameter(param,value); }
  virtual string get_parameter(int param) const { return(child_repp->get_parameter(param)); }

  // --
  // reimplemented functions from DYNAMIC_OBJECT<string>

  AUDIO_IO_BUFFERED_PROXY* clone(void) { std::cerr << "Not implemented!" << std::endl; return this; }
  AUDIO_IO_BUFFERED_PROXY* new_expr(void) { std::cerr << "Not implemented!" << std::endl; return this; }

  // --
  // reimplemented functions from ECA_AUDIO_POSITION

  virtual long length_in_samples(void) const { return(child_repp->length_in_samples()); }
  virtual void seek_position(void);
  /* -- not reimplemented 
   * virtual void length_in_samples(long pos) { return(child_repp->length_in_samples(pos); }
   * virtual long position_in_samples(void) const { return(child_repp->position_in_samples()); }
   * virtual void position_in_samples(long pos) { child_repp->position_in_samples(pos); }
   */

  // --
  // reimplemented functions from AUDIO_IO

  virtual int supported_io_modes(void) const { return(child_repp->supported_io_modes()); }
  virtual bool supports_nonblocking_mode(void) const { return(child_repp->supports_nonblocking_mode()); }
  virtual bool supports_seeking(void) const { return(child_repp->supports_seeking()); }
  virtual bool finite_length_stream(void) const { return(child_repp->finite_length_stream()); }
  virtual bool locked_audio_format(void) const { return(child_repp->locked_audio_format()); }

  virtual void buffersize(long int samples, long int sample_rate) { child_repp->buffersize(samples, sample_rate); }
  virtual long int buffersize(void) const { return(child_repp->buffersize()); }

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);

  virtual bool finished(void) const;

protected:

  // --
  // functions reimplemented from AUDIO_IO

  virtual bool class_invariant(void) const { return(child_repp != 0); }
};

#endif
