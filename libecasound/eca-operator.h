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

  typedef SAMPLE_SPECS::sample_type sample_type;

  /**
   * An optional virtual function returning a suggested 
   * default range for the specified parameter. This 
   * is useful when building generic user-interfaces.
   *
   * @param param parameter id
   */
  virtual pair<sample_type,sample_type> default_parameter_range(int param) const { return(make_pair(sample_type(),sample_type())); }

  /**
   * Test whether parameter is a on/off toggle. When
   * setting parameter values, value > 0 means 'on' 
   * and value <= 0 means 'off'.
   *
   * @param param parameter id
   */
  virtual bool is_toggle(int param) const { return(false); }

  virtual OPERATOR* clone(void) = 0;
  virtual OPERATOR* new_expr(void) = 0;

  virtual ~OPERATOR (void) { }
};

#endif
