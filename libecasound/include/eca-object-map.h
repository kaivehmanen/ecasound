#ifndef _ECA_OBJECT_MAP_H
#define _ECA_OBJECT_MAP_H

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
   * Register a new effect.
   */
  virtual void register_object(const string& keyword, ECA_OBJECT* object);

  /**
   * List of registered objects ('object name'-'keyword' map).
   */
  virtual const map<string,string>& registered_objects(void) const;

  /**
   * Return the first object that matches with 'keyword'
   */
  virtual ECA_OBJECT* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  virtual string object_identifier(const ECA_OBJECT* object) const;

  virtual ~ECA_OBJECT_MAP (void) { }
};

#endif
