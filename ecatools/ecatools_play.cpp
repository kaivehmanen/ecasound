// ------------------------------------------------------------------------
// ecatools-play.cpp: A simple command-line tool for playing audio files
//                    using the default output device specified in 
//                    "~/.ecasoundrc".
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <signal.h> /* sigaction() */

#include <kvutils/kvu_com_line.h>
#include <kvutils/kvu_procedure_timer.h>
#include <kvutils/kvu_numtostr.h>

#include <eca-debug.h>
#include <eca-error.h>
#include <eca-control.h>
#include <eca-session.h>
#include <eca-version.h>

#include "ecatools_play.h"

static ECA_CONTROL* ecaplay_ectrl_repp = 0;
static PROCEDURE_TIMER* ecaplay_ptimer_repp = 0;
static struct timeval* ecaplay_lowerlimit_repp;
static int ecaplay_exit_request_rep = 0;
static int ecaplay_skip_files = 0;
static int ecaplay_total_files = 0;

static const string ecaplay_version = "20020427";

int process_option(const string& option);

int main(int argc, char *argv[])
{
  ecadebug->disable();
  // ecadebug->set_debug_level(15);

  struct sigaction es_handler_int;
  es_handler_int.sa_handler = signal_handler;
  sigemptyset(&es_handler_int.sa_mask);
  es_handler_int.sa_flags = 0;

  struct sigaction ign_handler;
  ign_handler.sa_handler = SIG_IGN;
  sigemptyset(&ign_handler.sa_mask);
  ign_handler.sa_flags = 0;

  /* handle the follwing signals explicitly */
  sigaction(SIGINT, &es_handler_int, 0);

  /* ignore the following signals */
  sigaction(SIGPIPE, &ign_handler, 0);

  COMMAND_LINE cline = COMMAND_LINE (argc, argv);

  if (cline.size() < 2) {
    print_usage();
    return(1);
  }

  cline.combine();
  
  PROCEDURE_TIMER ptimer;
  ecaplay_ptimer_repp = &ptimer;

  struct timeval lowerlimit;
  ecaplay_lowerlimit_repp = &lowerlimit;
  lowerlimit.tv_sec = 1;
  lowerlimit.tv_usec = 0;

  try {
    ECA_SESSION esession;
    ECA_CONTROL ectrl (&esession);
    ecaplay_ectrl_repp = &ectrl;
    ECA_AUDIO_FORMAT aio_params;

    cline.begin();
    cline.next(); // skip the program name

    ecaplay_total_files = cline.size() - 1;
    int playing_file = 0;
    int consecutive_errors = 0;
    while(cline.end() == false &&
	  ecaplay_exit_request_rep == 0) {

      // --
      // parse command-line options
      if (cline.current().size() > 1 &&
	  cline.current()[0] == '-') {
	int ret = process_option(cline.current());
	if (ret != 0) return(ret);

	cline.next();
	--ecaplay_total_files;
	continue;
      }
      
      string filename = cline.current();
      ++playing_file;

      if (ecaplay_skip_files == 0) {
	std::cout << "(ecaplay) Playing file '" << filename << "'";
        std::cout << " (" << playing_file;
	std::cout << "/" << ecaplay_total_files;
	std::cout << ")." << std::endl;
      }
      else {
	std::cout << "(ecaplay) Skipping file '" << filename << "'." << std::endl;
	--ecaplay_skip_files;
	cline.next();
	continue;
      }

      ectrl.add_chainsetup("default");
      ectrl.add_chain("default");
      ectrl.add_audio_input(filename);
      if (ectrl.get_audio_input() == 0) {
	std::cerr << "(ecaplay) Error! Skipping file " << filename << "." << std::endl;
      }
      else {
	ectrl.set_default_audio_format_to_selected_input();
	aio_params = ectrl.default_audio_format();
	ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	ectrl.add_default_output();
	ectrl.connect_chainsetup();
	if (ectrl.is_connected() != true) {
	  std::cerr << "(ecaplay) Error while playing file " << filename << ". Skipping...\n";
	  ++consecutive_errors;
	  if (consecutive_errors == 3) {
	    std::cerr << "(ecaplay) Too many errors, exiting." << std::endl;
	    ectrl.disconnect_chainsetup();
	    ectrl.remove_chainsetup();
	    break;
	  }
	}
	else {
	  consecutive_errors = 0;
	  ectrl.run();
	  if (ectrl.is_connected() == true) ectrl.disconnect_chainsetup();
	}
      }
      ectrl.remove_chainsetup();
      cline.next();
    }
  }
  catch(ECA_ERROR& e) {
    std::cerr << "(ecaplay) ---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
  catch(...) {
    std::cerr << "\n(ecaplay) Caught an unknown exception.\n";
  }
  return(0);
}

void signal_handler(int signum) {
  // std::cerr << "Caught an interrupt... moving to next file.\n";
  if (ecaplay_ptimer_repp != 0) {
    ecaplay_ptimer_repp->stop();
    if (ecaplay_ptimer_repp->events_under_lower_bound() > 0) {
      std::cerr << std::endl << "(ecaplay) Caught an exception. Exiting..." << std::endl;
      ecaplay_exit_request_rep = 1;
    }
    ecaplay_ptimer_repp->reset();
    ecaplay_ptimer_repp->set_lower_bound(ecaplay_lowerlimit_repp);
    ecaplay_ptimer_repp->start();
  }

  if (ecaplay_ectrl_repp != 0) {
    if (ecaplay_ectrl_repp->is_running() == true) ecaplay_ectrl_repp->stop();
  }
}

void print_usage(void) {
  std::cerr << "****************************************************************************\n";
  std::cerr << "* ecatools_play, v" << ecaplay_version;
  std::cerr << " (linked to ecasound v" << ecasound_library_version 
       << ")\n";
  std::cerr << "* (C) 1997-2001 Kai Vehmanen, released under GPL licence \n";
  std::cerr << "****************************************************************************\n";

  std::cerr << "\nUSAGE: ecaplay [-dhk] file1 [ file2, ... fileN ]\n\n";
}

int process_option(const string& option) {
  if (option == "--help" ||
      option == "--version") {
    print_usage();
    return(1);
  }

  switch(option[1]) 
    {
    case 'd': 
      {
	int debuglevel = atoi(kvu_get_argument_number(1,option).c_str());
	ecadebug->set_debug_level(debuglevel);
	break;
      }
      
    case 'k': 
      {
	ecaplay_skip_files = atoi(kvu_get_argument_number(1,option).c_str());
	break;
      }
      
    case 'h': 
      {
	print_usage();
	return(1);
      }
      
    default:
      {
	std::cerr << "(ecaplay) Error! Unknown option '" << option
		  << "'." << std::endl;
	return(2);
      }
    }
  return(0);
}
