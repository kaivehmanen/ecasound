#ifndef INCLUDED_AUDIOIO_ALSA_H
#define INCLUDED_AUDIOIO_ALSA_H

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef ALSALIB_032
#include <sys/asoundlib.h>
#endif

#include "samplebuffer.h"

/**
 * Class for handling ALSA-devices (Advanced Linux Sound Architecture).
 * @author Kai Vehmanen
 */
class ALSA_PCM_DEVICE : public AUDIO_IO_DEVICE {

 private:

#ifdef ALSALIB_032
  snd_pcm_t *audio_fd_repp;
#endif

  int card_number_rep, device_number_rep;
  int pcm_mode_rep, pcm_channel_rep;

  long int bytes_read_rep;

  long underruns_rep, overruns_rep;

  bool is_triggered_rep;
  bool is_prepared_rep;
  
 public:

  virtual string name(void) const { return("ALSA PCM device"); }
  virtual string description(void) const { return("ALSA PCM devices. Library versions 0.4.x and older."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual string parameter_names(void) const { return("label,card,device"); }

  virtual void open(void) throw(ECA_ERROR&);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  virtual void prepare(void) { }

  virtual long position_in_samples(void) const;

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  ALSA_PCM_DEVICE (int card = 0, int device = 0);
  ~ALSA_PCM_DEVICE(void);
  ALSA_PCM_DEVICE* clone(void) { cerr << "Not implemented!" << endl; return this; }
  ALSA_PCM_DEVICE* new_expr(void) { return new ALSA_PCM_DEVICE(); }
  
 private:
  
  ALSA_PCM_DEVICE (const ALSA_PCM_DEVICE& x) { }
  ALSA_PCM_DEVICE& operator=(const ALSA_PCM_DEVICE& x) { return *this; }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void) { return(new ALSA_PCM_DEVICE()); }
};

#endif

