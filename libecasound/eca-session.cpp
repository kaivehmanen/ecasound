// ------------------------------------------------------------------------
// eca-session.cpp: Ecasound runtime setup and parameters.
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <iostream>

#include <pthread.h>
#include <sys/mman.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kvutils/com_line.h>
#include <kvutils/message_item.h>
#include <kvutils/dbc.h>

#include "eca-resources.h"
#include "eca-version.h"

#include "eca-chain.h"
#include "audiofx.h"
#include "audioio.h"
#include "audioio-mp3.h"
#include "audioio-mikmod.h"
#include "audioio-timidity.h"
#include "audioio-ogg.h"

#include "osc-gen-file.h"

#include "eca-error.h"
#include "eca-debug.h"

#include "eca-static-object-maps.h"
#include "eca-comhelp.h"
#include "eca-session.h"
#include "eca-chainsetup.h"

#ifdef USE_CXX_STD_NAMESPACE
using namespace std;
#endif

ECA_SESSION::ECA_SESSION(void) {
  set_defaults();
}

ECA_SESSION::~ECA_SESSION(void) {
//    ecadebug->msg(ECA_DEBUG::system_objects,"ECA_SESSION destructor!");

  status(ep_status_notready);

  for(std::vector<ECA_CHAINSETUP*>::iterator q = chainsetups_rep.begin(); q != chainsetups_rep.end(); q++) {
    delete *q;
  }

  delete ecasound_stop_cond_repp;
  delete ecasound_stop_mutex_repp;
  
  /* delete static object maps */
  unregister_default_objects();

//    ecadebug->control_flow("Closing session");
}

ECA_SESSION::ECA_SESSION(COMMAND_LINE& cline) throw(ECA_ERROR&) {
  set_defaults();

  cline.combine();

  std::vector<std::string> options,csoptions;
  create_chainsetup_options(cline, &options);
  preprocess_options(&options);
  interpret_general_options(options,&csoptions);

  if (chainsetups_rep.size() == 0) {
    ECA_CHAINSETUP* comline_setup = new ECA_CHAINSETUP(csoptions);
    if (comline_setup->interpret_result() != true) {
      string temp = comline_setup->interpret_result_verbose();
      delete comline_setup;
      throw(ECA_ERROR("ECA-SESSION", temp));
    }
    else {
      add_chainsetup(comline_setup); /* ownership object transfered */
      if (selected_chainsetup_repp->is_valid()) connect_chainsetup();
    }
  }
}

void ECA_SESSION::set_defaults(void) {
  /* create static object maps */
  register_default_objects();

  status(ep_status_notready);
  connected_chainsetup_repp = 0;
  selected_chainsetup_repp = 0;
  active_chain_index_rep = 0;
  active_chainop_index_rep = 0;
  active_chainop_param_index_rep = 0;

  // --
  // Engine locks and mutexes

  ecasound_stop_cond_repp = new pthread_cond_t;
  ecasound_stop_mutex_repp = new pthread_mutex_t;

  ::pthread_cond_init(ecasound_stop_cond_repp, NULL);
  ::pthread_mutex_init(ecasound_stop_mutex_repp, NULL);

  // ---
  // Interpret resources 

  raisepriority_rep = ecaresources.boolean_resource("default-to-raisepriority");
  if (ecaresources.resource("default-to-interactive-mode") == "true") 
    iactive_rep = true;
  else
    iactive_rep = false;
  schedpriority_rep = atoi(ecaresources.resource("default-schedpriority").c_str());

  MP3FILE::set_mp3_input_cmd(ecaresources.resource("ext-mp3-input-cmd"));
  MP3FILE::set_mp3_output_cmd(ecaresources.resource("ext-mp3-output-cmd"));
  MIKMOD_INTERFACE::set_mikmod_cmd(ecaresources.resource("ext-mikmod-cmd"));
  TIMIDITY_INTERFACE::set_timidity_cmd(ecaresources.resource("ext-timidity-cmd"));
  OGG_VORBIS_INTERFACE::set_ogg_input_cmd(ecaresources.resource("ext-ogg-input-cmd"));
  OGG_VORBIS_INTERFACE::set_ogg_output_cmd(ecaresources.resource("ext-ogg-output-cmd"));

  multitrack_mode_rep = false;
}

/**
 * Add a new chainsetup
 *
 * require:
 *  name.empty() != true
 *
 * ensure:
 *  selected_chainsetup->name() == name ||
 *  chainsetup_names().size() has not changed
 */
void ECA_SESSION::add_chainsetup(const std::string& name) {
  // --------
  // require:
  DBC_REQUIRE(name != "");
  // --------

  ECA_CHAINSETUP* newsetup = new ECA_CHAINSETUP;
  newsetup->set_name(name);
  add_chainsetup(newsetup);

  // --------
  // ensure:
  DBC_ENSURE(selected_chainsetup_repp->name() == name);
  // --------
}

/**
 * Add a new chainsetup. Ownership of the object given as argument
 * is passed along the call. If a chainsetup with the same name already
 * exists, the call will fail and the chainsetup given as argument
 * is deleted.
 *
 * require:
 *  comline_setup != 0
 *
 * ensure:
 *  selected_chainsetup == comline_setup ||
 *  comline_setup == 0
 */
