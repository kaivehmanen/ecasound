#ifndef _AUDIOFX_ANALYSIS_H
#define _AUDIOFX_ANALYSIS_H

#include <vector>
#include <string>

#include "samplebuffer_iterators.h"
#include "audiofx.h"

/**
 * Virtual base for signal analyzers.
 * @author Kai Vehmanen
 */
class EFFECT_ANALYSIS : public EFFECT_BASE {

 public:

  virtual void set_parameter(int param, parameter_type value) { }
  virtual parameter_type get_parameter(int param) const { return(0.0); }

  virtual string parameter_names(void) const { return(""); }

  virtual ~EFFECT_ANALYSIS(void) { }
};

/**
 * Effect for analyzing sample volume.
 * @author Kai Vehmanen
 */
class EFFECT_ANALYZE : public EFFECT_ANALYSIS {

  static const int range_count = 16;
  static const SAMPLE_SPECS::sample_type clip_amplitude = SAMPLE_SPECS::max_amplitude - SAMPLE_SPECS::max_amplitude / 16384.0f; // max-(max/2^15)

  mutable vector<unsigned long int> num_of_samples; // number of samples processed
  mutable vector<vector<unsigned long int> > ranges;

  mutable parameter_type max_pos_period, max_neg_period;
  mutable unsigned long int clipped_pos_period, clipped_neg_period;
  SAMPLE_SPECS::sample_type max_pos, max_neg;
  unsigned long int clipped_pos, clipped_neg;
  bool cumulativemode_rep;
  SAMPLE_ITERATOR_CHANNELS i;

  void reset_stats(void);
  string status_entry(int range) const;

 public:

  parameter_type max_multiplier(void) const;
    
  virtual string name(void) const { return("Volume-analyze"); }
  virtual string parameter_names(void) const { return("cumulative-mode,result-max-multiplier"); }

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd);
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);
  virtual string status(void) const;
  
  EFFECT_ANALYZE* clone(void)  { return new EFFECT_ANALYZE(*this); }
  EFFECT_ANALYZE* new_expr(void)  { return new EFFECT_ANALYZE(); }
  EFFECT_ANALYZE (void);
};

/**
 * Calculates DC-offset.
 * @author Kai Vehmanen
 */
class EFFECT_DCFIND : public EFFECT_ANALYSIS {

private:

  vector<parameter_type> pos_sum;
  vector<parameter_type> neg_sum;
  vector<parameter_type> num_of_samples;

  SAMPLE_SPECS::sample_type tempval;
  SAMPLE_ITERATOR_CHANNELS i;

public:

  parameter_type get_deltafix(int channel) const;

  virtual string name(void) const { return("DC-Find"); }
  virtual string description(void) const { return("Calculates the DC-offset."); }
  virtual string parameter_names(void) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);
  virtual string status(void) const;

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd);
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  EFFECT_DCFIND* clone(void)  { return new EFFECT_DCFIND(*this); }
  EFFECT_DCFIND* new_expr(void)  { return new EFFECT_DCFIND(); }
  EFFECT_DCFIND (void);
};

#endif
