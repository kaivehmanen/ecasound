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
class ECA_CHAIN_OPERATOR_MAP : public ECA_OBJECT_MAP<CHAIN_OPERATOR> {

  bool defaults_registered;
  mutable map<string, CHAIN_OPERATOR*> object_map;
  mutable map<string,string> object_prefix_map;
  vector<string> object_names;

 public:

  /**
   * Register a new effect.
   */
  void register_object(const string& id_string, CHAIN_OPERATOR* object);

  /**
   * List of registered objects (keywords).
   */
  const vector<string>& registered_objects(void) const;

  /**
   * Return the first object that matches with 'keyword'
   */
  CHAIN_OPERATOR* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const CHAIN_OPERATOR* object) const;

  ECA_CHAIN_OPERATOR_MAP(void) : defaults_registered(false) { }
  virtual ~ECA_CHAIN_OPERATOR_MAP(void) { }
};

#endif
