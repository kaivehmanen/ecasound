// ------------------------------------------------------------------------
// midi-cc.cpp: Interface to MIDI continuous controllers
// Copyright (C) 1999,2001 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is fre software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#include <string>
#include <iostream>

#include <kvutils/message_item.h>

//  #include "eca-midi.h"
#include "midi-client.h"
#include "midi-server.h"
#include "midi-cc.h"

#include "eca-debug.h"

CONTROLLER_SOURCE::parameter_type MIDI_CONTROLLER::value(void) {
//    if (midi_in_queue.update_controller_value()) {
//      value_rep =
//        static_cast<double>(midi_in_queue.last_controller_value(static_cast<int>(channel), static_cast<int>(controller)));
//      value_rep /= 127.0;
//    }
  if (trace_request_rep == true) {
    if (server() != 0) {
      server()->add_controller_trace(channel, 
				     controller);
    }
    else {
      std::cerr << "(midi-cc) Warning! No MIDI-server found!" << std::endl;
    }
    trace_request_rep = false;
  }
  if (server() != 0)
    value_rep =
      static_cast<double>(server()->last_controller_value(channel, controller));
      
  value_rep /= 127.0;
  return(value_rep);
}

MIDI_CONTROLLER::MIDI_CONTROLLER(int controller_number, 
				 int midi_channel) 
  : controller(controller_number), 
    channel(midi_channel),
    value_rep(0.0),
    trace_request_rep(false) 
{ }

void MIDI_CONTROLLER::init(CONTROLLER_SOURCE::parameter_type phasestep) {
//      init_midi_queues();

    MESSAGE_ITEM otemp;
    otemp << "(midi-cc) MIDI-controller initialized using controller ";
    otemp.setprecision(0);
    otemp << controller << " and channel " << channel << ".";
    ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());

}

void MIDI_CONTROLLER::set_parameter(int param, CONTROLLER_SOURCE::parameter_type value) {
  switch (param) {
  case 1: 
    controller = static_cast<int>(value);
    break;
  case 2: 
    channel = static_cast<int>(value);
    break;
  }
  trace_request_rep = true;
}

CONTROLLER_SOURCE::parameter_type MIDI_CONTROLLER::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(static_cast<parameter_type>(controller));
  case 2: 
    return(static_cast<parameter_type>(channel));
  }
  return(0.0);
}
