#ifndef INCLUDED_AUDIOIO_NULL_H
#define INCLUDED_AUDIOIO_NULL_H

#include "audioio-types.h"

/**
 * Audio object that endlessly consumes and produces audio data.
 * And is incredibly fast. :)
 */
class NULLFILE : public AUDIO_IO_BUFFERED {
 public:

  virtual std::string name(void) const { return("Null audio object"); }

  virtual void open(void) throw (AUDIO_IO::SETUP_ERROR &) { }
  virtual void close(void) { }

  virtual long int read_samples(void* target_buffer, long int samples) { 
    for(int n = 0; n < samples * frame_size(); n++) ((char*)target_buffer)[n] = 0;
    return(samples); 
  }
  virtual void write_samples(void* target_buffer, long int samples) { }

  virtual bool finished(void) const { return(false); }
  virtual bool supports_seeking(void) const { return(false); }
  virtual void seek_position(void) { } 

  NULLFILE(const std::string& name = "null") { label(name); }
  virtual ~NULLFILE(void) { }
  NULLFILE* clone(void) { return new NULLFILE(*this); }
  NULLFILE* new_expr(void) { return new NULLFILE(); }
};

#endif
