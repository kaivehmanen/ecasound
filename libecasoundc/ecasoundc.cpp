/** 
 * @file ecasoundc.cpp C-API to the ecasound control interface
 */

// ------------------------------------------------------------------------
// ecasoundc.cpp: C-API to the ecasound control interface
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

#include <eca-control-interface.h>
#include <eca-debug.h>
#include <eca-error.h>
#include "ecasoundc.h"

static struct eci_control_internals {
  ECA_CONTROL_INTERFACE* eci;
} eci_rep = { 0 };

/* ---------------------------------------------------------------------
 * Constructing and destructing                                       
 */

/**
 * Initializes session. This call clears all status info and
 * prepares ecasound for processing. Can be used to "restart"
 * the library.
 */
void eci_init(void) {
  if (eci_rep.eci != 0)
    delete eci_rep.eci;

//    ecadebug->set_debug_level(ECA_DEBUG::info |
//  			    ECA_DEBUG::module_flow);

  if (eci_rep.eci == 0) {
    eci_rep.eci = new ECA_CONTROL_INTERFACE();
  }
}

/**
 * Frees all resources.
 */
void eci_cleanup(void) {
  if (eci_rep.eci != 0)
    delete eci_rep.eci;
}

/* ---------------------------------------------------------------------
 * Issuing EIAM commands 
 */

/**
 * Sends a command to the ecasound engine. See ecasound-iam(5) for
 * more info.
 */
void eci_command(const char* command) {
  eci_rep.eci->command(command);
}

/** 
 * A specialized version of 'eci_command()' taking a double value
 * as the 2nd argument.
 */
void eci_command_float_arg(const char* command, double arg) {
  eci_rep.eci->command_float_arg(command, arg);
}

/* ---------------------------------------------------------------------
 * Getting return values 
 */

/**
 * Returns the number of strings returned by the 
 * last ECI command.
 */
int eci_last_list_of_strings_count(void) {
  return(eci_rep.eci->last_list_of_strings().size());
}

/**
 * Returns the nth item of the list containing 
 * strings returned by the last ECI command.
 *
 * require:
 *  n >= 0 && n < eci_last_list_of_strings_count()
 */
const char* eci_last_list_of_strings_item(int n) {
  return(eci_rep.eci->last_list_of_strings()[n].c_str());
}

const char* eci_last_string(void) {
  return(eci_rep.eci->last_string().c_str());
}

double eci_last_float(void) {
  return(eci_rep.eci->last_float());
}

int eci_last_integer(void) {
  return(eci_rep.eci->last_integer());
}

long int eci_last_long_integer(void) {
  return(eci_rep.eci->last_long_integer());
}

/**
 * Returns pointer to a null-terminated string containing 
 * information about the last occured error.
 */
const char* eci_last_error(void) {
  return(eci_rep.eci->last_error().c_str());
}

const char* eci_last_type(void) {
  return(eci_rep.eci->last_type().c_str());
}
 
/* --------------------------------------------------------------------- 
 * Events 
 */

int eci_events_available(void) { return(0); }
void eci_next_event(void) { }
const char* eci_current_event(void) { return(0); }
