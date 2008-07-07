// ------------------------------------------------------------------------
// two-stage-linear-envelope.cpp: Two-stage linear envelope
// Copyright (C) 2000-2002,2005,2008 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3 (see Ecasound Programmer's Guide)
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

#include <kvu_dbc.h>
#include <kvu_numtostr.h>
#include <kvu_message_item.h>

#include "two-stage-linear-envelope.h"
#include "eca-logger.h"

CONTROLLER_SOURCE::parameter_t TWO_STAGE_LINEAR_ENVELOPE::value(void)
{
  parameter_t curpos = position_in_seconds_exact();
  parameter_t retval;

  /* note: length_in_seconds_exact() depends only on
   *       envelope parameters and equals to envelope
   *       length (position may go beyond length() */
  
  /* phase: hold start value */
  if (curpos < first_stage_length_rep) {
    retval = 0.0f;
  }
  /* phase: linear fade */ 
  else if (curpos < length_in_seconds_exact()) {
    /* note: length_in_seconds_exact() is 
     *       cached value of 
     *       first_state_length_rep+second_state_length_rep
     */
    DBC_CHECK(second_stage_length_rep > 0);
    retval = ((curpos - first_stage_length_rep) /
	      second_stage_length_rep);
  }
  /* phase: hold end value */
  else 
    retval = 1.0;

  return retval;
}

TWO_STAGE_LINEAR_ENVELOPE::TWO_STAGE_LINEAR_ENVELOPE(void)
{
  first_stage_length_rep = 0;
  second_stage_length_rep = 0;
  set_parameter(1, get_parameter(1));
  set_parameter(2, get_parameter(2));
} 

void TWO_STAGE_LINEAR_ENVELOPE::init(void)
{
  MESSAGE_ITEM otemp;
  otemp << "Envelope initialized: ";
  otemp.setprecision(3);
  otemp << "1. " << get_parameter(1);
  otemp << ", 2. " << get_parameter(2);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, otemp.to_string());
}

void TWO_STAGE_LINEAR_ENVELOPE::set_parameter(int param, CONTROLLER_SOURCE::parameter_t value)
{
  switch (param) {
  case 1:
    {
      first_stage_length_rep = value;
      set_length_in_seconds(first_stage_length_rep + second_stage_length_rep);
      break;
    }
  case 2:
    {
      second_stage_length_rep = value;
      set_length_in_seconds(first_stage_length_rep + second_stage_length_rep);
      break;
    }
  }
}

CONTROLLER_SOURCE::parameter_t TWO_STAGE_LINEAR_ENVELOPE::get_parameter(int param) const
{
  switch (param) {
  case 1:
    return first_stage_length_rep;

  case 2:
    return second_stage_length_rep;
  }
  return 0.0;
}
