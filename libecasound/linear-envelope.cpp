// ------------------------------------------------------------------------
// linear-envelope.cpp: Linear envelope
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include "eca-debug.h"

LINEAR_ENVELOPE::LINEAR_ENVELOPE(void)
  : FINITE_ENVELOPE(0)
{
} 

LINEAR_ENVELOPE::~LINEAR_ENVELOPE(void)
{
}

CONTROLLER_SOURCE::parameter_t LINEAR_ENVELOPE::value(void)
{
  change_position_in_seconds(step_length());
  if (position_in_seconds_exact() <= length_in_seconds_exact()) {
    curval = (position_in_seconds_exact() / length_in_seconds_exact());
  }
  return(curval);
}

void LINEAR_ENVELOPE::init(CONTROLLER_SOURCE::parameter_t step)
{
  step_length(step);

  MESSAGE_ITEM otemp;
  otemp << "(linear-envelope) Fade-in enveloped initialized; length ";
  otemp.setprecision(3);
  otemp << length_in_seconds();
  otemp << " seconds.";
  ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
}

void LINEAR_ENVELOPE::set_parameter(int param, CONTROLLER_SOURCE::parameter_t value)
{
  switch (param) {
  case 1:
    DBC_CHECK(samples_per_second() > 0);
    set_length_in_seconds(value);
    set_position_in_samples(0);
    curval = 0.0;
    break;
  }
}

CONTROLLER_SOURCE::parameter_t LINEAR_ENVELOPE::get_parameter(int param) const
{
  switch (param) {
  case 1:
    return(length_in_seconds());
  }
  return(0.0);
}
