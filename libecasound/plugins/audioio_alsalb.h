#ifndef INCLUDED_AUDIOIO_ALSALB_H
#define INCLUDED_AUDIOIO_ALSALB_H

#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if (defined ALSALIB_032 || defined ALSALIB_050)
#include <sys/asoundlib.h>

#include "samplebuffer.h"

/**
 * Class for handling ALSA loopback-devices (Advanced Linux Sound Architecture).
 * @author Kai Vehmanen
 */
class ALSA_LOOPBACK_DEVICE : public AUDIO_IO_DEVICE {

  snd_pcm_loopback_t *audio_fd;

  int card_number, device_number, subdevice_number;
  bool pb_mode;
  long int bytes_read;

  bool is_triggered;
  
 public:

  virtual string name(void) const { return("ALSA PCM-loopback device"); }
  virtual int supported_io_modes(void) const { return(io_read); }

#ifdef ALSALIB_050
  virtual string parameter_names(void) const { return("label,card,device,subdevice"); }
#else
  virtual string parameter_names(void) const { return("label,card,device"); }
#endif

  virtual void open(void) throw(ECA_ERROR*);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples) { }

  virtual void stop(void) { }
  virtual void start(void) { }
  virtual void prepare(void) { }

  virtual long position_in_samples(void) const { return(0); }

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  ALSA_LOOPBACK_DEVICE (int card = 0, int device = 0, bool playback_mode = true);
  ~ALSA_LOOPBACK_DEVICE(void);
  ALSA_LOOPBACK_DEVICE* clone(void) { cerr << "Not implemented!" << endl; return this; }
  ALSA_LOOPBACK_DEVICE* new_expr(void) { return new ALSA_LOOPBACK_DEVICE(); }
  
 private:
  
  ALSA_LOOPBACK_DEVICE (const ALSA_LOOPBACK_DEVICE& x) { }
  ALSA_LOOPBACK_DEVICE& operator=(const ALSA_LOOPBACK_DEVICE& x) { return *this; }
};

#endif // (defined ALSALIB_032 || defined ALSALIB_050)
#endif
