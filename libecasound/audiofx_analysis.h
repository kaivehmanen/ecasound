#ifndef INCLUDED_AUDIOFX_ANALYSIS_H
#define INCLUDED_AUDIOFX_ANALYSIS_H

#include <string>
#include <vector>

#include <pthread.h>

#include "samplebuffer_iterators.h"
#include "audiofx.h"

class MESSAGE_ITEM;

/**
 * Virtual base for signal analyzers.
 * @author Kai Vehmanen
 */
class EFFECT_ANALYSIS : public EFFECT_BASE {

 protected:

  static const SAMPLE_SPECS::sample_t clip_amplitude;

 public:

  virtual void set_parameter(int param, parameter_t value) { }
  virtual parameter_t get_parameter(int param) const { return(0.0); }

  virtual std::string parameter_names(void) const { return(""); }

  virtual ~EFFECT_ANALYSIS(void);
};

/**
 * Analyzes the audio signal volume by using a set of 
 * amplitude range buckets.
 *
 * @author Kai Vehmanen
 */
class EFFECT_VOLUME_BUCKETS : public EFFECT_ANALYSIS {

  static const int range_count;
  static const SAMPLE_SPECS::sample_t clip_amplitude;

  mutable std::vector<unsigned long int> num_of_samples; // number of samples processed
  mutable std::vector<std::vector<unsigned long int> > ranges;

  mutable parameter_t max_pos_period, max_neg_period;
  mutable unsigned long int clipped_pos_period, clipped_neg_period;
  mutable pthread_mutex_t lock_rep;
  mutable std::string status_rep;
  
  SAMPLE_SPECS::sample_t max_pos, max_neg;
  unsigned long int clipped_pos, clipped_neg;
  bool cumulativemode_rep;
  SAMPLE_ITERATOR_CHANNELS i;

  void reset_stats(void);
  void status_entry(int range, std::string& otemp) const;

 public:

  parameter_t max_multiplier(void) const;
    
  virtual std::string name(void) const { return("Volume analysis"); }
  virtual std::string parameter_names(void) const { return("cumulative-mode,result-max-multiplier"); }

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd) const;
  virtual void set_parameter(int param, parameter_t value);
  virtual parameter_t get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);
  virtual std::string status(void) const;
  
  virtual EFFECT_VOLUME_BUCKETS* clone(void) const { return new EFFECT_VOLUME_BUCKETS(*this); }
  virtual EFFECT_VOLUME_BUCKETS* new_expr(void) const { return new EFFECT_VOLUME_BUCKETS(); }
  EFFECT_VOLUME_BUCKETS (void);
  virtual ~EFFECT_VOLUME_BUCKETS (void);
};

/**
 * Keeps track of peak amplitude.
 *
 * @author Kai Vehmanen
 */
class EFFECT_VOLUME_PEAK : public EFFECT_ANALYSIS {

 public:

  virtual std::string name(void) const { return("Peak amplitude watcher"); }
  virtual std::string parameter_names(void) const;

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd) const;
  virtual void set_parameter(int param, parameter_t value);
  virtual parameter_t get_parameter(int param) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);
  // virtual std::string status(void) const;
  
  virtual EFFECT_VOLUME_PEAK* clone(void) const { return new EFFECT_VOLUME_PEAK(*this); }
  virtual EFFECT_VOLUME_PEAK* new_expr(void) const { return new EFFECT_VOLUME_PEAK(); }
  EFFECT_VOLUME_PEAK (void);
  virtual ~EFFECT_VOLUME_PEAK (void);

 private:

  mutable volatile parameter_t * volatile max_amplitude_repp;
  mutable long int clipped_samples_rep;

  mutable std::string status_rep;

  SAMPLE_ITERATOR_CHANNELS i;
};

/**
 * Calculates DC-offset.
 *
 * @author Kai Vehmanen
 */
class EFFECT_DCFIND : public EFFECT_ANALYSIS {

private:

  std::vector<parameter_t> pos_sum;
  std::vector<parameter_t> neg_sum;
  std::vector<parameter_t> num_of_samples;

  SAMPLE_SPECS::sample_t tempval;
  SAMPLE_ITERATOR_CHANNELS i;

public:

  parameter_t get_deltafix(int channel) const;

  virtual std::string name(void) const { return("DC-Find"); }
  virtual std::string description(void) const { return("Calculates the DC-offset."); }
  virtual std::string parameter_names(void) const;

  virtual void init(SAMPLE_BUFFER *insample);
  virtual void process(void);
  virtual std::string status(void) const;

  virtual void parameter_description(int param, struct PARAM_DESCRIPTION *pd) const;
  virtual void set_parameter(int param, parameter_t value);
  virtual parameter_t get_parameter(int param) const;

  virtual EFFECT_DCFIND* clone(void) const { return new EFFECT_DCFIND(*this); }
  virtual EFFECT_DCFIND* new_expr(void) const { return new EFFECT_DCFIND(); }
  EFFECT_DCFIND (void);
};

#endif
