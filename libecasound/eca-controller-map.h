#ifndef _ECA_CONTROLLER_MAP_H
#define _ECA_CONTROLLER_MAP_H

#include <string>
#include <map>

#include "generic-controller.h"

/**
 * Dynamic register for controller sources
 *
 * @author Kai Vehmanen
 */
class ECA_CONTROLLER_MAP {

 public:

  /**
   * Register a new effect.
   */
  static void register_object(const string& id_string, GENERIC_CONTROLLER* object);

  /**
   * List of registered objects (keywords).
   */
  static const map<string,string>& registered_objects(void);

  /**
   * Return the first object that matches with 'keyword'
   */
  static GENERIC_CONTROLLER* object(const string& keyword);

  /**
   * Return the matching keyword for 'object'.
   */
  static string object_identifier(const GENERIC_CONTROLLER* object);
};

#endif
