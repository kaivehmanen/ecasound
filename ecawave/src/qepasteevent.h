#ifndef QEPASTEEVENT_H
#define QEPASTEEVENT_H

#include <string>

#include "qeblockingevent.h"

/**
 * Paste input contents into the output at specified position
 */
class QEPasteEvent : public QEBlockingEvent {

 public:

  QEPasteEvent(ECA_CONTROL* ctrl,
	       const string& input,
	       const string& output,
	       long int insert_pos); 

 private:

  ECA_CONTROL* ectrl;
};

#endif
