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
class ALSADEVICE : public AUDIO_IO_DEVICE {

// alsa-lib older than 0.3.1
#if SND_LIB_VERSION<769    
  typedef snd_pcm_t void*;
#endif

  snd_pcm_t *audio_fd;

  //  static int (ALSADEVICE::*snd_pcm_open)(snd_pcm_t **,int,int,int);

  int card_number, device_number;

  long int bytes_read;

  long underruns, overruns;

  bool is_triggered;
  
 public:

  void open(void);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples);

  void stop(void);
  void start(void);

  ALSADEVICE (int card, int device, const SIMODE mode, const ECA_AUDIO_FORMAT& form, long int buffersize);
  ALSADEVICE::~ALSADEVICE(void);
  ALSADEVICE* clone(void) { return new ALSADEVICE(*this); }
  
 private:
  
  //  ALSADEVICE(const ALSADEVICE& x) { }
  ALSADEVICE& operator=(const ALSADEVICE& x) { return *this; }
};

#endif // COMPILE_ALSA
#endif



