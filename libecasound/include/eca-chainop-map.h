#ifndef _ECA_CHAIN_OPERATOR_MAP_H
#define _ECA_CHAIN_OPERATOR_MAP_H

#include <string>
#include <map>

#include "eca-chainop.h"
#include "eca-object-map.h"

/**
 * Dynamic register for chain operators and their id-strings
 *
 * @author Kai Vehmanen
 */
class ECA_CHAIN_OPERATOR_MAP {

  ECA_OBJECT_MAP omap;

 public:

  /**
   * Register a new effect
   */
  void register_object(const string& id_string, CHAIN_OPERATOR* object);

  /**
   * List of registered effects ('effect name'-'keyword' map).
   */
  const map<string,string>& registered_objects(void) const;

  /**
   * Return the first effect that matches with 'keyword'
   */
  CHAIN_OPERATOR* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const CHAIN_OPERATOR* object) const;

  virtual ~ECA_CHAIN_OPERATOR_MAP(void) { }
};

#endif
