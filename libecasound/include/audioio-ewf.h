#ifndef _AUDIOIO_EWF_H
#define _AUDIOIO_EWF_H

#include <string>
#include "audioio.h"
#include "audioio-wave.h"
#include "samplebuffer.h"

/**
 * Ecasound Wave File (.EWF) class. A simple wrapper class for WAVE 
 * files that allows seeking beyond end position. When first write_buffer() 
 * call is issued, current sample offset is stored into the .ewf file
 * and corresponding .wav is opened for writing. Read_buffer() calls 
 * return silent buffers until sample_offset is reached. After that,
 * .wav file is read normally.
 * @author Kai Vehmanen
 */
class EWFFILE : public AUDIO_IO {

  WAVEFILE* wobject;

  bool wave_object_active;
  long int sample_offset;

  string wavename, ewfname;
    
  void read_ewf_parameters(void);
  void write_ewf_parameters(void);
  void seek_position_in_samples(long pos);
  
  // attributes from .ewf data file...:
  
  EWFFILE& operator=(const EWFFILE& x) { return *this; }

 public:
    
  bool finished(void) const { return(wobject->finished()); }
  bool is_realtime(void) const { return(false); }

  void buffersize(long int samples, long int sample_rate) { wobject->buffersize(samples, sample_rate); }
  long int buffersize(void) const { return(wobject->buffersize()); }

  void read_buffer(SAMPLE_BUFFER* sbuf);
  void write_buffer(SAMPLE_BUFFER* sbuf);

  //  long int read_samples(void* target_buffer, long int samples) { return(0); }
  //  void write_samples(void* target_buffer, long int samples) { }

  void seek_position(void);
  void buffersize_changed(void) { wobject->buffersize(buffersize(), samples_per_second()); }
        
  void open(void);
  void close(void);
 
  EWFFILE* clone(void) { return new EWFFILE(*this); }
  EWFFILE (const string& name, 
	   const SIMODE mode, 
	   const ECA_AUDIO_FORMAT& form) throw(ECA_ERROR*);
  ~EWFFILE(void);
};

#endif
