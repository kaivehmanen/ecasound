#ifndef INCLUDE_MIDI_CC_H
#define INCLUDE_MIDI_CC_H

#include <string>

#include "ctrl-source.h"
#include "midi-client.h"

/**
 * Interface to MIDI continuous controllers
 */
class MIDI_CONTROLLER : public CONTROLLER_SOURCE,
			public MIDI_CLIENT {
    
  int controller, channel;
  parameter_type value_rep;
  bool trace_request_rep;

 public:

  /**
   * Initialize MIDI-controller (child-thread is spawned)
   */
  void init(parameter_type phasestep);

  std::string parameter_names(void) const { return("controller,channel"); }
  void set_parameter(int param, parameter_type value);
  parameter_type get_parameter(int param) const;

  std::string name(void) const {  return("MIDI-Controller"); }
  parameter_type value(void);
 
  MIDI_CONTROLLER* clone(void)  { return new MIDI_CONTROLLER(*this); }
  MIDI_CONTROLLER* new_expr(void)  { return new MIDI_CONTROLLER(); }
  MIDI_CONTROLLER(int controller_number = 0, int midi_channel = 0);
};

#endif
