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
  string current_event_rep;

 public:

  // -------------------------------------------------------------------
  // Issuing EIAM commands
  // -------------------------------------------------------------------

  void command(const string& cmd);
  void command_float_arg(const string& cmd, double arg);

  // -------------------------------------------------------------------
  // Getting return values
  // -------------------------------------------------------------------

  const vector<string>& last_list_of_strings(void) const;
  const string& last_string(void) const;
  double last_float(void) const;
  int last_integer(void) const;
  long int last_long_integer(void) const;
  const string& last_error(void) const;
  const string& last_type(void) const;
  
  // -------------------------------------------------------------------
  // Events
  // -------------------------------------------------------------------

  bool events_available(void);
  void next_event(void);
  const string& current_event(void);

  // -------------------------------------------------------------------
  // Constructors and destructors
  // -------------------------------------------------------------------

  ECA_CONTROL_INTERFACE(void);
  ~ECA_CONTROL_INTERFACE(void);
};

#endif
