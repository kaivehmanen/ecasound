#ifndef INCLUDED_LINEAR_ENVELOPE_H
#define INCLUDED_LINEAR_ENVELOPE_H

#include <string>

#include "finite-envelope.h"

/**
 * Linear envelope
 */
class LINEAR_ENVELOPE : public FINITE_ENVELOPE {

 public:

  LINEAR_ENVELOPE(void);
  virtual ~LINEAR_ENVELOPE(void);

  LINEAR_ENVELOPE* clone(void) const { return new LINEAR_ENVELOPE(*this); }
  LINEAR_ENVELOPE* new_expr(void) const { return new LINEAR_ENVELOPE(*this); }

  std::string name(void) const { return("Linear envelope"); }
  parameter_t value(void);

  void init(parameter_t step);

  std::string parameter_names(void) const { return("length-sec"); }
  void set_parameter(int param, parameter_t value);
  parameter_t get_parameter(int param) const;

  private:

  parameter_t curval;
};

#endif

