#ifndef _CTRL_SOURCE_H
#define _CTRL_SOURCE_H

#include <string>

#include "dynamic-object.h"
#include "sample-specs.h"

/**
 * Base class for all controller sources
 */
class CONTROLLER_SOURCE : public DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type> {

 public:

  typedef SAMPLE_SPECS::sample_type parameter_type;

  /**
   * Initialize controller source
   */
  virtual void init(parameter_type step) { step_length(step); }

  /**
   * Return current value and advance by 'phase_step' seconds.
   */
  virtual parameter_type value(void) = 0; 

  /**
   * Set step length. Internal clock is advanced by 'step_length'
   * seconds everytime value() is called.
   */
  void step_length(parameter_type v) { step_rep = v; }

  /**
   * Current phase step
   */
  parameter_type step_length(void) const { return(step_rep); }

  /**
   * Virtual method that clones the current object and returns 
   * a pointer to it. This must be implemented by all subclasses!
   */
  virtual CONTROLLER_SOURCE* clone(void) = 0;

 private:

  parameter_type step_rep;
};

#endif
