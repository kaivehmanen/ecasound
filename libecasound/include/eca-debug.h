#ifndef _DEBUG_H
#define _DEBUG_H

#include <string>

class MAINDEBUG {

  bool is_enabled_value;

public:

  virtual void flush(void) = 0;
  virtual void set_debug_level(int level) = 0;
  virtual int get_debug_level(void) = 0; 

  void enable(void) { is_enabled_value = true; }
  void disable(void) { is_enabled_value = false; }
  bool is_enabled(void) { return(is_enabled_value); }

  virtual ~MAINDEBUG(void) { }

  virtual void control_flow(const string& part) = 0;
  virtual void msg(const string& info) = 0;
  virtual void msg(int level, const string& info) = 0;
};

void attach_debug_object(MAINDEBUG* newdebug);

extern MAINDEBUG* ecadebug;

#endif

