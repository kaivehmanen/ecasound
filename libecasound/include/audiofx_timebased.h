#ifndef _AUDIOFX_TIMEBASED_H
#define _AUDIOFX_TIMEBASED_H

#include <vector>
#include <deque>
#include <string>

#include "audiofx.h"

typedef deque<SAMPLE_SPECS::sample_type> SINGLE_BUFFER;

/**
 * Base class for time-based effects (delays, reverbs, etc).
 */
class EFFECT_TIME_BASED : public EFFECT_BASE {

 protected:
    
  parameter_type dtime;        // delay time in samples
  parameter_type laskuri;
  
 public:

  EFFECT_TIME_BASED (void) : dtime(0.0), laskuri(0.0) { }
  virtual EFFECT_TIME_BASED* clone(void) = 0;
};

/** 
 * Delay effect.
 */
class EFFECT_DELAY : public EFFECT_TIME_BASED {

 private:

  SAMPLE_ITERATOR_CHANNEL l,r;

  parameter_type surround;
  parameter_type dnum;
  parameter_type mix;

  vector<vector<SINGLE_BUFFER> > buffer;

 public:

  string name(void) const { return("Delay"); }

  string parameter_names(void) const { return("delay_time,surround_mode,number_of_delays,mix_%"); }

  parameter_type get_parameter(int param) const;
  void set_parameter(int param, parameter_type value);

  void init(SAMPLE_BUFFER* insample);
  void process(void);

  parameter_type get_delta_in_samples(void) { return(dnum * dtime); }

  EFFECT_DELAY* clone(void)  { return new EFFECT_DELAY(*this); }
  EFFECT_DELAY (parameter_type delay_time = 0.0, int surround_mode = 0, int num_of_delays = 1, parameter_type mix_percent = 50.0);
};

/*
 * Transforms a mono signal to stereo using a panned delay signal.
 * Suitable delays values range from 1 to 40 milliseconds. 
 */
class EFFECT_FAKE_STEREO : public EFFECT_TIME_BASED {

  vector<deque<SAMPLE_SPECS::sample_type> > buffer;
  SAMPLE_ITERATOR_CHANNEL l,r;

 public:

  string name(void) const { return("Fake stereo"); }

  string parameter_names(void) const { return("delay_time"); }

  parameter_type get_parameter(int param) const;
  void set_parameter(int param, parameter_type value);

  void init(SAMPLE_BUFFER* insample);
  void process(void);

  EFFECT_FAKE_STEREO* clone(void)  { return new EFFECT_FAKE_STEREO(*this); }
  EFFECT_FAKE_STEREO (parameter_type delay_time = 0.0);
};

/*
 * Simple reverb effect.
 */
class EFFECT_REVERB : public EFFECT_TIME_BASED {

 private:
    
  vector<deque<SAMPLE_SPECS::sample_type>  > buffer;
  SAMPLE_ITERATOR_CHANNEL l,r;

  parameter_type surround;
  parameter_type feedback;

 public:

  string name(void) const { return("Reverb"); }

  string parameter_names(void) const { return("delay_time,surround_mode,feedback_%"); }

  parameter_type get_parameter(int param) const;
  void set_parameter(int param, parameter_type value);

  void init(SAMPLE_BUFFER* insample);
  void process(void);

  parameter_type get_delta_in_samples(void) { return(dtime); }

  EFFECT_REVERB* clone(void)  { return new EFFECT_REVERB(*this); }
  EFFECT_REVERB (parameter_type delay_time = 0.0, int surround_mode = 0, parameter_type feedback_percent = 50.0);
};

#endif
