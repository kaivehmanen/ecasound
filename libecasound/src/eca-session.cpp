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

#include <config.h>

#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <pthread.h>

#include <kvutils.h>

#include "eca-resources.h"

#include "eca-chain.h"
#include "audiofx.h"
#include "audioio.h"
#include "audioio-mp3.h"
#include "audioio-mikmod.h"

#include "osc-gen.h"

#include "eca-error.h"
#include "eca-debug.h"

#include "eca-comhelp.h"
#include "eca-session.h"
#include "eca-chainsetup.h"

ECA_SESSION::ECA_SESSION(void) {
  //  pthread_mutex_init(&status_lock, NULL);
  ecaresources.load();
  set_defaults();
  set_scheduling();
}

ECA_SESSION::~ECA_SESSION(void) {
  ecadebug->msg(1,"ECA_SESSION destructor!");

  status(ep_status_notready);

  ecaresources.save();

  for(vector<ECA_CHAINSETUP*>::iterator q = chainsetups.begin(); q != chainsetups.end(); q++) {
    delete *q;
  }

  ecadebug->control_flow("Closing session");
}

ECA_SESSION::ECA_SESSION(COMMAND_LINE& cline) throw(ECA_ERROR*) {
  //  pthread_mutex_init(&status_lock, NULL);
  ecaresources.load();
  set_defaults();

  interpret_general_options(cline);

  if (chainsetups.size() == 0) {
    ECA_CHAINSETUP* comline_setup = new ECA_CHAINSETUP(&ecaresources, cline);
    try {
      //      select_chainsetup(comline_setup->name());
      add_chainsetup(comline_setup);
      if (selected_chainsetup->is_valid()) connect_chainsetup();
    }
    catch (ECA_ERROR* e) {
      if (iactive) {
	if (e->error_action() != ECA_ERROR::retry) throw;
      }
      else
	throw;
    }
  }

  set_scheduling();

  if (iactive == false) {
    // These were added for convenience...
    if (cline.size() < 2) {
      // No parameters, let's give some help.
      interpret_general_option("-h");
    }
    if (!is_selected_chainsetup_connected()) {
      // Still no inputs? If not in interactive mode, there really isn't
      // anything left to do.
      throw(new ECA_ERROR("ECA_SESSION","Nothing to do!"));
    }
  }
}

void ECA_SESSION::set_defaults(void) {
  status(ep_status_notready);
  connected_chainsetup = 0;
  selected_chainsetup = 0;

  // ---
  // Interpret resources 

  raisepriority_rep = ecaresources.boolean_resource("default-to-raisepriority");
  if (ecaresources.resource("default-to-interactive-mode") == "true") 
    iactive = true;
  else
    iactive = false;

  GENERIC_OSCILLATOR::set_preset_file(ecaresources.resource("resource-directory") + "/" + ecaresources.resource("resource-file-genosc-envelopes"));

  MP3FILE::set_mpg123_path(ecaresources.resource("ext-mpg123-path"));
  MP3FILE::set_mpg123_args(ecaresources.resource("ext-mpg123-args"));

  MP3FILE::set_lame_path(ecaresources.resource("ext-lame-path"));
  MP3FILE::set_lame_args(ecaresources.resource("ext-lame-args"));

  MIKMOD_INTERFACE::set_mikmod_path(ecaresources.resource("ext-mikmod-path"));
  MIKMOD_INTERFACE::set_mikmod_args(ecaresources.resource("ext-mikmod-args"));

  multitrack_mode = false;
}

void ECA_SESSION::set_scheduling(void) {
  if (raisepriority_rep == true) {
    struct sched_param sparam;
    sparam.sched_priority = 10;

    if (sched_setscheduler(0, SCHED_FIFO, &sparam) == -1) 
      ecadebug->msg("(eca-session) Unable to change scheduling policy!");
    else 
      ecadebug->msg("(eca-session) Using realtime-scheduling (SCHED_FIFO/10).");
  }
}

void ECA_SESSION::add_chainsetup(const string& name) {
  // --------
  // require:
  assert(name != "");
  // --------

  ECA_CHAINSETUP* newsetup = new ECA_CHAINSETUP (&ecaresources, name,
						  false);
  add_chainsetup(newsetup);

  // --------
  // ensure:
  assert(selected_chainsetup->name() == name);
  // --------
}

void ECA_SESSION::add_chainsetup(ECA_CHAINSETUP* comline_setup) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(comline_setup != 0);
  // --------
  int old_size = chainsetups.size();

  vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups.begin();
  while(p != chainsetups.end()) {
    if ((*p)->name() == comline_setup->name()) {
      delete comline_setup;
      throw(new ECA_ERROR("ECA-SESSION","Chainsetup \"" + (*p)->name() + 
			  "\" already exists.", ECA_ERROR::retry));
    }
    ++p;
  }

  selected_chainsetup = comline_setup;
  chainsetups.push_back(comline_setup);

  // --------
  // ensure:
  assert(selected_chainsetup == comline_setup);
  assert(chainsetups.size() == old_size + 1);
  // --------
}

void ECA_SESSION::remove_chainsetup(void) {
  // --------
  // require:
  assert(connected_chainsetup != selected_chainsetup);
  // --------

  vector<ECA_CHAINSETUP*>::iterator p = chainsetups.begin();
  while(p != chainsetups.end()) {
    if (*p == selected_chainsetup) {
      selected_chainsetup = 0;
      delete *p;
      chainsetups.erase(p);
      break;
    }
    ++p;
  }

  // --------
  // ensure:
  assert(selected_chainsetup == 0);
  // --------
}

