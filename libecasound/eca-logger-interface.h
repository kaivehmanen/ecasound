#ifndef INCLUDE_ECA_LOGGER_INTERFACE_H
#define INCLUDE_ECA_LOGGER_INTERFACE_H

#include <iostream> /* remove me */
#include <string>
#include "eca-logger.h"

/**
 * Virtual base class for logging subsystem implementations.
 *
 * @author Kai Vehmanen
 */
class ECA_LOGGER_INTERFACE {

public:

  /**
   * Class constructor. Initializes log level to 'disabled'.
   */
  ECA_LOGGER_INTERFACE(void) : debug_value_rep(0) { }

  /**
   * Class destructor.
   */
  virtual ~ECA_LOGGER_INTERFACE(void) { }

  /**
   * Issues a generic log message.
   */
  void msg(ECA_LOGGER::Msg_level_t level, const std::string& module_name, const std::string& log_message) { 
    do_msg(level, module_name, log_message); 
  }

  /**
   * Flush all log messages.
   */
  void flush(void) { do_flush(); }

  /**
   * Sets logging level to 'level' state to 'enabled'.
   */
  void set_log_level(ECA_LOGGER::Msg_level_t level, bool enabled) { 
    if (enabled == true) {
      debug_value_rep |= level;
    }
    else {
      debug_value_rep &= ~level;
    }
  }

  /**
   * Gets current log level bitmask.
   */
  int get_log_level_bitmask(void) const {
    return(debug_value_rep);
  }
  
  /**
   * Sets state of all logging types according to 'bitmask'.
   */
  void set_log_level_bitmask(int level_bitmask) {
    debug_value_rep = static_cast<ECA_LOGGER::Msg_level_t>(level_bitmask);
  }
 
  /**
   * Whether 'level' is set or not?
   */
  bool is_log_level_set(ECA_LOGGER::Msg_level_t level) const { return((level & debug_value_rep) > 0 ? true : false); }

  /**
   * Disables logging.
   * 
   * Note! Is equivalent to 
   * 'set_log_level(ECA_LOGGER_INTERFACE::disabled)'.
   */
  void disable(void) { debug_value_rep = 0; }

  protected:

  virtual void do_msg(ECA_LOGGER::Msg_level_t level, const std::string& module_name, const std::string& log_message) = 0;
  virtual void do_flush(void) = 0;
  virtual void do_log_level_changed(void) = 0;

  private:

  int debug_value_rep;

};

#endif
