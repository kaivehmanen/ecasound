#ifndef INCLUDED_ECA_OBJECT_MAP_H
#define INCLUDED_ECA_OBJECT_MAP_H

#include <string>
#include <map>
#include <list>

#include "eca-object.h"

/**
 * A virtual base for dynamic object maps
 *
 * Object maps are used to centralize object creation.
 * Most parts of the library don't require info about
 * object details. Object maps make it possible to 
 * hide these details completely, and in one place.
 *
 * Related design patterns:
 *     - Prototype (GoF117)
 *     - Factory Method (GoF107)
 *
 * @author Kai Vehmanen
 */
class ECA_OBJECT_MAP {

 private:

  std::list<std::string> object_keywords_rep;
  mutable std::map<std::string, ECA_OBJECT*> object_map;
  mutable std::map<std::string,std::string> object_expr_map;

 public:

  virtual void register_object(const std::string& keyword, const std::string& expr, ECA_OBJECT* object);
  virtual void unregister_object(const std::string& keyword);
  virtual void flush(void);

  virtual bool has_keyword(const std::string& keyword) const;
  virtual bool has_object(const ECA_OBJECT* obj) const;

  virtual const std::list<std::string>& registered_objects(void) const;
  virtual const ECA_OBJECT* object(const std::string& keyword) const;
  virtual const ECA_OBJECT* object_expr(const string& expr) const;
  virtual string expr_to_keyword(const std::string& expr) const;
  virtual string keyword_to_expr(const std::string& keyword) const;
  virtual std::string object_identifier(const ECA_OBJECT* object) const;

  virtual ~ECA_OBJECT_MAP (void);
};

#endif
