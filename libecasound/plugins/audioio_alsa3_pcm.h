#ifndef INCLUDED_AUDIOIO_ALSA3_PCM_H
#define INCLUDED_AUDIOIO_ALSA3_PCM_H

#include "audioio_alsa3.h"

/**
 * Class for handling named ALSA pcm-devices (Advanced Linux Sound Architecture).
 */
class ALSA_NAMED_PCM_DEVICE_06X : public ALSA_PCM_DEVICE_06X {

 public:

  virtual string name(void) const { return("ALSA named PCM device"); }
  virtual string description(void) const { return("ALSA named PCM devices. Library versions 0.6.x and newer."); }

  virtual string parameter_names(void) const { return("label,pcm_name"); }
  virtual void set_parameter(int param, string value);
  virtual string get_parameter(int param) const;

  ALSA_NAMED_PCM_DEVICE_06X (void);
  ~ALSA_NAMED_PCM_DEVICE_06X(void) { }
  ALSA_NAMED_PCM_DEVICE_06X* clone(void) { cerr << "Not implemented!" << endl; return this; }
  ALSA_NAMED_PCM_DEVICE_06X* new_expr(void) { return new ALSA_NAMED_PCM_DEVICE_06X(); }
  
 private:

  void print_status_debug(void);
  ALSA_NAMED_PCM_DEVICE_06X (const ALSA_NAMED_PCM_DEVICE_06X& x) { }
  ALSA_NAMED_PCM_DEVICE_06X& operator=(const ALSA_NAMED_PCM_DEVICE_06X& x) { return *this; }
};

#endif
