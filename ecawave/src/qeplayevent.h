#ifndef QEPLAYEVENT_H
#define QEPLAYEVENT_H

#include <string>

#include "qenonblockingevent.h"

/**
 * Simple audio-playback using the default output device
 */
class QEPlayEvent : public QENonblockingEvent {

 public:

  virtual long int position_in_samples(void) const;
  virtual bool class_invariant(void) { return(ectrl != 0); }

  QEPlayEvent(ECA_CONTROL* ctrl,
	      const string& input,
	      const string& output,
	      long int start_pos, 
	      long int length);

 private:

  ECA_CONTROL* ectrl;
  long int start_pos_rep;
};

#endif
