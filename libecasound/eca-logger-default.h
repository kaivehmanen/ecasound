#ifndef INCLUDE_ECA_LOGGER_DEFAULT_H
#define INCLUDE_ECA_LOGGER_DEFAULT_H

#include <iostream>
#include <string>

#include "eca-logger-interface.h"

/**
 * Default logging subsystem implementation.
 *
 * @author Kai Vehmanen
 */
class ECA_LOGGER_DEFAULT : public ECA_LOGGER_INTERFACE {
  
public:

  virtual void do_msg(ECA_LOGGER::Msg_level_t level, const std::string& module_name, const std::string& log_message) { 
    if (is_log_level_set(level) == true) {
      if (level == ECA_LOGGER::subsystems) {
	std::cerr << "[* ";
      }
      
      if (is_log_level_set(ECA_LOGGER::module_names) == true) {
	std::cerr << module_name << ": ";
      }

      std::cerr << log_message;

      if (level == ECA_LOGGER::subsystems) {
	std::cerr << " *]";
      }
      std::cerr << std::endl;
    }
  }

  virtual void do_flush(void) { }
  virtual void do_log_level_changed(void) { }
  virtual ~ECA_LOGGER_DEFAULT(void) { }
};

#endif /* INCLUDE_ECA_LOGGER_DEFAULT_H */
