#ifndef QEPROCESSEVENT_H
#define QEPROCESSEVENT_H

#include <kvutils/definition_by_contract.h>
#include <ecasound/eca-chainop.h>

#include "qeevent.h"

/**
 * Single-chain effect processing
 */
class QEProcessEvent : public QEEvent {

 protected:

  /**
   * Set input source using a formatted string (refer to ecasound's documentation)
   */
  void add_chain_operator(CHAIN_OPERATOR* cop);

 public:

  virtual void restart(long int start_pos, long int length) = 0;

  QEProcessEvent(ECA_CONTROLLER* ctrl);
};

#endif
