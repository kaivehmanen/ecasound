#ifndef INCLUDED_AUDIOIO_TIMIDITY_H
#define INCLUDED_AUDIOIO_TIMIDITY_H

#include <string>
#include "audioio-types.h"
#include "samplebuffer.h"
#include "audioio-forked-stream.h"

/**
 * Interface class for Timidity++ input. Uses FIFO pipes.
 * @author Kai Vehmanen
 */
class TIMIDITY_INTERFACE : public AUDIO_IO_BUFFERED,
			   protected AUDIO_IO_FORKED_STREAM {

 private:
  
  static string default_timidity_cmd;

 public:

  static void set_timidity_cmd(const string& value);

 private:

  bool finished_rep;
  long int bytes_read_rep;
  int fd_rep;
  
  void seek_position_in_samples(long pos);
  TIMIDITY_INTERFACE& operator=(const TIMIDITY_INTERFACE& x) { return *this; }

  void fork_timidity(void) throw(ECA_ERROR&);
  void kill_timidity(void);
  
 public:

  virtual string name(void) const { return("MIDI file input using Timidity++."); }
  virtual int supported_io_modes(void) const { return(io_read); }

  virtual void open(void);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples) { }

  virtual bool finished(void) const { return(finished_rep); }
  virtual void seek_position(void);
 
  TIMIDITY_INTERFACE (const string& name = "");
  ~TIMIDITY_INTERFACE(void);
    
  TIMIDITY_INTERFACE* clone(void) { return new TIMIDITY_INTERFACE(*this); }
  TIMIDITY_INTERFACE* new_expr(void) { return new TIMIDITY_INTERFACE(); }
};

#endif
