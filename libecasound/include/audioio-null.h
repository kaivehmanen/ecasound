#ifndef _AUDIOIO_NULL_H
#define _AUDIOIO_NULL_H

#include "audioio-types.h"

/**
 * Audio object that accepts any audio data. And is incredibly fast. :)
 */
class NULLFILE : public AUDIO_IO_BUFFERED {
 public:

  string name(void) const { return("Null audio file"); }

  void open(void) { }
  void close(void) { }

  long int read_samples(void* target_buffer, long int samples) { return(samples); }
  void write_samples(void* target_buffer, long int samples) { }

  bool finished(void) const { return(false); }
  void seek_position(void) { } 

  NULLFILE(const string& name = "null") { }
  ~NULLFILE(void) { }
  NULLFILE* clone(void) { return new NULLFILE(*this); }
  NULLFILE* new_expr(void) { return new NULLFILE(); }
};

#endif
