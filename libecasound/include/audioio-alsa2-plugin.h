#ifndef _AUDIOIO_ALSA2_PLUGIN_H
#define _AUDIOIO_ALSA2_PLUGIN_H

#include <config.h>
#ifdef COMPILE_ALSA

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

extern "C" {
#include <sys/asoundlib.h>
}

#include "samplebuffer.h"
#include "audioio-types.h"

/**
 * Class for handling ALSA pcm2-devices using the pcm-plugin API.
 * @author Kai Vehmanen
 */
class ALSA_PCM2_PLUGIN_DEVICE : public AUDIO_IO_DEVICE {

  snd_pcm_t *audio_fd;
  snd_pcm_channel_info_t pcm_info;

  long int fragment_size;
  int card_number, device_number, subdevice_number;
  int pcm_mode, pcm_channel;

  long underruns, overruns;

  bool is_triggered;
  bool is_prepared;

 public:

  virtual string name(void) const { return("ALSA PCM-plugin device"); }
  virtual string description(void) const { return("ALSA PCM-plugin devices. Library versions 0.5.x and newer."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual string parameter_names(void) const { return("label,card,device,subdevice"); }

  virtual void open(void) throw(ECA_ERROR*);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  void prepare(void);

  virtual long position_in_samples(void) const;

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  ALSA_PCM2_PLUGIN_DEVICE (int card = 0, int device = 0, int subdevice = -1);
  ~ALSA_PCM2_PLUGIN_DEVICE(void);
  ALSA_PCM2_PLUGIN_DEVICE* clone(void) { cerr << "Not implemented!" << endl; return this; }
  ALSA_PCM2_PLUGIN_DEVICE* new_expr(void) { return new ALSA_PCM2_PLUGIN_DEVICE(); }
  
 private:

  void print_status_debug(void);
  ALSA_PCM2_PLUGIN_DEVICE (const ALSA_PCM2_PLUGIN_DEVICE& x) { }
  ALSA_PCM2_PLUGIN_DEVICE& operator=(const ALSA_PCM2_PLUGIN_DEVICE& x) { return *this; }
};

#endif // COMPILE_ALSA
#endif
