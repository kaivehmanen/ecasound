#ifndef QECOPYEVENT_H
#define QECOPYEVENT_H

#include <string>

#include "qeblockingevent.h"

/**
 * Copy specified range from input to output. Output
 * is truncated before use.
 */
class QECopyEvent : public QEBlockingEvent {

 public:

  QECopyEvent(ECA_CONTROL* ctrl,
	      const string& input,
	      const string& output,
	      long int start_pos, 
	      long int length);

 private:

  ECA_CONTROL* ectrl;
};

#endif


