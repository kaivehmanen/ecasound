#include "eca-debug.h"

class DEFAULTDEBUG : public ECA_DEBUG {
  
public:

  void flush(void) { }

  void control_flow(const string& part) { 
    if ((get_debug_level() & ECA_DEBUG::module_flow) != ECA_DEBUG::module_flow) return;
    cerr << "[* " << part << " *]" << endl;
  }
  void msg(int level, const string& info) { 
    if ((get_debug_level() & level) != level) return;
    cerr << info << "\n"; 
  }

  DEFAULTDEBUG(void) { }
};

static DEFAULTDEBUG ecasound_default_debug;
ECA_DEBUG* ecadebug = &ecasound_default_debug;

void attach_debug_object(ECA_DEBUG* newdebug) {
  ::ecadebug = newdebug;
}
