/** 
 * @file ecasoundc.cpp C-API to the ecasound control interface
 */

// ------------------------------------------------------------------------
// ecasoundc.cpp: C-API to the ecasound control interface
// Copyright (C) 2000,2001 Kai Vehmanen (kaiv@wakkanet.fi)
// Copyright (C) 2001 Aymeric Jeanneau (ajeanneau@cvf.fr)
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

typedef struct { ECA_CONTROL_INTERFACE* eci; } eci_internal_t;

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
 * Initializes session. This call creates a new ecasoundinstance and
 * prepares ecasound for processing. This function is reentrant; Be careful
 * when using mp3 : mpg123 is not reentrant.
 */
eci_handle_t eci_init_r(void) {
  eci_internal_t* eci_rep = (eci_internal_t*)calloc(1, sizeof(eci_internal_t));

  eci_rep->eci = new ECA_CONTROL_INTERFACE();
  return (eci_handle_t)eci_rep;
}


/**
 * Frees all resources.
 */
void eci_cleanup(void) {
  if (eci_rep.eci != 0)
    delete eci_rep.eci;
}

/**
 * Frees all resources.
 */
void eci_cleanup_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  if(eci_rep != 0) {
    if (eci_rep->eci != 0) {
      delete eci_rep->eci;
    }
    free(eci_rep);
  }
}

/* ---------------------------------------------------------------------
 * Issuing EIAM commands 
 */

/**
 * Sends a command to the ecasound engine. See ecasound-iam(5) for
 * more info.
 */
void eci_command(const char* command) {
//    try {
    eci_rep.eci->command(command);
//    }
//    catch(ECA_ERROR& e) {
//      cerr << "---\nERROR: [" << e.error_section() << "] : \"" <<
//        e.error_message() << "\"\n\n";
//    }
//    catch (exception& ex) {
//      cerr << "Ex: " << ex.what() << "." << endl;
//    }
//    catch(...) {
//      cerr << "Caught an exception!" << endl;
//    }
}

/**
 * Sends a command to the ecasound engine. See ecasound-iam(5) for
 * more info.
 */
void eci_command_r(eci_handle_t ptr, const char* command) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  eci_rep->eci->command(command);
}

/** 
 * A specialized version of 'eci_command()' taking a double value
 * as the 2nd argument.
 */
void eci_command_float_arg(const char* command, double arg) {
  eci_rep.eci->command_float_arg(command, arg);
}

/** 
 * A specialized version of 'eci_command()' taking a double value
 * as the 2nd argument.
 */
void eci_command_float_arg_r(eci_handle_t ptr, const char* command, double arg) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  eci_rep->eci->command_float_arg(command, arg);
}

/* ---------------------------------------------------------------------
 * Getting return values 
 */

/**
 * Returns the number of strings returned by the 
 * last ECI command.
 */
int eci_last_string_list_count(void) {
  return(eci_rep.eci->last_string_list().size());
}

/**
 * Returns the number of strings returned by the 
 * last ECI command.
 */
int eci_last_string_list_count_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_string_list().size());
}

/**
 * Returns the nth item of the list containing 
 * strings returned by the last ECI command.
 *
 * require:
 *  n >= 0 && n < eci_last_string_list_count()
 */
const char* eci_last_string_list_item(int n) {
  return(eci_rep.eci->last_string_list()[n].c_str());
}

/**
 * Returns the nth item of the list containing 
 * strings returned by the last ECI command.
 *
 * require:
 *  n >= 0 && n < eci_last_string_list_count()
 */
const char* eci_last_string_list_item_r(eci_handle_t ptr, int n) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_string_list()[n].c_str());
}

const char* eci_last_string(void) {
  return(eci_rep.eci->last_string().c_str());
}

const char* eci_last_string_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_string().c_str());
}

double eci_last_float(void) {
  return(eci_rep.eci->last_float());
}

double eci_last_float_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_float());
}

int eci_last_integer(void) {
  return(eci_rep.eci->last_integer());
}

int eci_last_integer_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_integer());
}

long int eci_last_long_integer(void) {
  return(eci_rep.eci->last_long_integer());
}

long int eci_last_long_integer_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_long_integer());
}

/**
 * Returns pointer to a null-terminated string containing 
 * information about the last occured error.
 */
const char* eci_last_error(void) {
  return(eci_rep.eci->last_error().c_str());
}

/**
 * Returns pointer to a null-terminated string containing 
 * information about the last occured error.
 */
const char* eci_last_error_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_error().c_str());
}


const char* eci_last_type(void) {
  return(eci_rep.eci->last_type().c_str());
}

const char* eci_last_type_r(eci_handle_t ptr) {
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  return(eci_rep->eci->last_type().c_str());
}

/**
 * Whether an error has occured?
 */
int eci_error(void) { 
  if (eci_rep.eci->error()) return(1);
  return(0);
}

/**
 * Whether an error has occured?
 */
int eci_error_r(eci_handle_t ptr) { 
  eci_internal_t* eci_rep = (eci_internal_t*)ptr;
  if (eci_rep->eci->error()) return(1);
  return(0);
}
 
/* --------------------------------------------------------------------- 
 * Events 
 */

int eci_events_available(void) { return(0); }
int eci_events_available_r(eci_handle_t ptr) { return(0); }
void eci_next_event(void) { }
void eci_next_event_r(eci_handle_t ptr) { }
const char* eci_current_event(void) { return(0); }
const char* eci_current_event_r(eci_handle_t ptr) { return(0); }
