#ifndef INCLUDED_AUDIOIO_ALSA2_H
#define INCLUDED_AUDIOIO_ALSA2_H

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
#include <sys/asoundlib.h>
#endif

#include "samplebuffer.h"
#include "audioio-types.h"

/**
 * Class for handling ALSA PCM-devices (Advanced Linux Sound Architecture).
 * @author Kai Vehmanen
 */
class ALSA_PCM_DEVICE_05X : public AUDIO_IO_DEVICE {

#ifdef ALSALIB_050
  snd_pcm_t *audio_fd_repp;
  snd_pcm_channel_info_t pcm_info_rep;
#endif

  long int fragment_size_rep;
 
  int card_number_rep, device_number_rep, subdevice_number_rep;
  int pcm_mode_rep, pcm_channel_rep;

  long int bytes_read_rep;
  long underruns_rep, overruns_rep;

  bool is_triggered_rep;
  bool is_prepared_rep;

 public:

  virtual string name(void) const { return("ALSA PCM-v2 device"); }
  virtual string description(void) const { return("ALSA PCM-v2 devices. Library version 0.5.x."); }

  virtual int supported_io_modes(void) const { return(io_read | io_write); }
  virtual string parameter_names(void) const { return("label,card,device,subdevice"); }

  virtual void open(void) throw(AUDIO_IO::SETUP_ERROR&);
  virtual void close(void);
  
  virtual long int read_samples(void* target_buffer, long int samples);
  virtual void write_samples(void* target_buffer, long int samples);

  virtual void stop(void);
  virtual void start(void);
  void prepare(void);

  virtual long position_in_samples(void) const;

  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  ALSA_PCM_DEVICE_05X (int card = 0, int device = 0, int subdevice = -1);
  ~ALSA_PCM_DEVICE_05X(void);
  ALSA_PCM_DEVICE_05X* clone(void) const { cerr << "Not implemented!" << endl; return this; }
  ALSA_PCM_DEVICE_05X* new_expr(void) const { return new ALSA_PCM_DEVICE_05X(); }
  
 private:

  void print_status_debug(void);
  ALSA_PCM_DEVICE_05X (const ALSA_PCM_DEVICE_05X& x) { }
  ALSA_PCM_DEVICE_05X& operator=(const ALSA_PCM_DEVICE_05X& x) { return *this; }
};

extern "C" {
AUDIO_IO* audio_io_descriptor(void) { return(new ALSA_PCM_DEVICE_05X()); }
int audio_io_interface_version(void);
const char* audio_io_keyword(void);
const char* audio_io_keyword_regex(void);
};

#endif
