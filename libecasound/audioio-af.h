#ifndef _AUDIOIO_AF_H
#define _AUDIOIO_AF_H

#include <string>
#include <audiofile.h>
#include "samplebuffer.h"

#include "audioio-types.h"
#include "samplebuffer.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef COMPILE_AF

/**
 * Interface to SGI audiofile library.
 * @author Kai Vehmanen
 */
class AUDIOFILE_INTERFACE : public AUDIO_IO_BUFFERED {

  long samples_read;
  bool finished_rep;
  AFfilehandle afhandle;

  AUDIOFILE_INTERFACE& operator=(const AUDIOFILE_INTERFACE& x) {
    return *this; }
  void debug_print_type(void);
  
  /**
   * Do a info query prior to actually opening the device.
   *
   * require:
   *  !is_open()
   *
   * ensure:
   *  !is_open()
   */
  void format_query(void) throw(ECA_ERROR*);
  
 public:

  virtual string name(void) const { return("SGI libaudiofile object"); }
  virtual string description(void) const { return("SGI libaudiofile object. Supports AIFF (.aiff, .aifc, .aif) and Sun/NeXT audio files (.au, .snd)."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual bool locked_audio_format(void) const { return(true); }
  
  virtual void open(void) throw(ECA_ERROR*);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual bool finished(void) const;
  virtual void seek_position(void);
    
  AUDIOFILE_INTERFACE* clone(void) { return new AUDIOFILE_INTERFACE(*this); }
  AUDIOFILE_INTERFACE* new_expr(void) { return new AUDIOFILE_INTERFACE(); }  

  AUDIOFILE_INTERFACE (const string& name = "");
  ~AUDIOFILE_INTERFACE(void);
};

#endif
#endif
