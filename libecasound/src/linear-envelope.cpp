// ------------------------------------------------------------------------
// linear-envelope.cpp: Linear envelope
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <kvutils/kvutils.h>

#include "linear-envelope.h"
#include "eca-debug.h"

CONTROLLER_SOURCE::parameter_type LINEAR_ENVELOPE::value(void) {
  curpos += step_length();
  if (curpos <= length_in_seconds()) {
    curval = (curpos / length_in_seconds());
  }
  return(curval);
}

LINEAR_ENVELOPE::LINEAR_ENVELOPE(CONTROLLER_SOURCE::parameter_type time_in_seconds)
  : FINITE_ENVELOPE(time_in_seconds) {
  set_parameter(1, get_parameter(1));
} 

void LINEAR_ENVELOPE::init(CONTROLLER_SOURCE::parameter_type step) {
  step_length(step);

  MESSAGE_ITEM otemp;
  otemp << "(linear-envelope) Fade-in enveloped initialized; length ";
  otemp.setprecision(3);
  otemp << length_in_seconds();
  otemp << " seconds.";
  ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
}

void LINEAR_ENVELOPE::set_parameter(int param, CONTROLLER_SOURCE::parameter_type value) {
  switch (param) {
  case 1:
    length_in_seconds(value);
    curpos = 0.0;
    curval = 0.0;
    break;
  }
}

CONTROLLER_SOURCE::parameter_type LINEAR_ENVELOPE::get_parameter(int param) const {
  switch (param) {
  case 1:
    return(length_in_seconds());
  }
  return(0.0);
}
