#ifndef _AUDIOIO_EWF_H
#define _AUDIOIO_EWF_H

#include <string>
#include "audioio.h"
#include "audioio-wave.h"
#include "samplebuffer.h"
#include "eca-audio-time.h"
#include "resource-file.h"

/**
 * Ecasound Wave File - a simple wrapper class for handling 
 * other audio objects. When writing .ewf files, it's possible to 
 * seek beyond end position. When first write_buffer() call is issued, 
 * current sample offset is stored into the .ewf file and corresponding 
 * child object is opened for writing. Read_buffer() calls return silent 
 * buffers until sample_offset is reached. After that, audio object is 
 * processed normally. Similarly .ewf supports audio relocation, looping, etc...
 * @author Kai Vehmanen
 */
class EWFFILE : public AUDIO_IO {

  AUDIO_IO* child;
  SAMPLE_BUFFER tmp_buffer;

  bool child_looping_rep;
  ECA_AUDIO_TIME child_offset_rep,
                 child_start_pos_rep,
                 child_length_rep;
  string child_name_rep;
  bool child_active;

  RESOURCE_FILE ewf_rc;
    
  void read_ewf_data(void) throw(ECA_ERROR*);
  void write_ewf_data(void);
  void init_default_child(void) throw(ECA_ERROR*);
  
  EWFFILE& operator=(const EWFFILE& x) { return *this; }

 public:

  string name(void) const { return("Ecasound wave file"); }
  virtual bool locked_audio_format(void) const { return(true); }

  /**
   * Set start offset for child object
   */
  void child_offset(const ECA_AUDIO_TIME& v);

  /**
   * Set start position inside child object.
   */
  void child_start_position(const ECA_AUDIO_TIME& v);

  /**
   * Set child length. If not set, defaults to the total length. 
   */
  void child_length(const ECA_AUDIO_TIME& v);

  /**
   * Toggle whether child object data is looped.
   */
  void toggle_looping(bool v) { child_looping_rep = v; }
    
  bool finished(void) const;
  long length_in_samples(void) const;

  void buffersize(long int samples, long int sample_rate) { if (child != 0) child->buffersize(samples, sample_rate); }
  long int buffersize(void) const { if (child != 0) return(child->buffersize()); return(0); }
  void buffersize_changed(void) { if (child != 0) child->buffersize(buffersize(), samples_per_second()); }

  void read_buffer(SAMPLE_BUFFER* sbuf);
  void write_buffer(SAMPLE_BUFFER* sbuf);

  void seek_position(void);
  void open(void) throw(ECA_ERROR*);
  void close(void);
 
  EWFFILE* clone(void) { return new EWFFILE(*this); }
  EWFFILE* new_expr(void) { return new EWFFILE(); }
  EWFFILE (const string& name = "") { label(name); child = 0; }
  ~EWFFILE(void);
};

#endif
