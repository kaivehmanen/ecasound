#ifndef _AUDIOIO_AF_H
#define _AUDIOIO_AF_H

#include <config.h>
#ifdef COMPILE_AF

#include <string>
#include <audiofile.h>
#include "samplebuffer.h"

#include "audioio-types.h"
#include "samplebuffer.h"

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

  string name(void) const { return("SGI libaudiofile object"); }
  int supported_io_modes(void) const { return(io_read | io_write); }
  virtual bool locked_audio_format(void) const { return(true); }

  
  void open(void) throw(ECA_ERROR*);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  bool finished(void) const;
  void seek_position(void);
    
  AUDIOFILE_INTERFACE* clone(void) { return new AUDIOFILE_INTERFACE(*this); }
  AUDIOFILE_INTERFACE* new_expr(void) { return new AUDIOFILE_INTERFACE(); }  

  AUDIOFILE_INTERFACE (const string& name = "");
  ~AUDIOFILE_INTERFACE(void);
};

#endif
#endif


