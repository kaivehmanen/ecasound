#ifndef _ECA_PRESET_MAP_H
#define _ECA_PRESET_MAP_H

#include <string>
#include <vector>
#include <map>

#include "preset.h"

/**
 * Dynamic register for storing effect presets
 *
 * @author Kai Vehmanen
 */
class ECA_PRESET_MAP {

  /**
   * Vector of preset names
   */
  static vector<string> preset_map;

 public:

  /**
   * Returns a vector of registered presets
   */
  static const vector<string>& available_presets(void);

  /**
   * Whether 'preset_name' exists
   */
  static bool has(const string& preset_name);
};

#endif
