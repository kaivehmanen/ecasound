#ifndef INCLUDED_TWO_STAGE_LINEAR_ENVELOPE_H
#define INCLUDED_TWO_STAGE_LINEAR_ENVELOPE_H

#include <string>

#include "finite-envelope.h"

/**
 * Two-stage linear envelope
 */
class TWO_STAGE_LINEAR_ENVELOPE : public FINITE_ENVELOPE {

 public:

  std::string name(void) const { return("Two-stage linear envelope"); }
  parameter_type value(void);

  void init(parameter_type step);

  std::string parameter_names(void) const { return("1st-stage-sec,2nd-stage-sec"); }
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  TWO_STAGE_LINEAR_ENVELOPE(void); 
  TWO_STAGE_LINEAR_ENVELOPE* clone(void) const { return new TWO_STAGE_LINEAR_ENVELOPE(*this); }
  TWO_STAGE_LINEAR_ENVELOPE* new_expr(void) const { return new TWO_STAGE_LINEAR_ENVELOPE(); }

  private:

  parameter_type first_stage_length_rep, second_stage_length_rep;
  parameter_type curpos, curval;
};

#endif
