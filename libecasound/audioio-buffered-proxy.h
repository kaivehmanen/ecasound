#ifndef INCLUDED_AUDIOIO_BUFFERED_PROXY_H
#define INCLUDED_AUDIOIO_BUFFERED_PROXY_H

#include <string>
#include <kvutils/definition_by_contract.h>
#include "ecasound/audioio.h"
#include "ecasound/samplebuffer.h"
#include "audioio-proxy-server.h"

/**
 * Proxy class providing additional layer of buffering
 * to instances of class AUDIO_IO. Buffering is handled
 * by a separate i/o engine thread, which is common to all
 * proxy objects.
 * @author Kai Vehmanen
 */
class AUDIO_IO_BUFFERED_PROXY : public AUDIO_IO,
				protected DEFINITION_BY_CONTRACT {

  AUDIO_IO_PROXY_SERVER* pserver_repp;
  AUDIO_IO_PROXY_BUFFER* pbuffer_repp;
  AUDIO_IO* child_repp;

  AUDIO_IO_BUFFERED_PROXY& operator=(const AUDIO_IO_BUFFERED_PROXY& x) { return *this; }
  AUDIO_IO_BUFFERED_PROXY (const AUDIO_IO_BUFFERED_PROXY& x) { }

  int xruns_rep;

  void fetch_child_data(void);

 public:

  virtual string label(void) const { return(child_repp->label()); }
  virtual string name(void) const { return(child_repp->name()); }
  virtual string description(void) const { return(child_repp->description()); }

  virtual int supported_io_modes(void) const { return(child_repp->supported_io_modes()); }
  virtual bool supports_nonblocking_mode(void) const { return(child_repp->supports_nonblocking_mode()); }
  virtual bool supports_seeking(void) const { return(child_repp->supports_seeking()); }
  virtual bool finite_length_stream(void) const { return(child_repp->finite_length_stream()); }
  virtual bool locked_audio_format(void) const { return(child_repp->locked_audio_format()); }

  virtual bool finished(void) const;
  virtual long length_in_samples(void) const;

  virtual void buffersize(long int samples, long int sample_rate) { child_repp->buffersize(samples, sample_rate); }
  virtual long int buffersize(void) const { return(child_repp->buffersize()); }
  virtual void buffersize_changed(void) { child_repp->buffersize(buffersize(), samples_per_second()); }

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  void seek_position(void);
  void open(void) throw(SETUP_ERROR&);
  void close(void);

  AUDIO_IO_BUFFERED_PROXY (AUDIO_IO_PROXY_SERVER *pserver, AUDIO_IO* aobject); 
  ~AUDIO_IO_BUFFERED_PROXY(void);
  AUDIO_IO_BUFFERED_PROXY* clone(void) { cerr << "Not implemented!" << endl; return this; }
  AUDIO_IO_BUFFERED_PROXY* new_expr(void) { cerr << "Not implemented!" << endl; return this; }

protected:

  virtual bool class_invariant(void) const { return(child_repp != 0); }
};

#endif
