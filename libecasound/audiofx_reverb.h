#ifndef INCLUDED_AUDIOFX_REVERB_H
#define INCLUDED_AUDIOFX_REVERB_H

#include <vector>
#include "audiofx_timebased.h"

/**
 * Reverb effect
 *
 * May 2000, Stefan Fendt (C++ version by Kai Vehmanen)
 */
class ADVANCED_REVERB : public EFFECT_TIME_BASED {
 private:

  SAMPLE_ITERATOR_CHANNELS i_channels;
  long int srate_rep;
  parameter_type roomsize_rep;
  parameter_type feedback_rep;
  parameter_type wet_rep;

  class CHANNEL_DATA {
  public:
    vector<SAMPLE_SPECS::sample_type> buffer;
    vector<long int> dpos;
    vector<parameter_type> mul;
    long int bufferpos_rep;
    SAMPLE_SPECS::sample_type oldvalue, lpvalue;

    CHANNEL_DATA(void) : buffer(65536, 0.0), dpos(200), mul(200), bufferpos_rep(0) { }
  };
  vector<CHANNEL_DATA> cdata;

 public:

  virtual string name(void) const { return("Advanced reverb"); }
  virtual string parameter_names(void) const { return("Room-size,feedback-%,wet-%"); }

  virtual parameter_type get_parameter(int param) const;
  virtual void set_parameter(int param, parameter_type value);

  virtual void init(SAMPLE_BUFFER* insample);
  virtual void process(void);

  ADVANCED_REVERB* clone(void)  { return new ADVANCED_REVERB(*this); }
  ADVANCED_REVERB* new_expr(void)  { return new ADVANCED_REVERB(); }
  ADVANCED_REVERB (parameter_type roomsize = 10.0, parameter_type feedback_percent = 100.0, parameter_type wet_percent = 100.0);
};

#endif
