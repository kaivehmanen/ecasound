#ifndef _FINITE_ENVELOPE_H
#define _FINITE_ENVELOPE_H

#include <string>

#include "ctrl-source.h"
#include "eca-audio-position.h"

/**
 * Controller envelopes that have finite length
 */
class FINITE_ENVELOPE : public CONTROLLER_SOURCE,
			public ECA_AUDIO_POSITION

{

 public:

  virtual std::string name(void) const = 0;
  virtual parameter_t value(void) = 0;

  /**
   * Constructor
   * 
   * @param time_in_seconds Envelope length in seconds
   */
  FINITE_ENVELOPE(parameter_t time_in_seconds = 0.0) { set_length_in_seconds(time_in_seconds); }

};

#endif

