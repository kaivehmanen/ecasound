#ifndef INCLUDED_AUDIOIO_MP3_H
#define INCLUDED_AUDIOIO_MP3_H

#include <string>
#include <cstdio>
#include "audioio-types.h"
#include "audioio-forked-stream.h"

/**
 * Interface for mp3 decoders and encoders that support 
 * input/output using standard streams. Defaults to
 * mpg123 and lame.
 * @author Kai Vehmanen
 */
class MP3FILE : public AUDIO_IO_BUFFERED,
		protected AUDIO_IO_FORKED_STREAM {

 private:
  
  static std::string default_mp3_input_cmd;
  static std::string default_mp3_output_cmd;

 public:

  static void set_mp3_input_cmd(const std::string& value);
  static void set_mp3_output_cmd(const std::string& value);

 private:

  bool finished_rep;
  bool triggered_rep;
  int pid_of_child_rep;
  long pcm_rep;
  long int bytes_rep;
  long last_position_rep;
  int fd_rep;
  FILE* f1_rep;
  bool mono_input_rep;
  
  void process_mono_fix(char* target_buffer, long int bytes_rep);
  void get_mp3_params(const std::string& fname) throw(AUDIO_IO::SETUP_ERROR&);
  
  //  MP3FILE(const MP3FILE& x) { }
  MP3FILE& operator=(const MP3FILE& x) { return *this; }

  void fork_mp3_input(void);
  void fork_mp3_output(void);
  
 public:

  virtual std::string name(void) const { return("Mp3 stream"); }
  virtual std::string description(void) const { return("Interface for mp3 decoders and encoders that support input/output using standard streams."); }
  virtual bool locked_audio_format(void) const { return(true); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR &);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual bool finished(void) const { return(finished_rep); }
  virtual void seek_position(void);

  // --
  // Realtime related functions
  // --
  
  MP3FILE (const std::string& name = "");
  ~MP3FILE(void);
    
  MP3FILE* clone(void) const { return new MP3FILE(*this); }
  MP3FILE* new_expr(void) const { return new MP3FILE(*this); }
};

#endif
