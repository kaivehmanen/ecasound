#ifndef _GENERIC_OSCILLATOR_H
#define _GENERIC_OSCILLATOR_H

#include <fstream>
#include <vector>
#include <string>

#include "oscillator.h"

/**
 * Generic oscillator
 *
 * Oscillator value is changed linearly between envelope points. 
 * These points are read from an ascii configuration file.
 */
class GENERIC_OSCILLATOR : public OSCILLATOR {

 public:

  static void set_preset_file(const string& fname);
  static string filename;

  bool preset_found;
  bool linear;
  vector<double> ienvelope;
  double L; // loop length in seconds
  double pdistance; // distance of two points in seconds
  double pcounter; // current position in seconds
  size_t current;
  double curval;

  int preset_rep;

  void read_envelope(void) throw(ECA_ERROR*);
  
 public:

  /**
   * Initialize generic controller
   */
  void init(parameter_type phasestep);

  string parameter_names(void) const { return("freq,preset-number"); }
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  parameter_type value(void);
  string name(void) const { return("Generic oscillator"); }

  GENERIC_OSCILLATOR* clone(void)  { return new GENERIC_OSCILLATOR(*this); }
  GENERIC_OSCILLATOR(void) : OSCILLATOR(0.0, 0.0) { }
  GENERIC_OSCILLATOR(double freq, int preset_number);
  virtual ~GENERIC_OSCILLATOR (void);
};

#endif


