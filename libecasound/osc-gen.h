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

  int mode_rep;
  double start_value_rep, end_value_rep;
  double loop_length_rep; // loop length in seconds
  double loop_pos_rep; // current position in seconds
  double next_pos_rep;
  double last_pos_rep;
  int epairs_rep;
  int eindex_rep;
  int pindex_rep;
  double current_value_rep;
  std::string param_names_rep;
  void set_param_count(int n);

 protected:

  // FIXME: replace this with a function: void set_point(int n);
  std::vector<double> ienvelope_rep;

  void set_start_value(double v) { start_value_rep = v; }
  void set_end_value(double v) { end_value_rep = v; }
  void prepare_envelope(void);
  void update_current_static(void);
  void update_current_linear(void);

 public:

  /**
   * Initialize generic controller
   */
  virtual void init(parameter_type phasestep);

  virtual std::string parameter_names(void) const;
  virtual void set_parameter(int param, parameter_type value);
  virtual parameter_type get_parameter(int param) const;

  virtual parameter_type value(void);
  std::string name(void) const { return("Generic oscillator"); }

  GENERIC_OSCILLATOR* clone(void)  { return new GENERIC_OSCILLATOR(*this); }
  GENERIC_OSCILLATOR* new_expr(void)  { return new GENERIC_OSCILLATOR(*this); }
  GENERIC_OSCILLATOR(void) : OSCILLATOR(0.0, 0.0) { }
  GENERIC_OSCILLATOR(double freq, int mode);
  virtual ~GENERIC_OSCILLATOR (void);
};

#endif


