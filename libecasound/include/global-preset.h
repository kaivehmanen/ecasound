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

 public:

  virtual GLOBAL_PRESET* clone(void) { return(new GLOBAL_PRESET(*this)); }
  virtual ~GLOBAL_PRESET (void) { }

  GLOBAL_PRESET(const string& preset_name);
};

#endif
