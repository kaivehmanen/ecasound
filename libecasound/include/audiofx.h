#ifndef _AUDIOFX_H
#define _AUDIOFX_H

#include <vector>

#include "samplebuffer_iterators.h"
#include "eca-chainop.h"

/**
 * Virtual base for all audio effect classes.
 * @author Kai Vehmanen
 */
class EFFECT_BASE : public CHAIN_OPERATOR {

 public:
  virtual ~EFFECT_BASE(void) { }
};

/**
 * Adjusts DC-offset.
 * @author Kai Vehmanen
 */
class EFFECT_DCFIX : public EFFECT_BASE {

private:

  SAMPLE_BUFFER::sample_type deltafix[2];
  SAMPLE_ITERATOR_CHANNEL i;

public:

  string name(void) const { return("DC-Fix"); }

  string parameter_names(void) const { return("delta-value-left,delta-value-right"); }

  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void init(SAMPLE_BUFFER *insample);
  void process(void);

  EFFECT_DCFIX* clone(void)  { return new EFFECT_DCFIX(*this); }
  EFFECT_DCFIX (const EFFECT_DCFIX& x);
  EFFECT_DCFIX (parameter_type delta_left = 0.0, parameter_type delta_right = 0.0);
};

#endif
