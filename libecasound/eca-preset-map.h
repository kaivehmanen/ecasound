#ifndef _ECA_PRESET_MAP_H
#define _ECA_PRESET_MAP_H

#include <string>
#include <vector>
#include <map>

#include "preset.h"
#include "eca-object-map.h"
#include "resource-file.h"

/**
 * Dynamic register for storing effect presets
 *
 * @author Kai Vehmanen
 */
class ECA_PRESET_MAP : public ECA_OBJECT_MAP {

 private:

  RESOURCE_FILE preset_file;
  mutable map<string, PRESET*> object_map;
  mutable map<string,string> object_keyword_map;

 public:

  /**
   * Register a new preset
   */
  void register_object(const string& id_string, PRESET* object);

  /**
   * List of registered presets ('preset name'-'keyword' map).
   */
  const map<string,string>& registered_objects(void) const;

  /**
   * Return the first object that matches with 'keyword'
   */
  ECA_OBJECT* object(const string& keyword) const;

  /**
   * Return the matching keyword for 'object'.
   */
  string object_identifier(const PRESET* object) const;

  ECA_PRESET_MAP(void);
  virtual ~ECA_PRESET_MAP(void) { }
};

#endif
