#ifndef _LINEAR_ENVELOPE_H
#define _LINEAR_ENVELOPE_H

#include <string>

#include "finite-envelope.h"

/**
 * Linear envelope
 */
class LINEAR_ENVELOPE : public FINITE_ENVELOPE {

 public:

  std::string name(void) const { return("Linear envelope"); }
  parameter_type value(void);

  void init(parameter_type step);

  std::string parameter_names(void) const { return("length-sec"); }
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  LINEAR_ENVELOPE(parameter_type time_in_seconds = 0.0); 
  LINEAR_ENVELOPE* clone(void)  { return new LINEAR_ENVELOPE(*this); }
  LINEAR_ENVELOPE* new_expr(void)  { return new LINEAR_ENVELOPE(*this); }

  private:

  parameter_type curpos, curval;
};

#endif

