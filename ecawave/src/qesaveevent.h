#ifndef QESAVEEVENT_H
#define QESAVEEVENT_H

#include <string>

#include <kvutils/definition_by_contract.h>

#include "qeevent.h"

/**
 * Simple audio-playback using the default output device
 */
class QESaveEvent : public QEBlockingEvent {

 public:

  bool class_invariant(void) { return(ectrl != 0); }
  QESaveEvent(ECA_CONTROLLER* ctrl,
	      const string& input,
	      const string& output,
	      long int start_pos, 
	      long int length);

 private:

  ECA_CONTROLLER* ectrl;
};

#endif
