#ifndef INCLUDED_AUDIOIO_ARTS_H
#define INCLUDED_AUDIOIO_ARTS_H

#include <string>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef COMPILE_ARTS
extern "C" {
#include <artsc.h>
}
#endif

#include "eca-version.h"

/**
 * Interface for communicating with aRts/MCOP.
 * @author Kai Vehmanen
 */
class ARTS_INTERFACE : public AUDIO_IO_DEVICE {

  ARTS_INTERFACE(const ARTS_INTERFACE& x) { }
  ARTS_INTERFACE& operator=(const ARTS_INTERFACE& x) { return *this; }

#ifdef COMPILE_ARTS
  arts_stream_t stream_rep;
#endif
  long int samples_rep;
  static int ref_rep;
  
 public:

  string name(void) const { return("aRts client"); }
  string description(void) const { return("aRts client. Audio input and output using aRts server."); }

  virtual void open(void) throw(ECA_ERROR&);
  virtual void close(void);

  virtual int supported_io_modes(void) const { return(io_read | io_write); }

  virtual void stop(void);
  virtual void start(void);
  virtual void prepare(void) { }
  virtual long position_in_samples(void) const;

  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  ARTS_INTERFACE (const string& name = "arts");
  ~ARTS_INTERFACE(void);
    
  ARTS_INTERFACE* clone(void) { cerr << "Not implemented!" << endl; return this; }
  ARTS_INTERFACE* new_expr(void) { return new ARTS_INTERFACE(); }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void) { return(new ARTS_INTERFACE()); }
int audio_io_interface_version(void) { return(ECASOUND_LIBRARY_VERSION_CURRENT); }
};

#endif
