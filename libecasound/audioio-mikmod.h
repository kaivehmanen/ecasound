#ifndef INCLUDED_AUDIOIO_MIKMOD_H
#define INCLUDED_AUDIOIO_MIKMOD_H

#include <string>
#include "audioio-types.h"
#include "samplebuffer.h"
#include "audioio-forked-stream.h"

/**
 * Interface class for MikMod input. Uses FIFO pipes.
 * @author Kai Vehmanen
 */
class MIKMOD_INTERFACE : public AUDIO_IO_BUFFERED,
			 protected AUDIO_IO_FORKED_STREAM {

 private:
  
  static string default_mikmod_cmd;

 public:

  static void set_mikmod_cmd(const string& value);

 private:

  bool finished_rep;
  long int bytes_read_rep;
  int fd_rep;
  
  void seek_position_in_samples(long pos);
  MIKMOD_INTERFACE& operator=(const MIKMOD_INTERFACE& x) { return *this; }

  void fork_mikmod(void) throw(ECA_ERROR&);
  void kill_mikmod(void);
  
 public:

  virtual string name(void) const { return("MikMod tracker module"); }
  virtual int supported_io_modes(void) const { return(io_read); }

  virtual void open(void);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples) { }

  virtual bool finished(void) const { return(finished_rep); }
  virtual void seek_position(void);
 
  MIKMOD_INTERFACE (const string& name = "");
  ~MIKMOD_INTERFACE(void);
    
  MIKMOD_INTERFACE* clone(void) { return new MIKMOD_INTERFACE(*this); }
  MIKMOD_INTERFACE* new_expr(void) { return new MIKMOD_INTERFACE(); }
};

#endif
