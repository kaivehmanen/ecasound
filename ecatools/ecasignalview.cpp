// ------------------------------------------------------------------------
// ecasignalview.cpp: A simple command-line tools for monitoring
//                    signal amplitude.
// Copyright (C) 1999-2005 Kai Vehmanen
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

#include <iostream>
#include <string>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstdio>

#include <signal.h>
#include <unistd.h>

#include <kvutils/kvu_com_line.h>
#include <kvutils/kvu_utils.h>
#include <kvutils/kvu_numtostr.h>

#include <eca-control-interface.h>

#include "ecicpp_helpers.h"

#if defined(ECA_USE_NCURSES_H) || defined(ECA_USE_NCURSES_NCURSES_H) || defined(ECA_USE_CURSES_H)
#define ECASV_USE_CURSES 1

#ifdef ECA_USE_NCURSES_H
#include <ncurses.h>
#include <term.h> /* for setupterm() */
#elif ECA_USE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#include <ncurses/term.h> /* for setupterm() */
#else
#include <curses.h>
#include <term.h> /* for setupterm() */
#endif

#endif /* ECA_*CURSES_H */

/**
 * Import namespaces 
 */

using namespace std;

/**
 * Type definitions
 */

struct ecasv_channel_stats {
  double last_peak;
  double drawn_peak;
  double max_peak;
  long int clipped_samples;
};

/**
 * Function declarations
 */

int main(int argc, char *argv[]);
void ecasv_parse_command_line(int argc, char *argv[]);
void ecasv_fill_defaults(void);
std::string ecasv_cop_to_string(ECA_CONTROL_INTERFACE* cop);
void ecasv_output_init(void);
void ecasv_output_cleanup(void);
void ecasv_print_vu_meters(ECA_CONTROL_INTERFACE* eci, std::vector<struct ecasv_channel_stats>* chstats);
void ecasv_update_chstats(std::vector<struct ecasv_channel_stats>* chstats, int ch, double value);
void ecasv_create_bar(double value, int barlen, unsigned char* barbuf);
void ecasv_print_usage(void);
void ecasv_signal_handler(int signum);

/**
 * Static global variables
 */

static const string ecatools_signalview_version = "20050401-7";
static const double ecasv_clipped_threshold_const = 1.0f - 1.0f / 16384.0f;
static const int ecasv_bar_length_const = 32;
static const int ecasv_header_height_const = 10;
static const long int ecasv_rate_default_const = 50;
static const long int ecasv_buffersize_default_const = 128;

static unsigned char ecasv_bar_buffer[ecasv_bar_length_const + 1] = { 0 };
static bool ecasv_enable_debug, ecasv_enable_cumulative_mode;
static long int ecasv_buffersize, ecasv_rate_msec;
static string ecasv_input, ecasv_output, ecasv_format_string;
static int ecasv_chcount = 0;

static ECA_CONTROL_INTERFACE* ecasv_eci_repp = 0;

static sig_atomic_t done = 0;
static sig_atomic_t reset_stats = 0;

/**
 * Function definitions
 */

