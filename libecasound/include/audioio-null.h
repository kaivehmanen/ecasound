#ifndef _AUDIOIO_NULL_H
#define _AUDIOIO_NULL_H

#include "audioio.h"

/**
 * Audio object that accepts any audio data. And is incredibly fast. :)
 */
class NULLFILE : public AUDIO_IO_FILE {
 public:

  void open(void) { }
  void close(void) { }

  long int read_samples(void* target_buffer, long int samples) { return(0); }
  void write_samples(void* target_buffer, long int samples) { }

  bool finished(void) const { return(false); }
  void seek_position(void) { } 

  NULLFILE(const string& name = "null", 
	   SIMODE mode = si_read,
	   const ECA_AUDIO_FORMAT& fmt = ECA_AUDIO_FORMAT()) 
    : AUDIO_IO_FILE(name, mode, fmt) { }

  ~NULLFILE(void) { }
  NULLFILE* clone(void) { return new NULLFILE(*this); }
};

#endif
