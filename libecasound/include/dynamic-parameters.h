#ifndef _DYNAMIC_PARAMETERS_H
#define _DYNAMIC_PARAMETERS_H

#include <map>
#include <string>
#include <utility>

#include <kvutils/kvutils.h>

/**
 * Virtual template class that provides a system for dynamically 
 * controlling a set of parameters. Supports naming of parameters, 
 * default parameter ranges, parameter naming, etc.
 * @author Kai Vehmanen
 */
template<class T>
class DYNAMIC_PARAMETERS {

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
   * A comma-separated list of parameters names. Derived classes 
   * must implement this.
   */
  virtual string parameter_names(void) const = 0;

  /**
   * Set parameter value. Implementations should be able to
   * handle arbitrary values of 'value'.
   *
   * @param param parameter id
   * @param value new value
   */
  virtual void set_parameter(int param, T value) = 0;

  /**
   * Get parameter value
   *
   * @param param parameter id
   */
  virtual T get_parameter(int param) const = 0;

  /**
   * An optional virtual function returning a suggested 
   * default range for the specified parameter. This 
   * is useful when building generic user-interfaces.
   *
   * @param param parameter id
   */
  virtual pair<T,T> default_parameter_range(int param) const { return(make_pair(T(),T())); }

  virtual ~DYNAMIC_PARAMETERS (void) { }
};

#endif
