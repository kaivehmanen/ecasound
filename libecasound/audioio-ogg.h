#ifndef INCLUDED_AUDIOIO_OGG_H
#define INCLUDED_AUDIOIO_OGG_H

#include <string>
#include <cstdio>
#include "audioio-types.h"
#include "samplebuffer.h"
#include "audioio-forked-stream.h"

/**
 * Interface to ogg decoders and encoders that support i/o 
 * using standard streams. By default ogg123 and vorbize are
 * used.
 *
 * @author Kai Vehmanen
 */
class OGG_VORBIS_INTERFACE : public AUDIO_IO_BUFFERED,
			     protected AUDIO_IO_FORKED_STREAM {

 private:
  
  static std::string default_ogg_input_cmd;
  static std::string default_ogg_output_cmd;

 public:

  static void set_ogg_input_cmd(const std::string& value);
  static void set_ogg_output_cmd(const std::string& value);

 private:

  bool triggered_rep;
  bool finished_rep;
  long int bytes_rep;
  int fd_rep;
  FILE* f1_rep;
  
  void fork_ogg_input(void);
  void fork_ogg_output(void);
  
 public:

  virtual std::string name(void) const { return("Ogg Vorbis stream"); }
  virtual std::string description(void) const { return("Interface for ogg decoders and encoders that support i/o using standard streams."); }
  virtual bool locked_audio_format(void) const { return(true); }
  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual bool supports_seeking(void) const { return(false); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR &);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual bool finished(void) const { return(finished_rep); }
  virtual void seek_position(void);

  // --
  // Realtime related functions
  // --
  
  OGG_VORBIS_INTERFACE (const std::string& name = "");
  ~OGG_VORBIS_INTERFACE(void);
    
  OGG_VORBIS_INTERFACE* clone(void) const { return new OGG_VORBIS_INTERFACE(*this); }
  OGG_VORBIS_INTERFACE* new_expr(void) const { return new OGG_VORBIS_INTERFACE(*this); }
};

#endif
