#ifndef _DYNAMIC_PARAMETERS_H
#define _DYNAMIC_PARAMETERS_H

#include <map>
#include <string>

#include <kvutils/kvutils.h>

#include "samplebuffer.h"

/**
 * Virtual class that provides a system for dynamically controlling 
 * a set of parameters. Supports naming of parameters, minimum and 
 * maximum values and validity checks.
 * @author Kai Vehmanen
 */
class DYNAMIC_PARAMETERS {

 public:

  typedef SAMPLE_BUFFER::sample_type parameter_type;

 private:

  map<string,int> param_map;
  map<int,string> param_revmap;

  /**
   * Add a new parameter. Derived classes should use this routine 
   * to specify their parameters.
   */
  void add_parameter(const string& name) { 
    int id = (int)(param_map.size() + 1);
    param_map[name] = id;
    param_revmap[id] = name;
  }

 public:

  void map_parameters(void) {
    vector<string> t = string_to_vector(parameter_names(), ',');
    vector<string>::const_iterator p = t.begin();
    while(p != t.end()) {
      add_parameter(*p);
      ++p;
    }
  }

  /**
   * Get number of of parameters.
   */
  int number_of_params(void) const { return(param_map.size()); }

  /**
   * Get parameter id number.
   * @param name parameter name
   */
  int get_parameter_id(const string& name) { return(param_map[name]); }

  /**
   * Get parameter name. 
   * @param id parameter id number
   * 
   */
  const string& get_parameter_name(int id) { return(param_revmap[id]); }

  /**
   * Object name. Identifies the type represented by dynamic parameters.
   * Must be implemented in subclasses.
   */
  virtual string name(void) const = 0;

  /**
   * A comma-separated list of parameters names. Derived classes 
   * must implement this.
   */
  virtual string parameter_names(void) const = 0;

  /**
   * Set parameter value
   *
   * @param param parameter id
   * @param value new value
   */
  virtual void set_parameter(int param, parameter_type value) = 0;

  /**
   * Get parameter value
   *
   * @param param parameter id
   */
  virtual parameter_type get_parameter(int param) const = 0;

  /**
   * If implemented in subclasses, this can be used for doing 
   * sanity checks on parameter values before using them.
   *
   * @param param parameter id
   * @param value value to check
   */
  virtual bool valid_parameter(int param, parameter_type value) { return(true); }

  virtual ~DYNAMIC_PARAMETERS (void) { }
};

#endif


