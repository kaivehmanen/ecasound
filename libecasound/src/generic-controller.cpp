// ------------------------------------------------------------------------
// generic_controller.cpp: General sources for control signals
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
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

#include <cmath>
#include <string>

#include <kvutils/kvu_numtostr.h>

#include "generic-controller.h"
#include "eca-chainop.h"

#include "eca-debug.h"

void GENERIC_CONTROLLER::process(void) {
  // --------
  require(is_valid() == true, __FILE__, __LINE__);
  // --------

  new_value = (source->value() * (rangehigh - rangelow)) + rangelow;
  
  //  ecadebug->msg(ECA_DEBUG::user_objects, "Setting new parameter value... to " + kvu_numtostr(new_value) + ".");
  target->set_parameter(param_id, new_value);
}

GENERIC_CONTROLLER* GENERIC_CONTROLLER::clone(void) {
  assert(source != 0);
  CONTROLLER_SOURCE* s = source->clone();

  OPERATOR* t = 0;
  if (target != 0)
    t = target->clone();

  GENERIC_CONTROLLER* obj = new GENERIC_CONTROLLER(*this);

  if (t != 0) obj->assign_target(t);
  obj->assign_source(s);

  return(obj);
}

GENERIC_CONTROLLER::GENERIC_CONTROLLER(CONTROLLER_SOURCE* src, OPERATOR* dobj, int par_id, double range_low, double range_high) {
  source = src;
  target = dobj;
  param_id = par_id;
  rangelow = range_low;
  rangehigh = range_high;
}

string GENERIC_CONTROLLER::status(void) const {
  if (is_valid() == true) {
    return("Source \"" + source->name() + 
	   "\" connected to target \"" +
	   target->name() + "\"." +
	   " Current source value is " + 
	   kvu_numtostr(source->value()) + 
	   " and target " + 
	   kvu_numtostr(target->get_parameter(param_id)) + ".");
  }
  else {
    return("Controller not valid.");
  }
}

void GENERIC_CONTROLLER::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 
    param_number(static_cast<int>(value));
    break;
  case 2: 
    low_range_limit(value);
    break;
  case 3: 
    high_range_limit(value);
    break;
  default:
    source->set_parameter(param - 3, value);
  }
}

CHAIN_OPERATOR::parameter_type GENERIC_CONTROLLER::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return(static_cast<parameter_type>(param_number()));
  case 2: 
    return(low_range_limit());
  case 3: 
    return(high_range_limit());
  default:
    return(source->get_parameter(param - 3));
  }
  return(0.0);
}
