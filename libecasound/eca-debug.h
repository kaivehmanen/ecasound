#ifndef INCLUDE_ECA_DEBUG_H
#define INCLUDE_ECA_DEBUG_H

#include <string>

/**
 * A virtual base class representing a generic 
 * debugging subsystem.
 *
 * Debug level is a bitmasked integer value based on the following 
 * enum values:
 *
 * disabled - no output
 *
 * info - high-level info about user-visible objects and concepts
 * 
 * module_flow - high-level info about program control flow 
 *
 * system_objects - debug info about internal objects
 *
 * user_objects - debug info about user-visibe objects (audio i/o, chain operators, controllers)
 *
 * buffer_level - debug info that is printed for each processed buffer
 *
 * sample_level -  debug info printed for individual samples
 */
class ECA_DEBUG {

  int debug_value_rep;

public:

  enum {
    disabled = 0,
    info = 1,
    module_flow = 2,
    user_objects = 4,
    system_objects = 8,
    buffer_level = 16,
    sample_level = 32
  };

  virtual void flush(void) = 0;

  void set_debug_level(int level) { debug_value_rep = level; }
  int get_debug_level(void) const { return(debug_value_rep); } 
  void disable(void) { debug_value_rep = disabled; }

  virtual void control_flow(const std::string& part) = 0;
  virtual void msg(const std::string& arg) { msg(module_flow, arg); }
  virtual void msg(int level, const std::string& info) = 0;

  ECA_DEBUG(void) : debug_value_rep(0) { }
  virtual ~ECA_DEBUG(void) { }
};

void attach_debug_object(ECA_DEBUG* newdebug);
void detach_debug_object(void);

extern ECA_DEBUG* ecadebug;

#endif
