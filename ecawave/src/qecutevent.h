#ifndef QECUTEVENT_H
#define QECUTEVENT_H

#include <string>

#include "qeblockingevent.h"

/**
 * Cut specified range to from input to output. Output
 * is truncated before use.
 */
class QECutEvent : public QEBlockingEvent {

 public:

  virtual void start(void);

  QECutEvent(ECA_CONTROL* ctrl,
	      const string& input,
	      const string& output,
	      long int start_pos, 
	      long int length);

 private:

  ECA_CONTROL* ectrl;
  long int start_pos_rep, length_rep;
  string input_rep, output_rep;
};

#endif




