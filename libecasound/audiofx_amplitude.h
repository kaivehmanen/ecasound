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

  virtual string name(void) const { return("Amplify"); }

  virtual string parameter_names(void) const  { return("amp-%"); }

  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_AMPLIFY (parameter_type multiplier_percent = 100.0);
  virtual ~EFFECT_AMPLIFY(void) { }
  EFFECT_AMPLIFY* clone(void)  { return new EFFECT_AMPLIFY(*this); }
  EFFECT_AMPLIFY* new_expr(void)  { return new EFFECT_AMPLIFY(); }
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

  virtual string name(void) const { return("Amplify with clipping control"); }

  virtual string parameter_names(void) const { return("amp-%,max-clipped-samples"); }

  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_AMPLIFY_CLIPCOUNT* new_expr(void)  { return new EFFECT_AMPLIFY_CLIPCOUNT(); }
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

  virtual string name(void) const { return("Channel amplify"); }

  string parameter_names(void) const  { return("amp-%,channel"); }

  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_AMPLIFY_CHANNEL* clone(void)  { return new EFFECT_AMPLIFY_CHANNEL(*this); }
  EFFECT_AMPLIFY_CHANNEL* new_expr(void)  { return new EFFECT_AMPLIFY_CHANNEL(); }
  EFFECT_AMPLIFY_CHANNEL (parameter_type multiplier_percent = 100.0, int channel = 1);
};

/**
 * Limiter effect
 * @author Kai Vehmanen
 */
class EFFECT_LIMITER: public EFFECT_AMPLITUDE {

  parameter_type limit_rep;
  SAMPLE_ITERATOR i;

 public:

  virtual string name(void) const { return("Limiter"); }
  virtual string parameter_names(void) const  { return("limit-%"); }

  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_LIMITER (parameter_type multiplier_percent = 100.0);
  virtual ~EFFECT_LIMITER(void) { }
  EFFECT_LIMITER* clone(void)  { return new EFFECT_LIMITER(*this); }
  EFFECT_LIMITER* new_expr(void)  { return new EFFECT_LIMITER(); }
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

  virtual string name(void) const { return("Compressor"); }

  virtual string parameter_names(void) const  { return("compression-rate-dB,threshold-%"); }

  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_COMPRESS* clone(void)  { return new EFFECT_COMPRESS(*this); }
  EFFECT_COMPRESS* new_expr(void)  { return new EFFECT_COMPRESS(); }
  EFFECT_COMPRESS (const EFFECT_COMPRESS& x);
  EFFECT_COMPRESS (parameter_type compress_rate = 1.0, parameter_type thold = 10.0);
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
  
  virtual string name(void) const { return("Noisegate"); }
  virtual string description(void) const { return("Noise gate with attack and release."); }

  virtual string parameter_names(void) const {
    return("threshold-level-%,pre-hold-time-msec,attack-time-msec,post-hold-time-msec,release-time-msec");
  }

  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);

  EFFECT_NOISEGATE* clone(void)  { return new EFFECT_NOISEGATE(*this); }
  EFFECT_NOISEGATE* new_expr(void)  { return new EFFECT_NOISEGATE(); }
  EFFECT_NOISEGATE (parameter_type thlevel_percent = 100.0, 
		    parameter_type thtime = 50.0, 
		    parameter_type atime = 50.0, 
		    parameter_type htime = 50.0, 
		    parameter_type rtime = 50.0);
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

  virtual string name(void) const { return("Normal pan"); }
  virtual string description(void) const { return("Panning effect for controlling the stereo image."); }
  virtual string parameter_names(void) const { return("right-%"); }

  virtual int output_channels(int i_channels) const { return(2); }
    
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);
    
  EFFECT_NORMAL_PAN* clone(void)  { return new EFFECT_NORMAL_PAN(*this); }
  EFFECT_NORMAL_PAN* new_expr(void)  { return new EFFECT_NORMAL_PAN(); }
  EFFECT_NORMAL_PAN(parameter_type right_percent = 50.0);
};

#endif