void ECA_SESSION::add_chainsetup(ECA_CHAINSETUP* comline_setup) {
  // --------
  // require:
  DBC_REQUIRE(comline_setup != 0);
  // --------
  int old_size = chainsetups_rep.size();

  std::vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups_rep.begin();
  while(p != chainsetups_rep.end()) {
    if ((*p)->name() == comline_setup->name()) {
      delete comline_setup;
      comline_setup = 0;
      ecadebug->msg(ECA_DEBUG::system_objects, 
		    string("ECA-SESSION","Unable to add chainsetup \"") + 
		    (*p)->name() + 
		    "\"; chainsetup with the same name already exists.");
    }
    ++p;
  }

  selected_chainsetup_repp = comline_setup;
  chainsetups_rep.push_back(comline_setup);

  // --------
  // ensure:
  DBC_ENSURE(selected_chainsetup_repp == comline_setup);
  DBC_ENSURE(static_cast<int>(chainsetups_rep.size()) == old_size + 1);
  // --------
}

/**
 * Removes selected chainsetup
 *
 * require:
 *  connected_chainsetup != selected_chainsetup
 *
 * ensure:
 *  selected_chainsetup == 0
 */
void ECA_SESSION::remove_chainsetup(void) {
  // --------
  // require:
  DBC_REQUIRE(connected_chainsetup_repp != selected_chainsetup_repp);
  // --------

  std::vector<ECA_CHAINSETUP*>::iterator p = chainsetups_rep.begin();
  while(p != chainsetups_rep.end()) {
    if (*p == selected_chainsetup_repp) {
      selected_chainsetup_repp = 0;
      delete *p;
      chainsetups_rep.erase(p);
      break;
    }
    ++p;
  }

  // --------
  // ensure:
  DBC_ENSURE(selected_chainsetup_repp == 0);
  // --------
}

void ECA_SESSION::select_chainsetup(const std::string& name) {
  // --------
  // require:
  DBC_REQUIRE(name.empty() != true);
  // --------

  selected_chainsetup_repp = 0;
  std::vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups_rep.begin();
  while(p != chainsetups_rep.end()) {
    if ((*p)->name() == name) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-session) Chainsetup \"" + name + "\" selected.");
      selected_chainsetup_repp = *p;
      break;
    }
    ++p;
  }

  // --------
  // ensure:
  DBC_ENSURE(selected_chainsetup_repp == 0 ||
	 selected_chainsetup_repp->name() == name);
  // --------
}

void ECA_SESSION::save_chainsetup(void) throw(ECA_ERROR&) {
  // --------
  // require:
  DBC_REQUIRE(selected_chainsetup_repp != 0);
  // --------

  selected_chainsetup_repp->save();
}

void ECA_SESSION::save_chainsetup(const std::string& filename) throw(ECA_ERROR&) {
  // --------
  // require:
  DBC_REQUIRE(selected_chainsetup_repp != 0 && filename.empty() != true);
  // --------

  selected_chainsetup_repp->save_to_file(filename);
}

/**
 * Load a chainsetup from file (ecs). If operation fails,
 * selected_chainsetup_repp == 0, ie. no chainsetup 
 * selected.
 */
void ECA_SESSION::load_chainsetup(const std::string& filename) {
  // --------
  DBC_REQUIRE(filename.empty() != true);
  // --------

  ECA_CHAINSETUP* new_setup = new ECA_CHAINSETUP(filename);
  add_chainsetup(new_setup);
  if (new_setup != 0) /* adding chainsetup ok */
    selected_chainsetup_repp = new_setup;
  else
    selected_chainsetup_repp = 0;
  
  // --------
  // ensure:
  DBC_ENSURE(selected_chainsetup_repp->filename() == filename);
  // --------
}

void ECA_SESSION::connect_chainsetup(void) throw(ECA_ERROR&) {
  // --------
  // require:
  DBC_REQUIRE(selected_chainsetup_repp != 0);
  DBC_REQUIRE(selected_chainsetup_repp->is_valid());
  // --------

  if (selected_chainsetup_repp == connected_chainsetup_repp) return;

  if (connected_chainsetup_repp != 0) {
    disconnect_chainsetup();
  }

  /** 
   * enable() throws an exception if it wasn't possibly
   * to open/activate all input and output objects 
   */
  selected_chainsetup_repp->enable();
  connected_chainsetup_repp = selected_chainsetup_repp;

  ecadebug->msg(ECA_DEBUG::system_objects, "Connecting connected chainsetup to engine.");
 
  // --------
  // ensure:
  DBC_ENSURE(selected_chainsetup_repp == connected_chainsetup_repp);
  // --------
}

void ECA_SESSION::disconnect_chainsetup(void) {
  // --------
  // require:
  DBC_REQUIRE(connected_chainsetup_repp != 0);
  // --------

  ecadebug->msg(ECA_DEBUG::system_objects, "Disconnecting selected setup from engine.");

  connected_chainsetup_repp->disable();
  connected_chainsetup_repp = 0;

  // --------
  // ensure:
  DBC_ENSURE(connected_chainsetup_repp == 0);
  // --------
}

