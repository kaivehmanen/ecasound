#ifndef QEFADEINEVENT_H
#define QEFADEINEVENT_H

#include <string>
#include "qeblockingevent.h"

/**
 * Fade-in effect
 */
class QEFadeInEvent : public QEBlockingEvent {

 public:

  QEFadeInEvent(ECA_CONTROLLER* ctrl,
		const string& input,
		const string& output,
		long int insert_pos,
		long int insert_length); 

 private:

  ECA_CONTROLLER* ectrl;
};

#endif
