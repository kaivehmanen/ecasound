// ------------------------------------------------------------------------
// two-stage-linear-envelope.cpp: Two-stage linear envelope
// Copyright (C) 2000,2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <kvutils/kvu_numtostr.h>
#include <kvutils/message_item.h>

#include "two-stage-linear-envelope.h"
#include "eca-debug.h"

CONTROLLER_SOURCE::parameter_t TWO_STAGE_LINEAR_ENVELOPE::value(void) {
  curpos += step_length();
  if (curpos > first_stage_length_rep) {
    if (curpos <= length_in_seconds()) {
      curval = ((curpos - first_stage_length_rep) /
		second_stage_length_rep);
    }
    else 
      curval = 1.0;
  }
  return(curval);
}

TWO_STAGE_LINEAR_ENVELOPE::TWO_STAGE_LINEAR_ENVELOPE(void) {
  set_parameter(1, get_parameter(1));
  set_parameter(2, get_parameter(2));
} 

void TWO_STAGE_LINEAR_ENVELOPE::init(CONTROLLER_SOURCE::parameter_t step) {
  step_length(step);

  MESSAGE_ITEM otemp;
  otemp << "(two-stage-linear-envelope) Envelope initialized: ";
  otemp.setprecision(3);
  otemp << "1." << get_parameter(1);
  otemp << "- 2." << get_parameter(2);
  ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
}

void TWO_STAGE_LINEAR_ENVELOPE::set_parameter(int param, CONTROLLER_SOURCE::parameter_t value) {
  switch (param) {
  case 1:
    {
      first_stage_length_rep = value;
      length_in_seconds(first_stage_length_rep + second_stage_length_rep);
      curpos = 0.0;
      curval = 0.0;
      break;
    }
  case 2:
    {
      second_stage_length_rep = value;
      length_in_seconds(first_stage_length_rep + second_stage_length_rep);
      curpos = 0.0;
      curval = 0.0;
      break;
    }
  }
}

CONTROLLER_SOURCE::parameter_t TWO_STAGE_LINEAR_ENVELOPE::get_parameter(int param) const {
  switch (param) {
  case 1:
    return(first_stage_length_rep);

  case 2:
    return(second_stage_length_rep);
  }
  return(0.0);
}
