#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

#include <cmath>
#include <string>

#include "ctrl-source.h"

/**
 * Base class for various oscillators
 *
 * Unlike finite controller sources, oscillators
 * produce new values infinately.
 */
class OSCILLATOR : public CONTROLLER_SOURCE {

 public:

  virtual string name(void) const = 0;
  virtual parameter_type value(void) = 0;

  /**
   * Constructor
   * 
   * @param freq Oscillator frequency
   * @param phase Initial phase, multiple of pi
   */
  OSCILLATOR(parameter_type freq = 0, parameter_type initial_phase = 0) { 
    freq_value = freq;
    phase_value = initial_phase * M_PI;
  }

 private:
  
  parameter_type freq_value, phase_value;

 protected:

  parameter_type phase_offset(void) const { return(phase_value); }  
  parameter_type frequency(void) const { return(freq_value); }

  parameter_type phase_offset(parameter_type v) { phase_value = v; }
  parameter_type frequency(parameter_type v) { freq_value = v; }
};

#endif