int main(int argc, char *argv[])
{
  struct sigaction es_handler;
  es_handler.sa_handler = ecasv_signal_handler;
  sigemptyset(&es_handler.sa_mask);
  es_handler.sa_flags = 0;

  sigaction(SIGTERM, &es_handler, 0);
  sigaction(SIGINT, &es_handler, 0);
  sigaction(SIGQUIT, &es_handler, 0);
  sigaction(SIGABRT, &es_handler, 0);
  sigaction(SIGHUP, &es_handler, 0);

  struct sigaction ign_handler;
  ign_handler.sa_handler = SIG_IGN;
  sigemptyset(&ign_handler.sa_mask);
  ign_handler.sa_flags = 0;

  /* ignore the following signals */
  sigaction(SIGPIPE, &ign_handler, 0);
  sigaction(SIGFPE, &ign_handler, 0);

  ecasv_parse_command_line(argc,argv);

  ECA_CONTROL_INTERFACE eci;

  eci.command("cs-add default");
  eci.command("c-add default");
  eci.command("cs-set-param -b:" + kvu_numtostr(ecasv_buffersize));

  if (ecasv_format_string.size() > 0) {
    eci.command("cs-set-audio-format " + ecasv_format_string);
  }
  
  string format;
  if (ecicpp_add_input(&eci, ecasv_input, &format) < 0) return -1;

  cout << "Using audio format -f:" << format << "\n";

  ecasv_chcount = ecicpp_format_channels(format);
  cout << "Setting up " << ecasv_chcount << " separate channels for analysis." << endl;
  
  if (ecicpp_add_output(&eci, ecasv_output, format) < 0) return -1;
  
  ecasv_eci_repp = &eci;

  vector<struct ecasv_channel_stats> chstats;

  eci.command("cop-add -evp");
  eci.command("cop-add -ev");
  if (ecasv_enable_cumulative_mode == true) {
    eci.command("cop-set 2,1,1");
  }

  eci.command("cop-select 1");

  if (ecicpp_connect_chainsetup(&eci, "default") < 0) {
    return -1;
  }

  int secs = 0, msecs = ecasv_rate_msec;
  while(msecs > 999) {
    ++secs;
    msecs -= 1000;
  }

  ecasv_output_init();

  eci.command("start");

  while(! done ) {
    kvu_sleep(secs, msecs * 1000000);
    ecasv_print_vu_meters(&eci, &chstats);
  }

  ecasv_output_cleanup();
#ifdef ECASV_USE_CURSES
  endwin();
#endif
  
  return 0;
}

void ecasv_parse_command_line(int argc, char *argv[])
{
  COMMAND_LINE cline = COMMAND_LINE (argc, argv);
  if (cline.size() == 0 ||
      cline.has("--version") ||
      cline.has("--help") ||
      cline.has("-h")) {
    ecasv_print_usage();
    exit(1);
  }

  ecasv_enable_debug = false;
  ecasv_enable_cumulative_mode = false;
  ecasv_rate_msec = 0; 
  ecasv_buffersize = 0;

  cline.begin();
  cline.next(); // 1st argument
  while (cline.end() != true) {
    string arg = cline.current();
    if (arg.size() > 0) {
      if (arg[0] != '-') {
	if (ecasv_input == "")
	  ecasv_input = arg;
	else
	  if (ecasv_output == "")
	    ecasv_output = arg;
      }
      else {
	string prefix = kvu_get_argument_prefix(arg);
	if (prefix == "b") 
	  ecasv_buffersize = atol(kvu_get_argument_number(1, arg).c_str());
	if (prefix == "c") ecasv_enable_cumulative_mode = true;
	if (prefix == "d") ecasv_enable_debug = true;
	if (prefix == "f")
	  ecasv_format_string = string(arg.begin() + 3, arg.end());
	if (prefix == "r") 
	  ecasv_rate_msec = atol(kvu_get_argument_number(1, arg).c_str());
      }
    }
    cline.next();
  }

  ecasv_fill_defaults();
}

void ecasv_fill_defaults(void)
{
  // ECA_RESOURCES ecarc;

  if (ecasv_input.size() == 0) ecasv_input = "/dev/dsp";
  if (ecasv_output.size() == 0) ecasv_output = "null";
  if (ecasv_buffersize == 0) ecasv_buffersize = ecasv_buffersize_default_const;
  if (ecasv_rate_msec == 0) ecasv_rate_msec = ecasv_rate_default_const;
  if (ecasv_format_string.size() == 0) ecasv_format_string = "s16_le,2,44100,i";

  // ecarc.resource("default-audio-format");
}

string ecasv_cop_to_string(ECA_CONTROL_INTERFACE* eci)
{
  eci->command("cop-status");
  return(eci->last_string());
}

void ecasv_output_init(void)
{
#ifdef ECASV_USE_CURSES
    initscr();
    erase();

    mvprintw(0, 0, "******************************************************\n");
    mvprintw(1, 0, "* ecasignalview v%s (C) 1999-2002 Kai Vehmanen  \n", ecatools_signalview_version.c_str());
    mvprintw(2, 0, "******************************************************\n\n");

    mvprintw(4, 0, "input=\"%s\"\noutput=\"%s\"\naudio_format=\"%s\", refresh_rate_ms='%ld', buffersize='%ld'\n", 
	     ecasv_input.c_str(), ecasv_output.c_str(), ecasv_format_string.c_str(), ecasv_rate_msec, ecasv_buffersize);

    memset(ecasv_bar_buffer, ' ', ecasv_bar_length_const - 4);
    ecasv_bar_buffer[ecasv_bar_length_const - 4] = 0;

    mvprintw(8, 0, "channel-# %s| max-peak  clipped-samples\n", ecasv_bar_buffer);
    mvprintw(9, 0, "----------------------------------------------------------------\n");

    move(12, 0);

    refresh();
#endif
}

