#ifndef _AUDIOIO_MP3_H
#define _AUDIOIO_MP3_H

#include <string>
#include "audioio-types.h"
#include "samplebuffer.h"

/**
 * Interface class for MP3 input and output. Uses FIFO pipes to communicate 
 * with mpg123 (input) and lame (output).
 * @author Kai Vehmanen
 */
class MP3FILE : public AUDIO_IO_BUFFERED {

 private:
  
  static string default_mpg123_path;
  static string default_mpg123_args;

  static string default_lame_path;
  static string default_lame_args;

 public:

  static void set_mpg123_path(const string& value);
  static void set_mpg123_args(const string& value);

  static void set_lame_path(const string& value);
  static void set_lame_args(const string& value);

 private:

  bool finished_rep;
  int pid_of_child;
  long pcm;
  long int bytes;
  int fd;
  
  void get_mp3_params(const string& fname) throw(ECA_ERROR*);
  
  //  MP3FILE(const MP3FILE& x) { }
  MP3FILE& operator=(const MP3FILE& x) { return *this; }

  void fork_mpg123(void) throw(ECA_ERROR*);
  void kill_mpg123(void);
  void fork_lame(void) throw(ECA_ERROR*);
  void kill_lame(void);
  
 public:

  string name(void) const { return("MP3 file"); }

  void open(void);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  bool finished(void) const { return(finished_rep); }
  void seek_position(void);

  // --
  // Realtime related functions
  // --
  
  MP3FILE (const string& name = "");
  ~MP3FILE(void);
    
  //    MP3FILE* new_expr(void) { return new MP3FILE(); }
  MP3FILE* clone(void) { return new MP3FILE(*this); }
  MP3FILE* new_expr(void) { return new MP3FILE(*this); }
};

#endif

