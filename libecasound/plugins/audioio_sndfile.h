#ifndef INCLUDED_AUDIOIO_SNDFILE_H
#define INCLUDED_AUDIOIO_SNDFILE_H

#include <string>
#include <sndfile.h>
#include "samplebuffer.h"

#include "audioio-buffered.h"
#include "samplebuffer.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/**
 * Interface to SGI audiofile library.
 * @author Kai Vehmanen
 */
class SNDFILE_INTERFACE : public AUDIO_IO_BUFFERED {

 public:

  /** @name Public functions */
  /*@{*/

  SNDFILE_INTERFACE (const string& name = "");
  ~SNDFILE_INTERFACE(void);

  /*@}*/
  
  /** @name Reimplemented functions from ECA_OBJECT */
  /*@{*/

  virtual string name(void) const { return("Libsndfile object"); }
  virtual string description(void) const { return("Libsndfile object. Supports all commona audio formats."); }

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_PARAMETERS<string> */
  /*@{*/

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_OBJECT<string> */
  /*@{*/

  SNDFILE_INTERFACE* clone(void) const;
  SNDFILE_INTERFACE* new_expr(void) const { return new SNDFILE_INTERFACE(); }  

  /*@}*/

  /** @name Reimplemented functions from ECA_AUDIO_POSITION */
  /*@{*/

  virtual void seek_position(void);

  /*@}*/

  /** @name Function reimplemented from AUDIO_IO_BUFFERED */
  /*@{*/

  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  /*@}*/

  /** @name Function reimplemented from AUDIO_IO */
  /*@{*/

  virtual int supported_io_modes(void) const { return(io_read | io_write | io_readwrite); }
  virtual string parameter_names(void) const { return("filename,opt_filename"); }
  virtual bool locked_audio_format(void) const { return(true); }
  
  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);
  
  virtual bool finished(void) const;

  /*@}*/
    
private:

  std::string opt_filename_rep;
  SNDFILE* snd_repp;
  long samples_read;
  bool finished_rep;

  void open_parse_info(const SF_INFO* sfinfo) throw(AUDIO_IO::SETUP_ERROR&);

  SNDFILE_INTERFACE& operator=(const SNDFILE_INTERFACE& x) { return *this; }
};

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
extern "C" {
AUDIO_IO* audio_io_descriptor(void) { return(new SNDFILE_INTERFACE()); }
int audio_io_interface_version(void);
const char* audio_io_keyword(void);
const char* audio_io_keyword_regex(void);
};
#endif

#endif
