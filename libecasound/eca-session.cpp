// ------------------------------------------------------------------------
// eca-session.cpp: Ecasound runtime setup and parameters.
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <pthread.h>
#include <sys/mman.h>

#include <kvutils/com_line.h>
#include <kvutils/message_item.h>

#include "eca-resources.h"
#include "eca-version.h"

#include "eca-chain.h"
#include "audiofx.h"
#include "audioio.h"
#include "audioio-mp3.h"
#include "audioio-mikmod.h"
#include "audioio-timidity.h"
#include "audioio-ogg.h"

#include "osc-gen.h"

#include "eca-error.h"
#include "eca-debug.h"

#include "eca-comhelp.h"
#include "eca-session.h"
#include "eca-chainsetup.h"

ECA_SESSION::ECA_SESSION(void) {
  set_defaults();
}

ECA_SESSION::~ECA_SESSION(void) {
//    ecadebug->msg(ECA_DEBUG::system_objects,"ECA_SESSION destructor!");

  status(ep_status_notready);

  for(vector<ECA_CHAINSETUP*>::iterator q = chainsetups_rep.begin(); q != chainsetups_rep.end(); q++) {
    delete *q;
  }

  delete ecasound_stop_cond_repp;
  delete ecasound_stop_mutex_repp;

//    ecadebug->control_flow("Closing session");
}

ECA_SESSION::ECA_SESSION(COMMAND_LINE& cline) throw(ECA_ERROR&) {
  set_defaults();

  cline.combine();
  interpret_general_options(cline);

  vector<string> options;
  create_chainsetup_options(cline, &options);

  if (chainsetups_rep.size() == 0) {
    ECA_CHAINSETUP* comline_setup = new ECA_CHAINSETUP(options);
    try {
      //      select_chainsetup(comline_setup->name());
      add_chainsetup(comline_setup);
      if (selected_chainsetup_repp->is_valid()) connect_chainsetup();
    }
    catch (ECA_ERROR& e) {
      if (iactive_rep) {
	if (e.error_action() != ECA_ERROR::retry) throw;
      }
      else
	throw;
    }
  }
}

void ECA_SESSION::set_defaults(void) {
  status(ep_status_notready);
  connected_chainsetup_repp = 0;
  selected_chainsetup_repp = 0;

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

  GENERIC_OSCILLATOR::set_preset_file(ecaresources.resource("resource-directory") + "/" + ecaresources.resource("resource-file-genosc-envelopes"));

  MP3FILE::set_mp3_input_cmd(ecaresources.resource("ext-mp3-input-cmd"));
  MP3FILE::set_mp3_output_cmd(ecaresources.resource("ext-mp3-output-cmd"));
  MIKMOD_INTERFACE::set_mikmod_cmd(ecaresources.resource("ext-mikmod-cmd"));
  TIMIDITY_INTERFACE::set_timidity_cmd(ecaresources.resource("ext-timidity-cmd"));
  OGG_VORBIS_INTERFACE::set_ogg_input_cmd(ecaresources.resource("ext-ogg-input-cmd"));
  OGG_VORBIS_INTERFACE::set_ogg_output_cmd(ecaresources.resource("ext-ogg-output-cmd"));

  multitrack_mode_rep = false;
}

void ECA_SESSION::add_chainsetup(const string& name) {
  // --------
  // require:
  assert(name != "");
  // --------

  ECA_CHAINSETUP* newsetup = new ECA_CHAINSETUP;
  newsetup->set_name(name);
  add_chainsetup(newsetup);

  // --------
  // ensure:
  assert(selected_chainsetup_repp->name() == name);
  // --------
}

void ECA_SESSION::add_chainsetup(ECA_CHAINSETUP* comline_setup) throw(ECA_ERROR&) {
  // --------
  // require:
  assert(comline_setup != 0);
  // --------
  int old_size = chainsetups_rep.size();

  vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups_rep.begin();
  while(p != chainsetups_rep.end()) {
    if ((*p)->name() == comline_setup->name()) {
      delete comline_setup;
      throw(ECA_ERROR("ECA-SESSION","Chainsetup \"" + (*p)->name() + 
			  "\" already exists.", ECA_ERROR::retry));
    }
    ++p;
  }

  selected_chainsetup_repp = comline_setup;
  chainsetups_rep.push_back(comline_setup);

  // --------
  // ensure:
  assert(selected_chainsetup_repp == comline_setup);
  assert(static_cast<int>(chainsetups_rep.size()) == old_size + 1);
  // --------
}

void ECA_SESSION::remove_chainsetup(void) {
  // --------
  // require:
  assert(connected_chainsetup_repp != selected_chainsetup_repp);
  // --------

  vector<ECA_CHAINSETUP*>::iterator p = chainsetups_rep.begin();
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
  assert(selected_chainsetup_repp == 0);
  // --------
}

