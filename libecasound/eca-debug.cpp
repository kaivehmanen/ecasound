#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef USE_CXX_STD_NAMESPACE
using std::cerr;
using std::endl;
#endif

#include "eca-debug.h"

class DEFAULTDEBUG : public ECA_DEBUG {
  
public:

  void flush(void) { }

  void control_flow(const std::string& part) { 
    if ((get_debug_level() & ECA_DEBUG::module_flow) != ECA_DEBUG::module_flow) return;
    std::cerr << "[* " << part << " *]" << std::endl;
  }
  void msg(int level, const std::string& info) { 
    if ((get_debug_level() & level) != level) return;
    std::cerr << info << "\n"; 
  }

  DEFAULTDEBUG(void) { }
};

static DEFAULTDEBUG ecasound_default_debug;

/**
 * Public object for libecasound modules
 */
ECA_DEBUG* ecadebug = &ecasound_default_debug;

void attach_debug_object(ECA_DEBUG* newdebug) {
  ecadebug = newdebug;
}

void detach_debug_object(void) {
  ecadebug = &ecasound_default_debug;
}
