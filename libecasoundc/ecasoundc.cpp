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

#include <eca-control.h>
#include <eca-debug.h>
#include "ecasoundc.h"

static struct ecac_control_internals {
  ECA_SESSION* session;
  ECA_CONTROL* ctrl;
} ecac_rep = { 0, 0 };

static int ecac_error_rep;
static string ecac_error_string_rep;

/**
 * Initializes session. This call clears all status info and
 * prepares ecasound for processing. Can be used to "restart"
 * the library.
 */
void ecac_init(void) {
  cerr << "(ecasoundc) ecac_init" << endl;
  if (ecac_rep.ctrl != 0)
    delete ecac_rep.ctrl;
  if (ecac_rep.session != 0)
    delete ecac_rep.session;

  ecadebug->set_debug_level(ECA_DEBUG::info |
			    ECA_DEBUG::module_flow);

  if (ecac_rep.ctrl == 0) {
    ecac_rep.session = new ECA_SESSION();
    ecac_rep.ctrl = new ECA_CONTROL (ecac_rep.session);
  }
}

/**
 * Frees all resources.
 */
void ecac_cleanup(void) {
  cerr << "(ecasoundc) ecac_cleanup" << endl;
  if (ecac_rep.ctrl != 0)
    delete ecac_rep.ctrl;
  if (ecac_rep.session != 0)
    delete ecac_rep.session;
}

/**
 * Sends a command to the ecasound engine. See ecasound-iam(5) for
 * more info.
 */
void ecac_command(const char* command) {
  cerr << "(ecasoundc) ecac_command" << endl;
  try {
    ecac_error_rep = 0;
    ecac_rep.ctrl->command(command);
  }
  catch(ECA_ERROR& e) {
    ecac_error_rep = 1;
    ecac_error_string_rep = e.error_section() + ": \"" + e.error_message();
    cerr << "(ecasoundc) Error in ecac_command()!" << endl;
  }
}

/**
 * Boolean value reporting whether error has occured 
 * during last ecasoundc library call.
 */
int ecac_error(void) { return(ecac_error_rep); }

/**
 * Returns pointer to a null-terminated string containing 
 * information about the last occured error.
 */
const char* ecac_error_string(void) {
  return(ecac_error_string_rep.c_str());
}

/**
 * Fills the structure pointed by 'status'. Provides
 * info like "running/stopped", position, active objects,
 * selected options, etc.
 */
//  void ecac_engine_status(eca_engine_status_t* status);
