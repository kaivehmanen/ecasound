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

  virtual void register_object(const string& regexpr, ECA_OBJECT* object);
  virtual void unregister_object(const string& keyword);
  virtual const map<string,string>& registered_objects(void) const;
  virtual ECA_OBJECT* object(const string& expr, bool use_regexp = true) const;
  virtual string object_identifier(const ECA_OBJECT* object) const;
  virtual void flush(void);

  virtual ~ECA_OBJECT_MAP (void);
};

#endif
