#ifndef QENONBLOCKINGEVENT_H
#define QENONBLOCKINGEVENT_H

#include "qeevent.h"

/**
 * Virtual base for non-blocking processing events
 */
class QENonblockingEvent : public QEEvent {

 public:

  /**
   * Start processing
   */
  virtual void start(void) { nonblocking_start(); }

  /**
   * Stop processing
   *
   * require:
   *  ectrl->is_running() == true
   *  is_triggered() == true
   *
   * ensure:
   *  ectrl->is_connected() == false
   *  is_triggered() == false
   */
  virtual void stop(void);

  /**
   * Restart event with new position parameters
   */
  virtual void restart(long int start_pos, long int length) { }

  QENonblockingEvent(ECA_CONTROLLER* ctrl) : QEEvent(ctrl), ectrl(ctrl) { }
  virtual ~QENonblockingEvent(void) { }

 private:

  ECA_CONTROLLER* ectrl;
};

#endif
