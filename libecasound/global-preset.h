#ifndef _GLOBAL_PRESET_H
#define _GLOBAL_PRESET_H

#include "preset.h"

/**
 * Effect preset that is read from a global 
 * preset database.
 *
 * @author Kai Vehmanen
 */
class GLOBAL_PRESET : public PRESET {

 private:

  string preset_name_rep;

 public:

  virtual GLOBAL_PRESET* clone(void) { return(new GLOBAL_PRESET(preset_name_rep)); }
  virtual GLOBAL_PRESET* new_expr(void) { return(new GLOBAL_PRESET(preset_name_rep)); }
  virtual ~GLOBAL_PRESET (void) { }

  GLOBAL_PRESET(const string& preset_name = "");
};

#endif
