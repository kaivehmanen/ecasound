#ifndef INCLUDED_ECA_CONTROL_POSITION_H
#define INCLUDED_ECA_CONTROL_POSITION_H

#include "sample-specs.h"
#include "eca-audio-position.h"

/**
 * Virtual class implementing position and
 * length related chainsetup features.
 */
class ECA_CHAINSETUP_POSITION : public ECA_AUDIO_POSITION {

 public:

  // --
  // init and cleanup

  ECA_CHAINSETUP_POSITION(void);
  virtual ~ECA_CHAINSETUP_POSITION(void);

  inline bool is_over(void) const { return((position_in_samples() > length_in_samples() && length_set() == true) ? true : false); }

  // --
  // functions for setting and observing sample rate and looping

  void toggle_looping(bool v) { looping_rep = v; }
  bool looping_enabled(void) const { return(looping_rep); }

 private:

  bool looping_rep;
};

#endif
