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

  void set_parameter(int param, parameter_type value) { }
  parameter_type get_parameter(int param) const { return(0.0); }

  string parameter_names(void) const { return(""); }

  virtual ~EFFECT_ANALYSIS(void) { }
};

/**
 * Effect for analyzing sample volume.
 * @author Kai Vehmanen
 */
class EFFECT_ANALYZE : public EFFECT_ANALYSIS {

  vector<unsigned long int> num_of_samples; // number of samples processed
  vector<vector<unsigned long int> > ranges;
  static const int range_count = 16;

  parameter_type max;
  SAMPLE_ITERATOR_CHANNELS i;
  
 public:

  parameter_type max_multiplier(void) const;
    
  string name(void) const { return("Volume-analyze"); }

  void init(SAMPLE_BUFFER *insample);
  void process(void);
  string status(void);
  
  EFFECT_ANALYZE* clone(void)  { return new EFFECT_ANALYZE(*this); }
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

  SAMPLE_BUFFER::sample_type tempval;
  SAMPLE_ITERATOR_CHANNELS i;

public:

  parameter_type get_deltafix(int channel);
    
  string name(void) const { return("DC-Find"); }

  void init(SAMPLE_BUFFER *insample);
  void process(void);
  string status(void);

  EFFECT_DCFIND* clone(void)  { return new EFFECT_DCFIND(*this); }
  EFFECT_DCFIND (void);
};

#endif
