#ifndef _ECA_CONTROLLER_MAP_H
#define _ECA_CONTROLLER_MAP_H

#include <string>
#include <map>

#include "generic-controller.h"
#include "eca-object-map.h"

/**
 * Dynamic register for controller sources
 *
 * @author Kai Vehmanen
 */
class ECA_CONTROLLER_MAP {

  ECA_OBJECT_MAP omap;

 public:

  /**
   * Register a new effect.
   */
  void register_object(const string& id_string, GENERIC_CONTROLLER* object);

  /**
   * List of registered objects (keywords).
   */
  const map<string,string>& registered_objects(void) const;

  /**
   * Return the first object that matches with 'keyword'
   */
  GENERIC_CONTROLLER* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const GENERIC_CONTROLLER* object) const;

  virtual ~ECA_CONTROLLER_MAP(void) { }
};

#endif
