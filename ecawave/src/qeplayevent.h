#ifndef QEPLAYEVENT_H
#define QEPLAYEVENT_H

#include <string>

#include "qenonblockingevent.h"

/**
 * Simple audio-playback using the default output device
 */
class QEPlayEvent : public QENonblockingEvent {

 public:

  bool class_invariant(void) { return(ectrl != 0); }

  QEPlayEvent(ECA_CONTROLLER* ctrl,
	      const string& input,
	      const string& output,
	      long int start_pos, 
	      long int length);

 private:

  ECA_CONTROLLER* ectrl;
};

#endif
