#ifndef INCLUDED_OPERATOR_H
#define INCLUDED_OPERATOR_H

#include "sample-specs.h"
#include "dynamic-object.h"

/**
 * All ecasound objects which can be used as targets
 * for dynamic control, are called operators.
 *
 * @author Kai Vehmanen
 */
class OPERATOR : public DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type> {

 public:

  virtual OPERATOR* clone(void) = 0;
  virtual OPERATOR* new_expr(void) = 0;

  virtual ~OPERATOR (void) { }
};

#endif
