#ifndef QESAVEEVENT_H
#define QESAVEEVENT_H

#include <string>

#include "qeblockingevent.h"

/**
 * Simple audio-playback using the default output device
 */
class QESaveEvent : public QEBlockingEvent {

 public:

  bool class_invariant(void) { return(ectrl != 0); }
  QESaveEvent(ECA_CONTROL* ctrl,
	      const string& input,
	      const string& output);

 private:

  ECA_CONTROL* ectrl;
};

#endif
