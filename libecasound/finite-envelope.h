#ifndef _FINITE_ENVELOPE_H
#define _FINITE_ENVELOPE_H

#include <string>

#include "ctrl-source.h"

/**
 * Controller envelopes that have finite length
 */
class FINITE_ENVELOPE : public CONTROLLER_SOURCE {

  parameter_t length_in_seconds_rep;

 public:

  virtual std::string name(void) const = 0;
  virtual parameter_t value(void) = 0;

  /**
   * Constructor
   * 
   * @param time_in_seconds Envelope length in seconds
   */
  FINITE_ENVELOPE(parameter_t time_in_seconds = 0.0) : length_in_seconds_rep(time_in_seconds) { } 

 protected:

  parameter_t length_in_seconds(void) const { return(length_in_seconds_rep); }  
  void length_in_seconds(parameter_t v) { length_in_seconds_rep = v; }
};

#endif

