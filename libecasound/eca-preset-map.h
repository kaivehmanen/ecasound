#ifndef INCLUDED_ECA_PRESET_MAP_H
#define INCLUDED_ECA_PRESET_MAP_H

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

  mutable std::map<std::string, PRESET*> object_map;
  mutable std::map<std::string,string> object_keyword_map;

  void load_preset_file(const std::string& fname);

 public:

  virtual void register_object(const std::string& id_string, PRESET* object);
  virtual void unregister_object(const std::string& keyword);
  virtual const std::map<std::string,std::string>& registered_objects(void) const;
  virtual ECA_OBJECT* object(const std::string& keyword, bool use_regexp = false) const;
  virtual std::string object_identifier(const PRESET* object) const;
  virtual void flush(void);

  ECA_PRESET_MAP(void);
  virtual ~ECA_PRESET_MAP(void);
};

#endif
