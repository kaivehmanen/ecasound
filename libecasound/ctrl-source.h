#ifndef INCLUDE_CTRL_SOURCE_H
#define INCLUDE_CTRL_SOURCE_H

#include "eca-operator.h"

/**
 * Base class for all controller sources
 */
class CONTROLLER_SOURCE : public OPERATOR {

 public:

  typedef SAMPLE_SPECS::sample_t parameter_t;

  /**
   * Initialize controller source
   */
  virtual void init(parameter_t step) { step_length(step); }

  /**
   * Return current value and advance by 'phase_step' seconds.
   * Standard value range is [0,1].
   */
  virtual parameter_t value(void) = 0; 

  /**
   * Set step length. Internal clock is advanced by 'step_length'
   * seconds everytime value() is called.
   */
  void step_length(parameter_t v) { step_rep = v; }

  /**
   * Current phase step
   */
  parameter_t step_length(void) const { return(step_rep); }

  virtual CONTROLLER_SOURCE* clone(void) const = 0;
  virtual CONTROLLER_SOURCE* new_expr(void) const = 0;

 private:

  parameter_t step_rep;
};

#endif
