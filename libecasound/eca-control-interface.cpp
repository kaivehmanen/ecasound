// ------------------------------------------------------------------------
// eca-control-interface.cpp: C++ implementation of the Ecasound
//                            Control Interface
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include "eca-session.h"
#include "eca-control.h"

#include "eca-control-interface.h"

/**
 * Class constructor.
 */
ECA_CONTROL_INTERFACE::ECA_CONTROL_INTERFACE (void) { 
  session_repp = new ECA_SESSION();
  control_repp = new ECA_CONTROL(session_repp);
}

/**
 * Desctructor.
 */
ECA_CONTROL_INTERFACE::~ECA_CONTROL_INTERFACE (void) { 
  delete session_repp;
  delete control_repp;
}

/**
 * Parse string mode command and act accordingly.
 */
void ECA_CONTROL_INTERFACE::command(const string& cmd) { 
  control_repp->command(cmd);
}

void ECA_CONTROL_INTERFACE::command_float_arg(const string& cmd, double arg) {
  control_repp->command_float_arg(cmd, arg);
}

const vector<string>& ECA_CONTROL_INTERFACE::last_list_of_strings(void) const { return(control_repp->last_list_of_strings()); }
const string& ECA_CONTROL_INTERFACE::last_string(void) const { return(control_repp->last_string()); }
double ECA_CONTROL_INTERFACE::last_float(void) const { return(control_repp->last_float()); }
int ECA_CONTROL_INTERFACE::last_integer(void) const { return(control_repp->last_integer()); }
long int ECA_CONTROL_INTERFACE::last_long_integer(void) const { return(control_repp->last_long_integer()); }
const string& ECA_CONTROL_INTERFACE::last_error(void) const { return(control_repp->last_error()); }
const string& ECA_CONTROL_INTERFACE::last_type(void) const { return(control_repp->last_type()); }

bool ECA_CONTROL_INTERFACE::events_available(void) { return(false); }
void ECA_CONTROL_INTERFACE::next_event(void) { }
const string& ECA_CONTROL_INTERFACE::current_event(void) { return(current_event_rep); }
