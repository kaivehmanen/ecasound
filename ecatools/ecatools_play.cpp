// ------------------------------------------------------------------------
// ecatools-play.cpp: A simple command-line tool for playing audio files
//                    using the default output device specified in 
//                    "~/.ecasoundrc".
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <string>
#include <cstdio>
#include <signal.h>

#include <kvutils/com_line.h>
#include <kvutils/procedure_timer.h>

#include <eca-debug.h>
#include <eca-error.h>
#include <eca-control.h>
#include <eca-main.h>
#include <eca-session.h>
#include <eca-version.h>

#include "ecatools_play.h"

static ECA_CONTROL* ectrl_repp = 0;
static PROCEDURE_TIMER* ptimer_repp = 0;
static struct timeval* lowerlimit_repp;
static int exit_request_rep = 0;

int main(int argc, char *argv[])
{
#ifdef NDEBUG
  ecadebug->disable();
#else
  ecadebug->set_debug_level(ECA_DEBUG::info |
    			    ECA_DEBUG::module_flow |
    			    ECA_DEBUG::user_objects);
#endif

  struct sigaction es_handler_int;
  es_handler_int.sa_handler = signal_handler;
  sigemptyset(&es_handler_int.sa_mask);
  es_handler_int.sa_flags = 0;
  sigaction(SIGINT, &es_handler_int, 0);

  COMMAND_LINE cline = COMMAND_LINE (argc, argv);

  if (cline.size() < 2) {
    print_usage();
    return(1);
  }
  
  PROCEDURE_TIMER ptimer;
  ptimer_repp = &ptimer;

  struct timeval lowerlimit;
  lowerlimit_repp = &lowerlimit;
  lowerlimit.tv_sec = 1;
  lowerlimit.tv_usec = 0;

  try {
    string filename;

    ECA_SESSION esession;
    ECA_CONTROL ectrl (&esession);
    ectrl_repp = &ectrl;
    ECA_AUDIO_FORMAT aio_params;

    ectrl.toggle_interactive_mode(true);
    cline.begin();
    cline.next(); // skip the program name
    while(cline.end() == false &&
	  exit_request_rep == 0) {
      filename = cline.current();

      cerr << "Playing file \"" << filename << "\".\n";

      ectrl.add_chainsetup("default");
      ectrl.add_chain("default");
      ectrl.add_audio_input(filename);
      if (ectrl.get_audio_input() == 0) {
	cerr << "Error! Skipping file " << filename << "." << endl;
      }
      else {
	aio_params = ectrl.get_audio_format(ectrl.get_audio_input());
	ectrl.set_default_audio_format(aio_params);
	ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	ectrl.add_default_output();
	ectrl.connect_chainsetup();
	if (ectrl.is_connected() == false) {
	  cerr << "---\nError while playing file " << filename << ". Exiting...\n";
	  break;
	}
	ectrl.run();
	ectrl.disconnect_chainsetup();
      }
      ectrl.remove_chainsetup();
      cline.next();
    }
  }
  catch(ECA_ERROR& e) {
    cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
  catch(...) {
    cerr << "\nCaught an unknown exception.\n";
  }
  return(0);
}

void signal_handler(int signum) {
//    cerr << "Caught an interrupt... moving to next file.\n";
  if (ptimer_repp != 0) {
    ptimer_repp->stop();
    if (ptimer_repp->events_under_lower_bound() > 0) {
      cerr << endl << "Caught an exception. Exiting..." << endl;
      exit_request_rep = 1;
    }
    ptimer_repp->reset();
    ptimer_repp->set_lower_bound(lowerlimit_repp);
    ptimer_repp->start();
  }

  if (ectrl_repp != 0)
    ectrl_repp->quit();
}

void print_usage(void) {
  cerr << "****************************************************************************\n";
  cerr << "* ecatools_play, v" << ecatools_play_version;
  cerr << " (linked to ecasound v" << ecasound_library_version 
       << ")\n";
  cerr << "* (C) 1997-2001 Kai Vehmanen, released under GPL licence \n";
  cerr << "****************************************************************************\n";

  cerr << "\nUSAGE: ecatools_play file1 [ file2, ... fileN ]\n\n";
}