void ECA_SESSION::select_chainsetup(const string& name) {
  // --------
  // require:
  assert(name.empty() != true);
  // --------

  selected_chainsetup_repp = 0;
  vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups_rep.begin();
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
  assert(selected_chainsetup_repp->name() == name ||
	 selected_chainsetup_repp == 0);
  // --------
}

void ECA_SESSION::save_chainsetup(void) throw(ECA_ERROR&) {
  // --------
  // require:
  assert(selected_chainsetup_repp != 0);
  // --------

  selected_chainsetup_repp->save();
}

void ECA_SESSION::save_chainsetup(const string& filename) throw(ECA_ERROR&) {
  // --------
  // require:
  assert(selected_chainsetup_repp != 0 && filename.empty() != true);
  // --------

  selected_chainsetup_repp->save_to_file(filename);
}

void ECA_SESSION::load_chainsetup(const string& filename) throw(ECA_ERROR&) {
  // --------
  // require:
  assert(filename.empty() != true);
  // --------

  ECA_CHAINSETUP* new_setup = new ECA_CHAINSETUP(filename);
  add_chainsetup(new_setup);

  selected_chainsetup_repp = new_setup;

  // --------
  // ensure:
  assert(selected_chainsetup_repp->filename() == filename);
  // --------
}

void ECA_SESSION::connect_chainsetup(void) throw(ECA_ERROR&) {
  // --------
  // require:
  assert(selected_chainsetup_repp != 0);
  assert(selected_chainsetup_repp->is_valid());
  // --------

  if (selected_chainsetup_repp == connected_chainsetup_repp) return;

  if (connected_chainsetup_repp != 0) {
    disconnect_chainsetup();
  }

  connected_chainsetup_repp = selected_chainsetup_repp;
  connected_chainsetup_repp->enable();

  ecadebug->msg(ECA_DEBUG::system_objects, "Connecting connected chainsetup to engine.");
 
  // --------
  // ensure:
  assert(selected_chainsetup_repp == connected_chainsetup_repp);
  // --------
}

void ECA_SESSION::disconnect_chainsetup(void) {
  // --------
  // require:
  assert(connected_chainsetup_repp != 0);
  // --------

  ecadebug->msg(ECA_DEBUG::system_objects, "Disconnecting selected setup from engine.");

  connected_chainsetup_repp->disable();
  connected_chainsetup_repp = 0;

  // --------
  // ensure:
  assert(connected_chainsetup_repp == 0);
  // --------
}

vector<string> ECA_SESSION::chainsetup_names(void) const {
  vector<string> result;
  vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups_rep.begin();
  while(p != chainsetups_rep.end()) {
    result.push_back((*p)->name());
    ++p;
  }
  return(result);
}

void ECA_SESSION::create_chainsetup_options(COMMAND_LINE& cline,
					    vector<string>* options) {
  cline.begin();
  cline.next(); // skip the program name
  while(cline.end() == false) {
    if (is_session_option(cline.current()) != true)
      options->push_back(cline.current());
    cline.next();
  }
}

/**
 * Tests whether the given argument is a session-level option.
 */
bool ECA_SESSION::is_session_option(const string& arg) const {
  if (arg.size() < 2 ||
      arg[0] != '-') return(false);

  switch(arg[1]) {
  case 'c':
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

void ECA_SESSION::interpret_general_options(COMMAND_LINE& cline) {
  cline.begin();
  while(cline.end() == false) {
    if (cline.current().size() > 0 && cline.current()[0] == '-')
      interpret_general_option(cline.current());
    cline.next();
  }

  cline.begin();
  while(cline.end() == false) {
    if (cline.current().size() > 0 && cline.current()[0] == '-')
      interpret_chainsetup(cline.current());
    cline.next();
  }
}

void ECA_SESSION::interpret_general_option (const string& argu) {
  if (argu.size() < 2) return;
  if (argu[0] != '-') return;
  switch(argu[1]) {
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
    cout << ecasound_parameter_help();
    break;

  case 'q':
    ecadebug->disable();
    break;

  case 'r':
    {
      int prio = ::atoi(get_argument_number(1, argu).c_str());
      if (prio != 0) 
	schedpriority_rep = prio;
      ecadebug->msg("(eca-session) Raised-priority mode enabled. Locking memory. (prio:" + 
		    kvu_numtostr(schedpriority_rep) + ")");
      raisepriority_rep = true;
      if (::mlockall (MCL_CURRENT|MCL_FUTURE)) {
	ecadebug->msg("(eca-session) Warning! Couldn't lock all memory!");
      }
      else 
	ecadebug->msg(ECA_DEBUG::system_objects, "(eca-session) Memory locked!");
      break;
    }
    
  default: { }
  }
}

void ECA_SESSION::interpret_chainsetup (const string& argu) {
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