void ecasv_output_cleanup(void)
{
#ifdef ECASV_USE_CURSES
    endwin();
#endif

    // FIXME: should be enabled
#if 0    
    if (ecasv_eci_repp != 0) {
      cout << endl << endl << endl;
      ecasv_eci_repp->command("cop-status");
    }
#endif
}

void ecasv_print_vu_meters(ECA_CONTROL_INTERFACE* eci, vector<struct ecasv_channel_stats>* chstats)
{
  /* check wheter to reset peaks */
  if (reset_stats) {
    reset_stats = 0;
    for(int n = 0; n < ecasv_chcount; n++) {
      (*chstats)[n].max_peak = 0;
      (*chstats)[n].clipped_samples = 0;
    }
  }

#ifdef ECASV_USE_CURSES
  for(int n = 0; n < ecasv_chcount; n++) {
    eci->command("copp-select " + kvu_numtostr(n + 1));
    eci->command("copp-get");
    double value = eci->last_float();

    ecasv_update_chstats(chstats, n, value);

    ecasv_create_bar((*chstats)[n].drawn_peak, ecasv_bar_length_const, ecasv_bar_buffer);
    mvprintw(ecasv_header_height_const+n, 0, "Ch-%02d: %s| %.5f       %ld\n", 
	     n + 1, ecasv_bar_buffer, (*chstats)[n].max_peak, (*chstats)[n].clipped_samples);
  }
  move(ecasv_header_height_const + 2 + ecasv_chcount, 0);
  refresh();
#else
  cout << ecasv_cop_to_string(eci) << endl;
#endif
}

void ecasv_update_chstats(vector<struct ecasv_channel_stats>* chstats, int ch, double value)
{
  /* 1. in case a new channel is encoutered */
  if (static_cast<int>(chstats->size()) <= ch) {
    chstats->resize(ch + 1);
  }
  
  /* 2. update last_peak and drawn_peak */
  (*chstats)[ch].last_peak = value;
  if ((*chstats)[ch].last_peak < (*chstats)[ch].drawn_peak) {
    (*chstats)[ch].drawn_peak *= ((*chstats)[ch].last_peak / (*chstats)[ch].drawn_peak);
  }
  else {
    (*chstats)[ch].drawn_peak = (*chstats)[ch].last_peak;
  }

  /* 3. update max_peak */
  if (value > (*chstats)[ch].max_peak) {
    (*chstats)[ch].max_peak = value;
  }

  /* 4. update clipped_samples counter */
  if (value > ecasv_clipped_threshold_const) {
    (*chstats)[ch].clipped_samples++;
  }
}

void ecasv_create_bar(double value, int barlen, unsigned char* barbuf)
{
  int curlen = static_cast<int>(rint(((value / 1.0f) * barlen)));
  for(int n = 0; n < barlen; n++) {
    if (n <= curlen)
      barbuf[n] = '*';
    else
      barbuf[n] = ' ';
  }
}

void ecasv_print_usage(void)
{
  cerr << "****************************************************************************\n";
  cerr << "* ecasignalview, v" << ecatools_signalview_version << " (" << VERSION << ")\n";
  cerr << "* (C) 1999-2004 Kai Vehmanen, released under GPL licence\n";
  cerr << "****************************************************************************\n";

  cerr << "\nUSAGE: ecasignalview [options] [input] [output] \n";
  cerr << "\toptions:\n";
  cerr << "\t\t-b:buffersize\n";
  // cerr << "\t\t-c (cumulative mode)\n";
  cerr << "\t\t-d (debug mode)\n";
  cerr << "\t\t-f:bits,channels,samplerate\n";
  cerr << "\t\t-r:refresh_msec\n\n";
}

void ecasv_signal_handler(int signum)
{
  if (signum == SIGHUP) {
    reset_stats = 1;
  }
  else {
    cerr << "Interrupted... cleaning up.\n";
    done=1;
  }
}
