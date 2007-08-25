#ifndef INCLUDED_AUDIOIO_TONE_H
#define INCLUDED_AUDIOIO_TONE_H

#include <string>
#include "audioio.h"
#include "samplebuffer.h"
#include "eca-audio-time.h"

/**
 * Sine generator
 *
 * @author Kai Vehmanen
 * @author Richard W.E. Furse
 */
class AUDIO_IO_TONE : public AUDIO_IO {

 public:

  /** @name Public functions */
  /*@{*/

  AUDIO_IO_TONE (const std::string& name = "");
  virtual ~AUDIO_IO_TONE(void);

  /*@}*/
  
  /** @name Reimplemented functions from ECA_OBJECT */
  /*@{*/

  virtual std::string name(void) const { return("Tone generator"); }
  virtual std::string description(void) const { return("Audio input that produces sine and other basic tones for testing and other purposes."); }

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_PARAMETERS<string> */
  /*@{*/

  /* none */

  /*@}*/

  /** @name Reimplemented functions from DYNAMIC_OBJECT<string> */
  /*@{*/

  virtual AUDIO_IO_TONE* clone(void) const;
  virtual AUDIO_IO_TONE* new_expr(void) const { return new AUDIO_IO_TONE(); }

  /*@}*/

  /** @name Reimplemented functions from ECA_AUDIO_POSITION */
  /*@{*/

  virtual void seek_position(void);

  /*@}*/

  /** @name Reimplemented functions from AUDIO_IO */
  /*@{*/

  virtual bool locked_audio_format(void) const { return false; }
  virtual bool supports_seeking(void) const { return true; }
  virtual bool finite_length_stream(void) const { return true; }

  virtual bool finished(void) const { return false; }

  virtual void read_buffer(SAMPLE_BUFFER* sbuf);
  virtual void write_buffer(SAMPLE_BUFFER* sbuf);

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);

  virtual void set_buffersize(long int samples) { buffersize_rep = samples; }
  virtual long int buffersize(void) const { return buffersize_rep; }

  /*@}*/

  /** @name New functions */
  /*@{*/

    
  /*@}*/

private:

  SAMPLE_BUFFER buffer_rep;
  long int buffersize_rep;  

  AUDIO_IO_TONE& operator=(const AUDIO_IO_TONE& x) { return *this; }
  AUDIO_IO_TONE (const AUDIO_IO_TONE& x) { }

};

#endif
