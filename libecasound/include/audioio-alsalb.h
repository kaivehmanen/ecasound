#ifndef _AUDIOIO_ALSALB_H
#define _AUDIOIO_ALSALB_H

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
 * Class for handling ALSA loopback-devices (Advanced Linux Sound Architecture).
 * @author Kai Vehmanen
 */
class ALSALBDEVICE : public AUDIO_IO_DEVICE {

#if SND_LIB_VERSION<769    // if alsa-lib older than 0.3.1
  typedef snd_pcm_t void*;
#endif

  snd_pcm_loopback_t *audio_fd;

  int card_number, device_number;
  bool pb_mode;
  long int bytes_read;

  bool is_triggered;
  
 public:

  void open(void) throw(ECA_ERROR*);
  void close(void);
  
  long int read_samples(void* target_buffer, long int samples);
  void write_samples(void* target_buffer, long int samples) { }

  void stop(void) { }
  void start(void) { }

  ALSALBDEVICE (int card, int device, const SIMODE mode, const
		ECA_AUDIO_FORMAT& form, long int buffersize,
		bool playback_mode = true);
  ALSALBDEVICE::~ALSALBDEVICE(void);
  ALSALBDEVICE* clone(void) { return new ALSALBDEVICE(*this); }
  
 private:
  
  //  ALSALBDEVICE(const ALSALBDEVICE& x) { }
  ALSALBDEVICE& operator=(const ALSALBDEVICE& x) { return *this; }
};

#endif // COMPILE_ALSA
#endif
