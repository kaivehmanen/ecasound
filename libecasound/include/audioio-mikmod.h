#ifndef _AUDIOIO_MIKMOD_H
#define _AUDIOIO_MIMOD_H

#include <string>
#include "audioio-types.h"

class SAMPLE_BUFFER;

/**
 * Interface class for MikMod input. Uses FIFO pipes.
 * @author Kai Vehmanen
 */
class MIKMOD_INTERFACE : public AUDIO_IO_BUFFERED {

 private:
  
  static string default_mikmod_path;
  static string default_mikmod_args;

 public:

  static void set_mikmod_path(const string& value);
  static void set_mikmod_args(const string& value);

 private:

  bool finished_rep;
  int pid_of_child;
  long int bytes_read;
  int fd;
  
  void seek_position_in_samples(long pos);
  MIKMOD_INTERFACE& operator=(const MIKMOD_INTERFACE& x) { return *this; }

  void fork_mikmod(void) throw(ECA_ERROR*);
  void kill_mikmod(void);
  
 public:

  string name(void) const { return("MikMod tracker module"); }
  int supported_io_modes(void) const { return(io_read); }

  void open(void);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples) { }

  bool finished(void) const { return(finished_rep); }
  void seek_position(void);
 
  MIKMOD_INTERFACE (const string& name = "");
  ~MIKMOD_INTERFACE(void);
    
  MIKMOD_INTERFACE* clone(void) { return new MIKMOD_INTERFACE(*this); }
  MIKMOD_INTERFACE* new_expr(void) { return new MIKMOD_INTERFACE(); }
};

#endif
