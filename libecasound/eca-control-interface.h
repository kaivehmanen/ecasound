#ifndef INCLUDED_ECA_CONTROL_INTERFACE_H
#define INCLUDED_ECA_CONTROL_INTERFACE_H

#include <string>

class ECA_SESSION;
class ECA_CONTROL;

/**
 * C++ implementation of the Ecasound Control Interface
 * @author Kai Vehmanen
 */
class ECA_CONTROL_INTERFACE {

  ECA_SESSION* session_repp;
  ECA_CONTROL* control_repp;

 public:

  void command(const string& cmd);

  ECA_CONTROL_INTERFACE(void);
  ~ECA_CONTROL_INTERFACE(void);
};

#endif
