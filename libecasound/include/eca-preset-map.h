#ifndef _ECA_PRESET_MAP_H
#define _ECA_PRESET_MAP_H

#include <string>
#include <vector>
#include <map>

#include "preset.h"
#include "eca-object-map.h"

/**
 * Dynamic register for storing effect presets
 *
 * @author Kai Vehmanen
 */
class ECA_PRESET_MAP : public ECA_OBJECT_MAP<PRESET> {

  bool defaults_registered;
  mutable map<string, PRESET*> object_map;
  mutable map<string,string> object_prefix_map;
  vector<string> object_names;

 public:

  /**
   * Register a new effect.
   */
  void register_object(const string& id_string, PRESET* object);

  /**
   * List of registered objects (keywords).
   */
  const vector<string>& registered_objects(void) const;

  /**
   * Return the first object that matches with 'keyword'
   */
  PRESET* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const PRESET* object) const;

  ECA_PRESET_MAP(void) : defaults_registered(false) { }
  virtual ~ECA_PRESET_MAP(void) { }
};

#endif
