#ifndef INCLUDED_DYNAMIC_PARAMETERS_H
#define INCLUDED_DYNAMIC_PARAMETERS_H

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

 public:

  typedef T parameter_type;

  /**
   * Initializes parameters before actual use. Virtual function 
   * parameter_names() is used for getting a list of parameters.
   */
  void map_parameters(void) {
    vector<string> t = string_to_vector(parameter_names(), ',');
    vector<string>::const_iterator p = t.begin();
    while(p != t.end()) {
      add_parameter(*p);
      ++p;
    }
  }

  /**
   * Gets the total number of of parameters.
   */
  int number_of_params(void) const { return(param_map.size()); }

  /**
   * Gets the index number of parameter 'name'.
   * @param name parameter name
   */
  int get_parameter_id(const string& name) { return(param_map[name]); }

  /**
   * Gets name of parameter with index 'id'.
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
   * Sets the parameter value. Implementations should be able to
   * handle arbitrary values of 'value'. Argument validity 
   * can be tested by a combination of set_parameter() and 
   * get_parameter() calls. Parameter value is valid, if 
   * get_parameter() returns it without changes.
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

  /**
   * Test whether parameter is a on/off toggle. When
   * setting parameter values, value > 0 means 'on' 
   * and value <= 0 means 'off'.
   *
   * @param param parameter id
   */
  virtual bool is_toggle(int param) const { return(false); }

  virtual ~DYNAMIC_PARAMETERS (void) { }

 private:

  map<string,int> param_map;
  map<int,string> param_revmap;

  /**
   * Add a new parameter.
   */
  void add_parameter(const string& name) { 
    if (param_map.find(name) != param_map.end()) return;
    int id = (int)(param_map.size() + 1);
    param_map[name] = id;
    param_revmap[id] = name;
  }
};

#endif
