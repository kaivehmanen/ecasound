#ifndef _AUDIOIO_ALSA_H
#define _AUDIOIO_ALSA_H

#include <config.h>
#ifdef COMPILE_ALSA

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/asoundlib.h>

#include "samplebuffer.h"

/**
 * Class for handling ALSA-devices (Advanced Linux Sound Architecture).
 * @author Kai Vehmanen
 */
class ALSA_PCM_DEVICE : public AUDIO_IO_DEVICE {

#ifdef ALSALIB_031
  typedef snd_pcm_t void*;
#endif
  snd_pcm_t *audio_fd;

  int card_number, device_number;
  int pcm_mode, pcm_channel;

  long int bytes_read;

  long underruns, overruns;

  bool is_triggered;
  bool is_prepared;
  
 public:

  string name(void) const { return("ALSA PCM-v1 device"); }
  int supported_io_modes(void) const { return(io_read | io_write); }
  string parameter_names(void) const { return("label,card,device"); }

  void open(void) throw(ECA_ERROR*);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  void stop(void);
  void start(void);
  void prepare(void) { }

  long position_in_samples(void) const;

  void set_parameter(int param, string value);
  string get_parameter(int param) const;

  ALSA_PCM_DEVICE (int card = 0, int device = 0);
  ~ALSA_PCM_DEVICE(void);
  ALSA_PCM_DEVICE* clone(void) { return new ALSA_PCM_DEVICE(*this); }
  ALSA_PCM_DEVICE* new_expr(void) { return new ALSA_PCM_DEVICE(); }
  
 private:
  
  ALSA_PCM_DEVICE& operator=(const ALSA_PCM_DEVICE& x) { return *this; }
};

#endif // COMPILE_ALSA
#endif
