#ifndef INCLUDED_ECA_CONTROL_INTERFACE_H
#define INCLUDED_ECA_CONTROL_INTERFACE_H

#include <string>
#include <vector>

class ECA_SESSION;
class ECA_CONTROL;

/**
 * C++ implementation of the Ecasound Control Interface
 * @author Kai Vehmanen
 */
class ECA_CONTROL_INTERFACE {

  ECA_SESSION* session_repp;
  ECA_CONTROL* control_repp;
  std::string current_event_rep;

 public:

  // -------------------------------------------------------------------
  // Issuing EIAM commands
  // -------------------------------------------------------------------

  void command(const std::string& cmd);
  void command_float_arg(const std::string& cmd, double arg);

  // -------------------------------------------------------------------
  // Getting return values
  // -------------------------------------------------------------------

  const std::vector<std::string>& last_string_list(void) const;
  const std::string& last_string(void) const;
  double last_float(void) const;
  int last_integer(void) const;
  bool last_bool(void) const;
  long int last_long_integer(void) const;
  const std::string& last_error(void) const;
  const std::string& last_type(void) const;
  bool error(void) const;

  /**
   * Returns last_integer() interpreted as a bool.
   */
  bool last_boolean(void) const { return(last_integer() != 0); }
  
  // -------------------------------------------------------------------
  // Events
  // -------------------------------------------------------------------

  bool events_available(void);
  void next_event(void);
  const std::string& current_event(void);

  // -------------------------------------------------------------------
  // Constructors and destructors
  // -------------------------------------------------------------------

  ECA_CONTROL_INTERFACE(void);
  ~ECA_CONTROL_INTERFACE(void);
};

#endif
