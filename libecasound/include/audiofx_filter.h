#ifndef _AUDIOFX_FILTER_H
#define _AUDIOFX_FILTER_H

#include <vector>
#include <deque>

#include "audiofx.h"
#include "samplebuffer_iterators.h"

/**
 * Virtual base for filter effects.
 * @author Kai Vehmanen
 */
class EFFECT_FILTER : public EFFECT_BASE {

 public:
  virtual ~EFFECT_FILTER(void) { }
};

/**
 * Base class for butterworth filter effects.
 * 
 * Based on SPKit Butterworth algorithms. 
 * (for more info, see http://www.music.helsinki.fi/research/spkit)
 */
class EFFECT_BW_FILTER : public EFFECT_FILTER {

private:
  
  SAMPLE_SPECS::sample_type outputSample;
  SAMPLE_ITERATOR_CHANNELS i;

  vector<vector<SAMPLE_SPECS::sample_type> > sin;
  vector<vector<SAMPLE_SPECS::sample_type> > sout;

  void init_values(void);

 protected:

  vector<SAMPLE_SPECS::sample_type> a;
  vector<SAMPLE_SPECS::sample_type> b;
   
public:

  void process_notused(SAMPLE_BUFFER* sbuf);
  void init(SAMPLE_BUFFER *insample);
  void process(void);

  virtual EFFECT_BW_FILTER* clone(void) = 0;

  //  EFFECT_BW_FILTER(void) : sin(2), sout(2), a(3), b(2) {

  EFFECT_BW_FILTER(void) : a(3), b(2) {
    init_values();
  }
};

/**
 * Bandpass filter
 *  
 * Based on SPKit Butterworth algorithms
 * (for more info, see http://www.music.helsinki.fi/research/spkit)
 */
class EFFECT_BANDPASS: public EFFECT_BW_FILTER {

private:
    
  parameter_type center;
  parameter_type width;

  parameter_type C;
  parameter_type D;

public:

  string name(void) const { return("Bandpass filter"); }

