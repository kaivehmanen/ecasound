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
  
 public:

  void open(void) throw(ECA_ERROR*);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  void stop(void);
  void start(void);

  ALSA_PCM_DEVICE (int card, int device, const SIMODE mode, const ECA_AUDIO_FORMAT& form, long int buffersize);
  ALSA_PCM_DEVICE::~ALSA_PCM_DEVICE(void);
  ALSA_PCM_DEVICE* clone(void) { return new ALSA_PCM_DEVICE(*this); }
  
 private:
  
  ALSA_PCM_DEVICE& operator=(const ALSA_PCM_DEVICE& x) { return *this; }
};

#endif // COMPILE_ALSA
#endif
