#include "eca-debug.h"

class DEFAULTDEBUG : public MAINDEBUG {
  
  int dlevel;

public:

  void flush(void) { }
  void set_debug_level(int level) { dlevel = level; }
  int get_debug_level(void) { return(dlevel); }

  void control_flow(const string& part) { 
    if (is_enabled()) cerr << part << "\n";
  }
  void msg(const string& info) { 
    if (is_enabled()) cerr << info << "\n";
  }
  void msg(int level, const string& info) { 
    if (is_enabled() 
	&& dlevel >= level) 
      cerr << info << "\n"; 
  }

  DEFAULTDEBUG(void) : dlevel(0) { disable(); }
};

DEFAULTDEBUG ddebug;
MAINDEBUG* ecadebug = &ddebug;

void attach_debug_object(MAINDEBUG* newdebug) {
  ecadebug = newdebug;
}
