#ifndef _AUDIOIO_NULL_H
#define _AUDIOIO_NULL_H

#include "audioio-types.h"

/**
 * Audio object that accepts any audio data. And is incredibly fast. :)
 */
class NULLFILE : public AUDIO_IO_BUFFERED {
 public:

  virtual string name(void) const { return("Null audio file"); }

  virtual void open(void) { }
  virtual void close(void) { }

  virtual long int read_samples(void* target_buffer, long int samples) { return(samples); }
  virtual void write_samples(void* target_buffer, long int samples) { }

  virtual bool finished(void) const { return(false); }
  virtual void seek_position(void) { } 

  NULLFILE(const string& name = "null") { }
  ~NULLFILE(void) { }
  NULLFILE* clone(void) { return new NULLFILE(*this); }
  NULLFILE* new_expr(void) { return new NULLFILE(); }
};

#endif