std::vector<std::string> ECA_SESSION::chainsetup_names(void) const {
  std::vector<std::string> result;
  std::vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups_rep.begin();
  while(p != chainsetups_rep.end()) {
    result.push_back((*p)->name());
    ++p;
  }
  return(result);
}

void ECA_SESSION::create_chainsetup_options(COMMAND_LINE& cline,
					    std::vector<std::string>* options) {
  cline.begin();
  cline.next(); // skip the program name
  while(cline.end() == false) {
    options->push_back(cline.current());
    cline.next();
  }
}

/**
 * Tests whether the given argument is a session-level option.
 */
bool ECA_SESSION::is_session_option(const std::string& arg) const {
  if (arg.size() < 2 ||
      arg[0] != '-') return(false);

  switch(arg[1]) {
  case 'c':
  case 'C':
  case 'D':
  case 'd':
  case 'h':
  case 'q':
  case 'r':
    return(true);

  case 's': 
    if (arg.size() > 2 && arg[2] == ':') return(true);
  }
  return(false);
}

/**
 * Preprocesses a set of options.
 * 
 * ensure:
 *  all options valid for further processing (all options
 *  must start with a '-' sign)
 */
void ECA_SESSION::preprocess_options(std::vector<std::string>* opts) {
  std::vector<std::string>::iterator p = opts->begin();
  while(p != opts->end()) {

    if (p->size() > 0 && (*p)[0] != '-') {
      /* hack1: options ending with .ecs as "-s:file.ecs" */
      string::size_type pos = p->find(".ecs");
      if (pos + 4 == p->size())
	*p = "-s:" + *p;
      
      /* hack2: rest as "-i:file.ecs" */
      else
	*p = "-i:" + *p;
    }
    ++p;
  }
}

/**
 * Interprets all session specific options from 'inopts'.
 * All unprocessed options are copied to 'outopts'. 
 */
void ECA_SESSION::interpret_general_options(const std::vector<std::string>& inopts,
					    std::vector<std::string>* outopts) {
  std::vector<std::string>::const_iterator p = inopts.begin();
  while(p != inopts.end()) {
    if (p->size() > 0 && (*p)[0] == '-')
      interpret_general_option(*p);
    ++p;
  }

  p = inopts.begin();
  while(p != inopts.end()) {
    if (p->size() > 0 && (*p)[0] == '-')
      interpret_chainsetup_option(*p);

    if (is_session_option(*p) != true) outopts->push_back(*p);

    ++p;
  }
}

void ECA_SESSION::interpret_general_option (const std::string& argu) {
  if (argu.size() < 2) return;
  if (argu[0] != '-') return;
  switch(argu[1]) {
  case 'C':
    {
      iactive_rep = false;
      ecadebug->msg("(eca-session) Interactive mode disabled."); 
      break;
    }

  case 'c':
    {
      iactive_rep = true;
      ecadebug->msg("(eca-session) Interactive mode enabled."); 
      break;
    }

  case 'd':
    {
      ecadebug->set_debug_level(atoi(get_argument_number(1, argu).c_str()));
      MESSAGE_ITEM mtempd;
      mtempd << "(eca-session) Set debug level to: " << ecadebug->get_debug_level();
      ecadebug->msg(mtempd.to_string());
      break;
    }
  case 'h':      // help!
    std::cout << ecasound_parameter_help();
    break;

  case 'q':
    ecadebug->disable();
    break;

  case 'r':
    {
      int prio = ::atoi(get_argument_number(1, argu).c_str());
      if (prio < 0) {
	ecadebug->msg("(eca-session) Raised-priority mode disabled.");
	raisepriority_rep = false;
      }
      else {
	if (prio != 0) 
	  schedpriority_rep = prio;
	ecadebug->msg("(eca-session) Raised-priority mode enabled. (prio:" + 
		      kvu_numtostr(schedpriority_rep) + ")");
	raisepriority_rep = true;
#ifdef HAVE_MLOCKALL
	if (::mlockall (MCL_CURRENT|MCL_FUTURE)) {
	  ecadebug->msg("(eca-session) Warning! Couldn't lock all memory!");
	}
	else 
	  ecadebug->msg("(eca-session) Memory locked!");
#else
	ecadebug->msg("(eca-session) Memory locking not available.");
#endif
      }
      break;
    }
    
  default: { }
  }
}

void ECA_SESSION::interpret_chainsetup_option (const std::string& argu) {
  if (argu.size() == 0) return;
  
  string tname = get_argument_number(1, argu);
 
  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 's': {
    if (argu.size() > 2 && argu[2] == ':') {
      load_chainsetup(tname);
      if (selected_chainsetup_repp->is_valid()) connect_chainsetup();
    }
    break;
  }
  default: { }
  }
}

void ECA_SESSION::status(ECA_SESSION::Engine_status temp) { ep_status_rep = temp; }
ECA_SESSION::Engine_status ECA_SESSION::status(void) const { return(ep_status_rep); }
