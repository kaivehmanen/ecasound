#ifndef _AUDIOFX_AMPLITUDE_H
#define _AUDIOFX_AMPLITUDE_H

#include <vector>

#include "samplebuffer_iterators.h"
#include "audiofx.h"

/**
 * Virtual base for amplitude effects and dynamic processors.
 * @author Kai Vehmanen
 */
class EFFECT_AMPLITUDE : public EFFECT_BASE {

 public:
  virtual ~EFFECT_AMPLITUDE(void) { }
};

#include "audiofx_compressor.h"

/**
 * Normal amplifier
 * @author Kai Vehmanen
 */
class EFFECT_AMPLIFY: public EFFECT_AMPLITUDE {

  parameter_type kerroin;
  SAMPLE_ITERATOR i;

 public:

  string name(void) const { return("Amplify"); }

  string parameter_names(void) const  { return("amp-%"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_AMPLIFY (parameter_type multiplier_percent = 100.0);
  virtual ~EFFECT_AMPLIFY(void) { }
  EFFECT_AMPLIFY* clone(void)  { return new EFFECT_AMPLIFY(*this); }
};

/**
 * Amplifier with clip control.
 * @author Kai Vehmanen
 */
class EFFECT_AMPLIFY_CLIPCOUNT : public EFFECT_AMPLITUDE {

  parameter_type kerroin;
  int nm, num_of_clipped, maxnum_of_clipped;
  SAMPLE_ITERATOR i;

 public:

  string name(void) const { return("Amplify with clipping control"); }

  string parameter_names(void) const { return("amp-%,max-clipped-samples"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_AMPLIFY_CLIPCOUNT* clone(void)  { return new EFFECT_AMPLIFY_CLIPCOUNT(*this); }
  EFFECT_AMPLIFY_CLIPCOUNT (parameter_type multiplier_percent = 100.0, int max_clipped = 0);
};

/**
 * Channel amplifier
 * @author Kai Vehmanen
 */
class EFFECT_AMPLIFY_CHANNEL: public EFFECT_AMPLITUDE {

  parameter_type kerroin;
  int channel_rep;
  SAMPLE_ITERATOR_CHANNEL i;

 public:

  string name(void) const { return("Channel amplify"); }

  string parameter_names(void) const  { return("amp-%,channel"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_AMPLIFY_CHANNEL* clone(void)  { return new EFFECT_AMPLIFY_CHANNEL(*this); }
  EFFECT_AMPLIFY_CHANNEL (parameter_type multiplier_percent = 100.0, int channel = 1);
};

/**
 * Dynamic compressor.
 * @author Kai Vehmanen
 */
class EFFECT_COMPRESS : public EFFECT_AMPLITUDE {

  parameter_type crate;
  parameter_type threshold;
  SAMPLE_ITERATOR_CHANNELS i;

  parameter_type delta, ratio, new_value;
  bool first_time;

  vector<SAMPLE_SPECS::sample_type> lastin, lastout;

 public:

  string name(void) const { return("Compress"); }

  string parameter_names(void) const  { return("compression-rate-dB,threshold-%"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_COMPRESS* clone(void)  { return new EFFECT_COMPRESS(*this); }
  EFFECT_COMPRESS (const EFFECT_COMPRESS& x);
  EFFECT_COMPRESS (parameter_type compress_rate, parameter_type thold);
  EFFECT_COMPRESS (void) : first_time(true) { 
    map_parameters();
  }
};

/**
 * Noise gate with attack and release
 * @author Kai Vehmanen
 */
class EFFECT_NOISEGATE : public EFFECT_AMPLITUDE {

  SAMPLE_ITERATOR_CHANNELS i;

  parameter_type th_level;
  parameter_type th_time;
  parameter_type atime, htime, rtime;
  
  vector<parameter_type> th_time_lask;
  vector<parameter_type> attack_lask;
  vector<parameter_type> hold_lask;
  vector<parameter_type> release_lask;
  vector<parameter_type> kerroin;

  enum { ng_waiting, ng_attacking, ng_active, ng_holding, ng_releasing };

  vector<int> ng_status;
  
 public:
  
  string name(void) const { return("Noisegate"); }

  string parameter_names(void) const {
    return("threshold-level-%,pre-hold-time-msec,attack-time-msec,post-hold-time-msec,release-time-msec");
  }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_NOISEGATE* clone(void)  { return new EFFECT_NOISEGATE(*this); }
  EFFECT_NOISEGATE (parameter_type thlevel_percent, parameter_type thtime, parameter_type atime, parameter_type htime, parameter_type rtime);
  EFFECT_NOISEGATE (void) { 
    map_parameters();
  }
};

/**
 * Panning effect for controlling the stereo image.
 * @author Kai Vehmanen
 */
class EFFECT_NORMAL_PAN : public EFFECT_AMPLITUDE {

private:

  SAMPLE_ITERATOR_CHANNEL i;

  parameter_type right_percent_rep;
  parameter_type l_kerroin, r_kerroin;
  
public:

  string name(void) const { return("Normal pan"); }

  string parameter_names(void) const { return("right-%"); }
    
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);
    
  EFFECT_NORMAL_PAN* clone(void)  { return new EFFECT_NORMAL_PAN(*this); }
  EFFECT_NORMAL_PAN(parameter_type right_percent = 50.0);
};

#endif
