#ifndef INCLUDED_ECA_CONTROL_INTERFACE_H
#define INCLUDED_ECA_CONTROL_INTERFACE_H

/**
 * C++ implementation of the Ecasound Control Interface
 * @author Kai Vehmanen
 */
class ECA_CONTROL_INTERFACE {

 public:

  void command(const string& cmd);

  ECA_CONTROL_INTERFACE(void);
};

#endif
