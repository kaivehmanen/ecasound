#ifndef QEBLOCKINGEVENT_H
#define QEBLOCKINGEVENT_H

#include "qeevent.h"

/**
 * Virtual base for blocking processing events
 */
class QEBlockingEvent : public QEEvent {

 public:

  /**
   * Start processing
   */
  virtual void start(void) { blocking_start(); }

  QEBlockingEvent(ECA_CONTROLLER* ctrl) : QEEvent(ctrl) { }
  virtual ~QEBlockingEvent(void) { }
};

#endif
