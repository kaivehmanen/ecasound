// ------------------------------------------------------------------------
// ecatools-signalview.cpp: A simple command-line tools for monitoring
//                          signal amplitude.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <cstdio>
#include <signal.h>
#include <unistd.h>

#include <kvutils/com_line.h>
#include <kvutils/kvutils.h>

#include <eca-debug.h>
#include <eca-error.h>
#include <eca-control.h>
#include <eca-engine.h>
#include <eca-session.h>
#include <audiofx_analysis.h>
#include <audiofx_amplitude.h>
#include <audioio.h>
#include <eca-version.h>

#ifdef HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#include <ncurses/term.h>
#else
#include <curses.h>
#include <term.h>
#endif

#include "ecatools_signalview.h"

bool enable_debug, enable_cumulative_mode;
long int buffersize, rate_msec;
string input, output, format_string;

void parse_command_line(int argc, char *argv[]) {
  COMMAND_LINE cline = COMMAND_LINE (argc, argv);
  if (cline.size() == 0 ||
      cline.has("--version") ||
      cline.has("--help")) {
    print_usage();
    exit(1);
  }

  enable_debug = false;
  enable_cumulative_mode = false;
  rate_msec = 0; 
  buffersize = 0;

  cline.begin();
  cline.next(); // 1st argument
  while (cline.end() != true) {
    string arg = cline.current();
    if (arg.size() > 0) {
      if (arg[0] != '-') {
	if (input == "")
	  input = arg;
	else
	  if (output == "")
	    output = arg;
      }
      else {
	string prefix = get_argument_prefix(arg);
	if (prefix == "b") 
	  buffersize = atol(get_argument_number(1, arg).c_str());
	if (prefix == "c") enable_cumulative_mode = true;
	if (prefix == "d") enable_debug = true;
	if (prefix == "f")
	  format_string = arg;
	if (prefix == "r") 
	  rate_msec = atol(get_argument_number(1, arg).c_str());
      }
    }
    cline.next();
  }
  if (input.size() == 0) input = "/dev/dsp";
  if (output.size() == 0) output = "null";
  if (buffersize == 0) buffersize = 128;
  if (rate_msec == 0) rate_msec = 200;
}

int main(int argc, char *argv[])
{
  struct sigaction es_handler;
  es_handler.sa_handler = signal_handler;
  sigemptyset(&es_handler.sa_mask);
  es_handler.sa_flags = 0;

  sigaction(SIGTERM, &es_handler, 0);
  sigaction(SIGINT, &es_handler, 0);
  sigaction(SIGQUIT, &es_handler, 0);
  sigaction(SIGABRT, &es_handler, 0);

  parse_command_line(argc,argv);

  if (enable_debug)
    ecadebug->set_debug_level(ECA_DEBUG::info |
			      ECA_DEBUG::module_flow);
  else
    ecadebug->disable();

  try {
    ECA_SESSION esession;
    ECA_CONTROL ectrl (&esession);
//      ectrl.toggle_interactive_mode(true);
    ECA_AUDIO_FORMAT aio_params;
    ectrl.add_chainsetup("default");
    ectrl.add_chain("default");
    ectrl.set_buffersize(buffersize);
    if (format_string.size() > 0)  
      ectrl.set_chainsetup_parameter(format_string);
    ectrl.add_audio_input(input);
    if (ectrl.get_audio_input() == 0) {
      std::cerr << "---\nError while opening \"" << input << "\". Exiting...\n";
      exit(0);
    }
    if (format_string.size() == 0) {
      ectrl.set_default_audio_format_to_selected_input();
      aio_params = ectrl.default_audio_format();
      ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
    }
    ectrl.add_audio_output(output);
    if (ectrl.get_audio_output() == 0) {
      std::cerr << "---\nError while opening \"" << output << "\". Exiting...\n";
      exit(0);
    }
    EFFECT_ANALYZE cop;
    if (enable_cumulative_mode == true)
      cop.set_parameter(1, 1);
    ectrl.add_chain_operator((CHAIN_OPERATOR*)&cop);
    ectrl.connect_chainsetup();
    if (ectrl.is_connected() == false) {
      std::cerr << "---\nError while connecting chainsetup.\n";
      exit(0);

    }

#if defined USE_NCURSES || defined USE_TERMCAP
    initscr();
    erase();
    move(0,0);
    refresh();
#endif

    ectrl.start();
    while(true) {
      usleep(rate_msec * 1000);
#if defined USE_NCURSES || defined USE_TERMCAP
      erase();
      move(0,0);
      refresh();
      printw("%s", cop.status().c_str());
      refresh();
#else
      std::cout << cop.status() << std::endl;
#endif
    }
#if defined USE_NCURSES || defined USE_TERMCAP
    endwin();
#endif
  }
  catch(ECA_ERROR& e) {
    std::cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
  catch(...) {
    std::cerr << "\nCaught an unknown exception.\n";
  }
  return(0);
}

void print_usage(void) {
  std::cerr << "****************************************************************************\n";
  std::cerr << "* ecatools_signalview, v" << ecatools_signalview_version;
  std::cerr << " (linked to ecasound v" << ecasound_library_version 
       << ")\n";
  std::cerr << "* (C) 1997-2000 Kai Vehmanen, released under GPL licence\n";
  std::cerr << "****************************************************************************\n";

  std::cerr << "\nUSAGE: ecatools_signalview [options] [input] [output] \n";
  std::cerr << "\toptions:";
  std::cerr << "\t-b:buffersize\n";
  std::cerr << "\t-c (cumulative mode)\n";
  std::cerr << "\t-d (debug mode)\n";
  std::cerr << "\t-r:refresh_msec\n\n";
}

void signal_handler(int signum) {
  std::cerr << "Unexpected interrupt... cleaning up.\n";
#if defined USE_NCURSES || defined USE_TERMCAP
  endwin();
#endif
  exit(1);
}
