#ifndef INCLUDED_AUDIOIO_ALSA2_PLUGIN_H
#define INCLUDED_AUDIOIO_ALSA2_PLUGIN_H

#include <string>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ALSALIB_050
extern "C" {
#include <sys/asoundlib.h>
}
#endif

#include "sample-specs.h"
#include "samplebuffer.h"
#include "audioio-device.h"

/**
 * Class for handling ALSA pcm2-devices using the pcm-plugin API.
 * @author Kai Vehmanen
 */
class ALSA_PCM2_PLUGIN_DEVICE : public AUDIO_IO_DEVICE {

#ifdef ALSALIB_050
  snd_pcm_t *audio_fd_repp;
  snd_pcm_channel_info_t pcm_info_rep;
#endif

  long int fragment_size_rep;
  int card_number_rep, device_number_rep, subdevice_number_rep;
  int pcm_mode_rep, pcm_channel_rep;

  long underruns_rep, overruns_rep;

  bool is_triggered_rep;
  bool is_prepared_rep;

 public:

  virtual string name(void) const { return("ALSA PCM-plugin device"); }
  virtual string description(void) const { return("ALSA PCM-plugin devices. Library versions 0.5.x and newer."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual string parameter_names(void) const { return("label,card,device,subdevice"); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  void prepare(void);

  virtual SAMPLE_SPECS::sample_pos_t position_in_samples(void) const;

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  ALSA_PCM2_PLUGIN_DEVICE (int card = 0, int device = 0, int subdevice = -1);
  ~ALSA_PCM2_PLUGIN_DEVICE(void);
  ALSA_PCM2_PLUGIN_DEVICE* clone(void) const { cerr << "Not implemented!" << endl; return 0; }
  ALSA_PCM2_PLUGIN_DEVICE* new_expr(void) const { return new ALSA_PCM2_PLUGIN_DEVICE(); }
  
 private:

  void print_status_debug(void);
  ALSA_PCM2_PLUGIN_DEVICE (const ALSA_PCM2_PLUGIN_DEVICE& x) { }
  ALSA_PCM2_PLUGIN_DEVICE& operator=(const ALSA_PCM2_PLUGIN_DEVICE& x) { return *this; }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void) { return(new ALSA_PCM2_PLUGIN_DEVICE()); }
int audio_io_interface_version(void);
const char* audio_io_keyword(void);
const char* audio_io_keyword_regex(void);
};

#endif
