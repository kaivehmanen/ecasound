#ifndef _TWO_STAGE_LINEAR_ENVELOPE_H
#define _TWO_STAGE_LINEAR_ENVELOPE_H

#include <string>

#include "finite-envelope.h"

/**
 * Two-stage linear envelope
 */
class TWO_STAGE_LINEAR_ENVELOPE : public FINITE_ENVELOPE {

 public:

  string name(void) const { return("Two-stage linear envelope"); }
  parameter_type value(void);

  void init(parameter_type step);

  string parameter_names(void) const { return("1st_stage_sec,2nd_stage_sec"); }
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  TWO_STAGE_LINEAR_ENVELOPE(void); 
  TWO_STAGE_LINEAR_ENVELOPE* clone(void)  { return new TWO_STAGE_LINEAR_ENVELOPE(*this); }

  private:

  parameter_type first_stage_length_rep, second_stage_length_rep;
  parameter_type curpos, curval;
};

#endif
