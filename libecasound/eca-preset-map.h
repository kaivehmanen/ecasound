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

  void register_object(const string& id_string, PRESET* object);
  const map<string,string>& registered_objects(void) const;

  /**
   * Returns the first object that matches 'keyword'. Regular 
   * expressions are not used.
   */
  ECA_OBJECT* object(const string& keyword, bool use_regexp = false) const;
  string object_identifier(const PRESET* object) const;

  ECA_PRESET_MAP(void);
  virtual ~ECA_PRESET_MAP(void);
};

#endif
