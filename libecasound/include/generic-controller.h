#ifndef _GENERIC_CONTROLLER_H
#define _GENERIC_CONTROLLER_H

#include <kvutils/definition_by_contract.h>

#include "ctrl-source.h"
#include "dynamic-object.h"
#include "sample-specs.h"

/**
 * Generic controller class that connects controller sources
 * to objects supporting dynamic parameter control (classes 
 * which inherit DYNAMIC_PARAMETERS).
 */
class GENERIC_CONTROLLER : public DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>, 
			   public DEFINITION_BY_CONTRACT {

  DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* target;
  CONTROLLER_SOURCE* source;

  int param_id;
  double rangelow, rangehigh;
  double new_value;

 public:

  typedef SAMPLE_SPECS::sample_type parameter_type;

  string name(void) const { return(source->name()); }
  string status(void) const;

  string parameter_names(void) const { return("param-id,range-low,range-high," +  source->parameter_names()); }
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  void assign_target(DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* obj) { target  = obj; }
  void assign_source(CONTROLLER_SOURCE* obj) { source = obj; }

  DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* target_pointer(void) const { return(target); }

  /**
   * Initialize controller source
   */
  void init(parameter_type phase_step) { source->init(phase_step); }

  int param_number(void) const { return(param_id); }
  double low_range_limit(void) const { return(rangelow); }
  double high_range_limit(void) const { return(rangehigh); }

  bool is_valid(void) const { return(target != 0 && source != 0); }
  void param_number(int v) { param_id = v; }
  void low_range_limit(parameter_type v) { rangelow = v; }
  void high_range_limit(parameter_type v) { rangehigh = v; }
  
  /**
   * Fetch new value from controller source, scale it and
   * set target parameters.
   *
   * require:
   *  is_valid() == true
   */
  void process(void);

  GENERIC_CONTROLLER* clone(void);
  GENERIC_CONTROLLER* new_expr(void) { return(new GENERIC_CONTROLLER(0)); }

  GENERIC_CONTROLLER(CONTROLLER_SOURCE* source,
		     DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* dobj = 0, 
		     int param_id = 0, 
		     double range_low = 0.0, 
		     double range_high = 0.0);
};


#endif

