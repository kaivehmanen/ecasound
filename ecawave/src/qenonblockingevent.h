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
   *  ectrl->is_engine_started() == true
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

  /**
   * Tests whether processing has finished
   */
  bool is_finished(void) const { return(ectrl->is_finished()); }

  /**
   * Returns the current position in samples
   */
  virtual long int position_in_samples(void) const;

  QENonblockingEvent(ECA_CONTROL* ctrl) : QEEvent(ctrl), ectrl(ctrl) { }
  virtual ~QENonblockingEvent(void) { }

 private:

  ECA_CONTROL* ectrl;
};

#endif