  string parameter_names(void) const { return("center-freq,width"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  EFFECT_BANDPASS* clone(void)  { return new EFFECT_BANDPASS(*this); }
  EFFECT_BANDPASS* new_expr(void)  { return new EFFECT_BANDPASS(); }
  EFFECT_BANDPASS (parameter_type centerf = 0.0, parameter_type width = 1.0);
};

/**
 * Bandreject filter
 *  
 * Based on SPKit Butterworth algorithms
 * (for more info, see http://www.music.helsinki.fi/research/spkit)
 */
class EFFECT_BANDREJECT: public EFFECT_BW_FILTER {

private:
    
  parameter_type center;
  parameter_type width;

  parameter_type C;
  parameter_type D;

public:

  string name(void) const { return("Bandreject filter"); }

  string parameter_names(void) const { return("center-freq,width"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  EFFECT_BANDREJECT* clone(void)  { return new EFFECT_BANDREJECT(*this); }
  EFFECT_BANDREJECT* new_expr(void)  { return new EFFECT_BANDREJECT(); }
  EFFECT_BANDREJECT (parameter_type centerf = 0.0, parameter_type width = 1.0);
};

/**
 * Highpass filter
 *
 * Based on SPKit Butterworth algorithms
 * (for more info, see http://www.music.helsinki.fi/research/spkit)
 */
class EFFECT_HIGHPASS : public EFFECT_BW_FILTER {
  
 private:

  parameter_type cutOffFreq;

  parameter_type C;
    
public:
    
  string name(void) const { return("Highpass filter"); }

  string parameter_names(void) const { return("cutoff-freq"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  EFFECT_HIGHPASS* clone(void)  { return new EFFECT_HIGHPASS(*this); }
  EFFECT_HIGHPASS* new_expr(void)  { return new EFFECT_HIGHPASS(); }
  EFFECT_HIGHPASS (parameter_type cutoff = 0.0);
};

/**
 * Allpass filter
 */
class EFFECT_ALLPASS_FILTER : public EFFECT_FILTER {

  vector<deque<SAMPLE_SPECS::sample_type> > inbuf, outbuf;
  SAMPLE_ITERATOR_CHANNELS i;

  parameter_type feedback_gain;
  parameter_type D;

public:

  string name(void) const { return("Allpass filter"); }
  string parameter_names(void) const { return("delay-samples,feedback-%"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_ALLPASS_FILTER* clone(void)  { return new EFFECT_ALLPASS_FILTER(*this); }  
  EFFECT_ALLPASS_FILTER* new_expr(void)  { return new EFFECT_ALLPASS_FILTER(); }
  EFFECT_ALLPASS_FILTER (void) { }
};


/**
 * Comb filter
 *
 * The basic theory behind this can be found from Ken Steiglitz's book 
 * "A digital signal processing primer", page 103.
 */
class EFFECT_COMB_FILTER : public EFFECT_FILTER {

  vector<deque<SAMPLE_SPECS::sample_type> > buffer;
  vector<SAMPLE_SPECS::sample_type> temp;
  SAMPLE_ITERATOR_CHANNELS i;

  parameter_type C;
  parameter_type D;

public:

  string name(void) const { return("Comb filter"); }
  string parameter_names(void) const { return("delay-samples,radius"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_COMB_FILTER* clone(void)  { return new EFFECT_COMB_FILTER(*this); }  
  EFFECT_COMB_FILTER* new_expr(void)  { return new EFFECT_COMB_FILTER(); }
  EFFECT_COMB_FILTER (int delay_in_samples = 0, parameter_type constant = 1.0);
};

/**
 * Inverse comb filter
 *
 * The basic theory behind this can be found from Ken Steiglitz's book 
 * "A digital signal processing primer", page 77.
 */
class EFFECT_INVERSE_COMB_FILTER : public EFFECT_FILTER {

  vector<parameter_type> laskuri;
  vector<deque<SAMPLE_SPECS::sample_type> > buffer;
  vector<SAMPLE_SPECS::sample_type> temp;
  SAMPLE_ITERATOR_CHANNELS i;

  parameter_type C;
  parameter_type D;

public:

  string name(void) const { return("Inverse comb filter"); }

  string parameter_names(void) const { return("delay-samples,radius"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_INVERSE_COMB_FILTER* clone(void)  { return new EFFECT_INVERSE_COMB_FILTER(*this); }  
  EFFECT_INVERSE_COMB_FILTER* new_expr(void)  { return new EFFECT_INVERSE_COMB_FILTER(); }
  EFFECT_INVERSE_COMB_FILTER (int delay_in_samples = 0, parameter_type constant = 1.0);
};

/**
 * Lowpass filter
 *  
 * Based on SPKit Butterworth algorithms
 * (for more info, see http://www.music.helsinki.fi/research/spkit)
 */
class EFFECT_LOWPASS: public EFFECT_BW_FILTER {

private:

  parameter_type cutOffFreq;

  parameter_type C;

public:

  string name(void) const { return("Lowpass filter"); }

  string parameter_names(void) const { return("cutoff-freq"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void set_cutoff(parameter_type value, long int srate);

  EFFECT_LOWPASS* clone(void)  { return new EFFECT_LOWPASS(*this); }  
  EFFECT_LOWPASS* new_expr(void)  { return new EFFECT_LOWPASS(); }
  EFFECT_LOWPASS (parameter_type cutoff = 0.0);
};

/**
 * A simple lowpass filter
 *                                              
 *   Algorithm:  1nd order filter.             
 *   From Fugue source code:                   
 *                                             
 *    output[N] = input[N] * A + input[N-1] * B
 *                                             
 *    A = 2.0 * pi * center                    
 *    B = exp(-A / frequency)
 */                                            
class EFFECT_LOWPASS_SIMPLE : public EFFECT_FILTER {

private:

  parameter_type cutOffFreq;
  parameter_type A, B;
  vector<SAMPLE_SPECS::sample_type> outhist, tempin, temphist;
  SAMPLE_ITERATOR_CHANNELS i;

public:

  string name(void) const { return("Simple lowpass filter"); }

  string parameter_names(void) const { return("cutoff-freq"); }

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  EFFECT_LOWPASS_SIMPLE* clone(void)  { return new EFFECT_LOWPASS_SIMPLE(*this); }
  EFFECT_LOWPASS_SIMPLE* new_expr(void)  { return new EFFECT_LOWPASS_SIMPLE(); }
  EFFECT_LOWPASS_SIMPLE (parameter_type cutoff = 0.0);
};

/**
 * Resonant bandpass filter
 */
class EFFECT_RESONANT_BANDPASS : public EFFECT_FILTER {

private:

  vector<SAMPLE_SPECS::sample_type> outhist1, outhist2;
  
  parameter_type center;
  parameter_type width;
  
  parameter_type a, b, c, R;
  parameter_type pole_angle;

  SAMPLE_ITERATOR_CHANNELS i;

public:

  string name(void) const { return("Resonant bandpass filter"); }

  string parameter_names(void) const { return("center-freq,width"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_RESONANT_BANDPASS* clone(void)  { return new EFFECT_RESONANT_BANDPASS(*this); }  
  EFFECT_RESONANT_BANDPASS* new_expr(void)  { return new EFFECT_RESONANT_BANDPASS(); }  
  EFFECT_RESONANT_BANDPASS (parameter_type centerf = 0.0, parameter_type width = 0.0);
};

/**
 * Resonant lowpass filter
 *
 * Algorithm is based on a sample filter-routine (iir_filter) posted to comp.dsp.
 */
class EFFECT_RESONANT_LOWPASS : public EFFECT_FILTER {

  SAMPLE_ITERATOR_CHANNELS i;
    
  vector<SAMPLE_SPECS::sample_type> outhist0, outhist1, outhist2, outhist3;
  vector<SAMPLE_SPECS::sample_type> newhist0, newhist1;
    
  class TRIPLE_COEFS {
  public:
    parameter_type a0, a1, a2;       // numerator coefficients
    parameter_type b0, b1, b2;       // denominator coefficients
  };

  class FILTER_COEFS {
  public:
    parameter_type A, B, C, D;       // filter coefficients
  };
    
  vector<TRIPLE_COEFS> ProtoCoef;         // Filter prototype coefficients,
                                          // for each filter section
  vector<FILTER_COEFS> Coef;
    
  parameter_type cutoff, Q, gain, gain_orig;
  parameter_type pi;
  parameter_type laskuri;

  parameter_type ad, bd, wp;      // for szxform()

  void szxform(int section);
  void refresh_values(void);

public:

  string name(void) const { return("Resonant lowpass filter"); }

  string parameter_names(void) const { return("cutoff-freq,resonance,gain"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_RESONANT_LOWPASS* clone(void)  { return new EFFECT_RESONANT_LOWPASS(*this); }  
  EFFECT_RESONANT_LOWPASS* new_expr(void)  { return new EFFECT_RESONANT_LOWPASS(); }  
  EFFECT_RESONANT_LOWPASS (parameter_type cutoff = 0.0,
			   parameter_type resonance = 1.0,
			   parameter_type gain = 1.0);
};

/**
 * Resonating bandpass filter
 *
 * Based on a second order all-pole (IIR) band-pass filter from SPKit 
 * (for more info, see: http://www.music.helsinki.fi/research/spkit)
 */
class EFFECT_RESONATOR : public EFFECT_FILTER {

private:

  SAMPLE_ITERATOR_CHANNELS i;
    
  parameter_type center;
  parameter_type width;

  vector<SAMPLE_SPECS::sample_type> cona;
  vector<SAMPLE_SPECS::sample_type> conb;

  vector<SAMPLE_SPECS::sample_type> saout0, saout1;

public:

  string name(void) const { return("Resonator filter"); }

  string parameter_names(void) const { return("center-freq,width"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_RESONATOR* clone(void)  { return new EFFECT_RESONATOR(*this); }
  EFFECT_RESONATOR* new_expr(void)  { return new EFFECT_RESONATOR(); }  
  EFFECT_RESONATOR (parameter_type center, parameter_type width);
  EFFECT_RESONATOR (void) : cona(1), conb(2) { }
};

#endif
