#ifndef _SINE_OSCILLATOR_H
#define _SINE_OSCILLATOR_H

#include <string>

#include "oscillator.h"

/**
 * Sine oscillator
 */
class SINE_OSCILLATOR : public OSCILLATOR {

  double phase, L;
  double phasemod;
  double curval;
  
 public:

  void init(parameter_type step);

  std::string parameter_names(void) const { return("freq,phase-offset"); }
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  std::string name(void) const { return("Sine oscillator"); }
  parameter_type value(void);

  SINE_OSCILLATOR* clone(void)  { return new SINE_OSCILLATOR(*this); }
  SINE_OSCILLATOR* new_expr(void)  { return new SINE_OSCILLATOR(); }
  SINE_OSCILLATOR (double freq = 0, double initial_phase = 0);
};

#endif
