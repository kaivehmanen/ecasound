#ifndef _FINITE_ENVELOPE_H
#define _FINITE_ENVELOPE_H

#include <string>

#include "ctrl-source.h"

/**
 * Controller envelopes that have finite length
 */
class FINITE_ENVELOPE : public CONTROLLER_SOURCE {

  parameter_type length_in_seconds_rep;

 public:

  virtual string name(void) const = 0;
  virtual parameter_type value(void) = 0;

  /**
   * Constructor
   * 
   * @param freq Oscillator frequency
   * @param phase Initial phase, multiple of pi
   */
  FINITE_ENVELOPE(parameter_type time_in_seconds) : length_in_seconds_rep(time_in_seconds) { } 

 protected:

  parameter_type length_in_seconds(void) const { return(length_in_seconds_rep); }  
  void length_in_seconds(parameter_type v) { length_in_seconds_rep = v; }
};

#endif

