#ifndef _ECA_OBJECT_MAP_H
#define _ECA_OBJECT_MAP_H

#include <string>
#include <map>

/**
 * A virtual base for dynamic object maps
 *
 * Object maps are used to centralize object creation.
 * Most parts of the library don'n require info about
 * object details. Object maps make it possible to 
 * hide these details completely, and in one palce.
 *
 * @author Kai Vehmanen
 */
template<class T>
class ECA_OBJECT_MAP {

 public:

  /**
   * Register a new effect.
   */
  virtual void register_object(const string& id_string, T* object) = 0;

  /**
   * List of registered objects (keywords).
   */
  virtual const vector<string>& registered_objects(void) const = 0;

  /**
   * Return the first object that matches with 'keyword'
   */
  virtual T* object(const string& keyword) const = 0;

  /**
   * Return the matching keyword for 'object'.
   */
  virtual string object_identifier(const T* object) const = 0;
};

#endif
