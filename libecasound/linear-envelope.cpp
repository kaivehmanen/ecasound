// ------------------------------------------------------------------------
// linear-envelope.cpp: Linear envelope
// Copyright (C) 1999-2002,2005,2008 Kai Vehmanen
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

#include "linear-envelope.h"
#include "eca-logger.h"

LINEAR_ENVELOPE::LINEAR_ENVELOPE(void)
{
} 

LINEAR_ENVELOPE::~LINEAR_ENVELOPE(void)
{
}

CONTROLLER_SOURCE::parameter_t LINEAR_ENVELOPE::value(void)
{
  parameter_t retval;

  /* note: length_in_seconds_exact() depends only on
   *       envelope parameters and equals to envelope
   *       length (position may go beyond length() */

  if (position_in_seconds_exact() < length_in_seconds_exact()) {
    retval = (position_in_seconds_exact() / length_in_seconds_exact());
  }
  else 
    retval = 1.0f;

  return retval;
}

void LINEAR_ENVELOPE::init(void)
{
  MESSAGE_ITEM otemp;
  otemp << "Linear enveloped initialized; length ";
  otemp.setprecision(3);
  otemp << length_in_seconds_exact();
  otemp << " seconds.";
  ECA_LOG_MSG(ECA_LOGGER::user_objects, otemp.to_string());
}

void LINEAR_ENVELOPE::set_parameter(int param, CONTROLLER_SOURCE::parameter_t value)
{
  switch (param) {
  case 1:
    set_length_in_seconds(value);
    break;
  }
}

CONTROLLER_SOURCE::parameter_t LINEAR_ENVELOPE::get_parameter(int param) const
{
  switch (param) {
  case 1:
    return length_in_seconds_exact();
  }
  return 0.0;
}