void ECA_SESSION::select_chainsetup(const string& name) {
  // --------
  // require:
  assert(name.empty() != true);
  // --------

  selected_chainsetup = 0;
  vector<ECA_CHAINSETUP*>::const_iterator p = chainsetups.begin();
  while(p != chainsetups.end()) {
    if ((*p)->name() == name) {
      ecadebug->msg(1, "(eca-session) Chainsetup \"" + name + "\" selected.");
      selected_chainsetup = *p;
      break;
    }
    ++p;
  }

  // --------
  // ensure:
  assert(selected_chainsetup->name() == name ||
	 selected_chainsetup == 0);
  // --------
}

void ECA_SESSION::save_chainsetup(void) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(selected_chainsetup != 0);
  // --------

  selected_chainsetup->save();
}

void ECA_SESSION::save_chainsetup(const string& filename) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(selected_chainsetup != 0 && filename.empty() != true);
  // --------

  selected_chainsetup->save_to_file(filename);
}

void ECA_SESSION::load_chainsetup(const string& filename) throw(ECA_ERROR*) {
  // --------
  // require:
  assert(filename.empty() != true);
  // --------

  ECA_CHAINSETUP* new_setup = new ECA_CHAINSETUP(&ecaresources,
						 filename, 
						 true);
  add_chainsetup(new_setup);

  selected_chainsetup = new_setup;

  // --------
  // ensure:
  assert(selected_chainsetup->filename() == filename);
  // --------
}

void ECA_SESSION::connect_chainsetup(void) {
  // --------
  // require:
  assert(selected_chainsetup != 0);
  assert(selected_chainsetup->is_valid());
  // --------

  if (selected_chainsetup == connected_chainsetup) return;

  if (connected_chainsetup != 0) {
    disconnect_chainsetup();
  }

  connected_chainsetup = selected_chainsetup;
  connected_chainsetup->enable();

  ecadebug->msg(1, "Connecting connected chainsetup to engine.");
 
  while(inslots.size() != 0) inslots.pop_back();
  while(inslots.size() != connected_chainsetup->inputs.size())
    inslots.push_back(SAMPLE_BUFFER(connected_chainsetup->buffersize(), SAMPLE_SPECS::channel_count_default));

  // --------
  // ensure:
  assert(selected_chainsetup == connected_chainsetup);
  // --------
}

void ECA_SESSION::disconnect_chainsetup(void) {
  // --------
  // require:
  assert(connected_chainsetup != 0);
  // --------

  ecadebug->msg(1, "Disconnecting selected setup from engine.");

  connected_chainsetup->disable();
  connected_chainsetup = 0;

  // --------
  // ensure:
  assert(connected_chainsetup == 0);
  // --------
}

void ECA_SESSION::interpret_general_options(COMMAND_LINE& cline) {
  cline.back_to_start();
  while(cline.ready()) {
    string temp = cline.next_argument();
    interpret_general_option(temp);
  }

 cline.back_to_start();    
 while(cline.ready()) {
   string argu = cline.next_argument();
   string argu_param = cline.next();
   if (argu_param.size() > 0) {
     if (argu_param[0] == '-') {
       cline.previous();
       argu_param == "";
     }
   }

    interpret_chainsetup(argu, argu_param);
  }
}

void ECA_SESSION::interpret_general_option (const string& argu) {
  if (argu.size() < 2) return;
  if (argu[0] != '-') return;
  switch(argu[1]) {
  case 'c':
    iactive = true;
    ecadebug->msg(0, "(eca-session) Interactive mode enabled."); 
    break;

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

  case 'r':
    {
      ecadebug->msg("(eca-session) Raised-priority mode enabled.");
      raisepriority_rep = true;
      break;
    }

  default: { }
  }
}

void ECA_SESSION::interpret_chainsetup (const string& argu,
					const string& toinen) {
  if (argu.size() == 0) return;
  
  string tname = get_argument_number(1, argu);
  if (tname == "") tname = toinen;
  else if (tname[0] == '-') tname = toinen;
  //  else tname = argu;

  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 's':
    if (argu.size() > 2 && argu[2] == ':') {
      load_chainsetup(tname);
      if (selected_chainsetup->is_valid()) connect_chainsetup();
    }
    else if (argu.size() == 2) {
      load_chainsetup(toinen);
      if (selected_chainsetup->is_valid()) connect_chainsetup();
    }
    break;
  }
}

bool ECA_SESSION::is_slave_output(AUDIO_IO* aiod) const {
  // --------
  // require:
  assert(connected_chainsetup != 0);
  // --------

  if (aiod->is_realtime()) return(false);
  vector<CHAIN*>::iterator q = connected_chainsetup->chains.begin();
  while(q != connected_chainsetup->chains.end()) {
    if ((*q)->output_id == aiod) {
      if ((*q)->input_id->is_realtime()) {
	ecadebug->msg(2,"(eca-session) slave output detected: " + (*q)->output_id->label());
	return(true);
      }
    }
    ++q;
  }
  return(false);
}

void ECA_SESSION::status(EP_STATUS temp) { ep_status = temp; }
EP_STATUS ECA_SESSION::status(void) const { return(ep_status); }
