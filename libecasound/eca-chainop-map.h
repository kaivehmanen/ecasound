#ifndef _ECA_CHAIN_OPERATOR_MAP_H
#define _ECA_CHAIN_OPERATOR_MAP_H

#include <string>
#include <map>

#include "eca-chainop.h"

/**
 * Dynamic register for chain operators and their id-strings
 *
 * @author Kai Vehmanen
 */
class ECA_CHAIN_OPERATOR_MAP {

 public:

  /**
   * Register a new effect
   */
  static void register_object(const string& id_string, CHAIN_OPERATOR* object);

  /**
   * List of registered effects ('effect name'-'keyword' map).
   */
  static const map<string,string>& registered_objects(void);

  /**
   * Return the first effect that matches with 'keyword'
   */
  static CHAIN_OPERATOR* object(const string& keyword);

  /**
   * Return the matching keyword for 'object'.
   */
  static string object_identifier(const CHAIN_OPERATOR* object);
};

#endif
