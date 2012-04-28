#ifndef INCLUDED_GENERIC_OSCILLATOR_H
#define INCLUDED_GENERIC_OSCILLATOR_H

#include <vector>
#include <string>

#include "oscillator.h"

#include "eca-error.h"

/**
 * Generic oscillator
 *
 * Oscillator value varies according to discrete 
 * envelope points.
 */
class GENERIC_OSCILLATOR : public OSCILLATOR {

 public:

  virtual void init(void);
  virtual parameter_t value(double pos_secs);
  virtual std::string parameter_names(void) const;
  virtual void set_parameter(int param, parameter_t value);
  virtual parameter_t get_parameter(int param) const;

  std::string name(void) const { return "Generic oscillator"; }

  GENERIC_OSCILLATOR* clone(void) const { return new GENERIC_OSCILLATOR(*this); }
  GENERIC_OSCILLATOR* new_expr(void) const { return new GENERIC_OSCILLATOR(*this); }
  GENERIC_OSCILLATOR(double freq = 0.0f, int mode = 0);
  virtual ~GENERIC_OSCILLATOR (void);

 protected:

  void prepare_envelope(void);

private:

  class POINT {
  public:
    double pos;
    double val;
  };

  size_t current_stage(double pos);
  void set_param_count(int n);

  std::vector<POINT> envtable_rep;
  std::vector<double> params_rep;
  size_t last_stage_rep;
  double last_pos_scaled_rep;

  int mode_rep;
  double first_value_rep, last_value_rep;
  double loop_length_rep; // loop length in seconds
  std::string param_names_rep;
};

#endif
