#ifndef _ECA_CONTROLLER_MAP_H
#define _ECA_CONTROLLER_MAP_H

#include <string>
#include <map>

#include "ctrl-source.h"

/**
 * Dynamic register for controller sources
 *
 * @author Kai Vehmanen
 */
class ECA_CONTROLLER_MAP {

 public:

  /**
   * 'id-string' - 'object pointer' map
   */
  static map<string, DYNAMIC_OBJECT*> object_map;

  /**
   * 'object-name' - 'prefix-string' map
   */
  static map<string, string> object_prefix_map;

  /**
   * Register a new controller
   */
  static void register_object(const string& id_string, DYNAMIC_OBJECT* object);

  /**
   * Register default controllers
   */
  static void register_default_objects(void);
};

#endif

