#ifndef INCLUDED_ECA_OBJECT_MAP_H
#define INCLUDED_ECA_OBJECT_MAP_H

#include <string>
#include <map>
#include <vector>

#include "eca-object.h"

/**
 * A virtual base for dynamic object maps
 *
 * Object maps are used to centralize object creation.
 * Most parts of the library don't require info about
 * object details. Object maps make it possible to 
 * hide these details completely, and in one place.
 *
 * @author Kai Vehmanen
 */
class ECA_OBJECT_MAP {

 private:

  mutable map<string, ECA_OBJECT*> object_map;
  mutable map<string,string> object_keyword_map;

 public:

  /**
   * Registers a new object-regexp pair. Map object won't take care
   * of deleting the registered objects.
   */
  virtual void register_object(const string& regexpr, ECA_OBJECT* object);

  /**
   * List of registered objects ('regexpr'-'object name' map).
   */
  virtual const map<string,string>& registered_objects(void) const;

  /**
   * Returns the first object that matches the expression 'expr'.
   * If 'use_regexp' is true, regex matching is used. For practical
   * reasons a non-const pointer is returned. However, in most 
   * cases the returned object should be cloned before actual use.
   * In other words, the returned pointer refers to the object
   * stored in the object map.
   */
  virtual ECA_OBJECT* object(const string& expr, bool use_regexp = true) const;

  /**
   * Returns the matching keyword for 'object'.
   */
  virtual string object_identifier(const ECA_OBJECT* object) const;

  virtual ~ECA_OBJECT_MAP (void);
};

#endif
