// ------------------------------------------------------------------------
// eca-text.cpp: Console-mode user-interface to ecasound.
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <map>
#include <string>

#include <cstdlib>
#include <cstdio>
#include <cstring> /* strdup() */

#include <signal.h> /* sigaction(), sigwait() */
#include <pthread.h>
#include <unistd.h> /* getpid() */

#include <kvutils/com_line.h>

#include <eca-iamode-parser.h>
#include <eca-control.h>
#include <eca-session.h>
#include <eca-engine.h>
#include <eca-version.h>
#include <eca-debug.h>
#include <eca-error.h>
#include <eca-comhelp.h>

#ifdef HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#include <ncurses/term.h>
#else
#include <curses.h>
#include <term.h>
#endif

#define READLINE_LIBRARY
#include <readline.h>
#include <history.h>

#include "textdebug.h"
#include "eca-text.h"

/**
 * Return code values
 */

static const int ecasound_return_ok = 0;
static const int ecasound_return_signal = 1;
static const int ecasound_return_cpp_exception = 2;
static const int ecasound_return_unknown_exception = 2;

/**
 * Static/private global variables
 */

static ECA_CONTROL* ecasound_pointer_to_ecacontrol = 0;
static ECA_SESSION* ecasound_pointer_to_ecasession = 0;
static TEXTDEBUG ecasound_textdebug;
static int ecasound_error_no = 0;

/**
 * Static/private function definitions
 */

static void ecasound_clean_exit(int n);
static void ecasound_setup_signals(void);
static void ecasound_print_header(std::ostream* dostream);
static void ecasound_parse_command_line(COMMAND_LINE& cline);
static void ecasound_start_passive(ECA_SESSION* param);
static void ecasound_start_iactive(ECA_SESSION* param);
static void ecasound_start_iactive_readline(ECA_SESSION* param);
static void ecasound_init_readline_support(void);

/**
 * Function definitions
 */

int main(int argc, char *argv[])
{
  ecasound_setup_signals();

  try { 
    /* FIXME: remove this at some point; ecasound shoult not leak exceptions anymore */

    COMMAND_LINE cline (argc, argv);
    ecasound_parse_command_line(cline);

    bool debug_to_cerr = true;
    if (cline.has("-D") != true) {
      debug_to_cerr = false;
      attach_debug_object(&ecasound_textdebug);  
    }
    ecadebug->set_debug_level(ECA_DEBUG::info |
			      ECA_DEBUG::module_flow);

    if (cline.has("-o:stdout") ||
	cline.has("stdout") || 
	cline.has("-d:0") || 
	cline.has('q'))
      ecadebug->disable();
    else {
      if (debug_to_cerr == true)
	ecasound_print_header(&cerr);
      else
	ecasound_print_header(&std::cout);
    }

    try {
      ECA_SESSION* session = new ECA_SESSION(cline);
      ecasound_pointer_to_ecasession = session; // used only for signal handling!
      if (session->is_interactive() == true) {
#if defined USE_NCURSES || defined USE_TERMCAP
	ecasound_start_iactive_readline(session);
#else
	ecasound_start_iactive(session);
#endif
      }
      else {
	ecasound_start_passive(session);
      }
    }
    catch(ECA_ERROR& e) {
      /* problems with ECA_SESSION constructor (...parsing 'cline' options) */
      std::cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
      ecasound_clean_exit(ecasound_return_cpp_exception);
    }

  }
  catch(...) {
    std::cerr << "---\nCaught an unknown exception! (1)\n";
    std::cerr << "This is a severe programming error that should be reported!\n";
    ecasound_error_no = ecasound_return_unknown_exception;
  }

  ecadebug->flush();

  ecasound_clean_exit(ecasound_error_no);
}

void ecasound_start_passive(ECA_SESSION* param) {
  ECA_CONTROL* ctrl = new ECA_CONTROL(param);
  ecasound_pointer_to_ecacontrol = ctrl;

  ctrl->connect_chainsetup();
  if (ctrl->is_connected() == true) {
    ctrl->run();
  }
}

/**
 * Ecasound interactive mode without ncurses.
 */
void ecasound_start_iactive(ECA_SESSION* param) {
  ECA_CONTROL* ctrl = new ECA_CONTROL(param);
  ecasound_pointer_to_ecacontrol = ctrl;

  string cmd;
  do {
    if (cmd.size() > 0) {
      try {  /* FIXME: remove this at some point */
	ctrl->command(cmd);
	ctrl->print_last_error();
	ctrl->print_last_value();
	if (cmd == "quit" || cmd == "q") {
	  std::cerr << "---\nExiting...\n";
	  break;
	}
      }
      catch(...) {
	std::cerr << "---\nCaught an unknown exception! (2)\n";
	std::cerr << "This is a severe programming error that should be reported!\n";
	ecasound_error_no = 1;
      }
    }
    std::cout << "ecasound ('h' for help)> ";
  }
  while(getline(cin,cmd));
}

#if defined USE_NCURSES || defined USE_TERMCAP
/**
 * Ecasound interactive mode with ncurses.
 */
void ecasound_start_iactive_readline(ECA_SESSION* param) {
  ECA_CONTROL* ctrl = new ECA_CONTROL(param);
  ecasound_pointer_to_ecacontrol = ctrl;

  char* cmd = 0;
  ecasound_init_readline_support();
  do {
    cmd = readline("ecasound ('h' for help)> ");
    //      cmd = readline();
    if (cmd != 0) {
      add_history(cmd);
      try {  /* FIXME: remove this at some point */
	string str (cmd);
	ctrl->command(str);
	ctrl->print_last_error();
	ctrl->print_last_value();
	if (str == "quit" || str == "q") {
	  std::cerr << "---\nExiting...\n";
	  free(cmd); 
	  cmd = 0;
	  break;
	}
      }
      catch(...) {
	std::cerr << "---\nCaught an unknown exception! (3)\n";
	std::cerr << "This is a severe programming error that should be reported!\n";
	ecasound_error_no = ecasound_return_unknown_exception;
      }
      if (cmd != 0) {
	free(cmd);
      }

    }
  }
  while(cmd != 0);
}

/**
 * Parses the command lines options in 'cline'.
 */
void ecasound_parse_command_line(COMMAND_LINE& cline) {
  if (cline.size() < 2) {
    // No parameters, let's give some help.
    std::cout << ecasound_parameter_help();
    ecasound_clean_exit(ecasound_return_ok);
  }
  
  cline.begin();
  while(cline.end() == false) {
    if (cline.current() == "--version") {
      std::cout << "ecasound v" << ecasound_library_version << std::endl;
      std::cout << "Copyright (C) 1997-2001 Kai Vehmanen" << std::endl;
      std::cout << "Ecasound comes with ABSOLUTELY NO WARRANTY." << std::endl;
      std::cout << "You may redistribute copies of ecasound under the terms of the GNU" << std::endl;
      std::cout << "General Public License. For more information about these matters, see" << std::endl; 
      std::cout << "the file named COPYING." << std::endl;
      ecasound_clean_exit(ecasound_return_ok);
    }
    else if (cline.current() == "--help") {
      std::cout << ecasound_parameter_help();
      ecasound_clean_exit(ecasound_return_ok);
    }
    cline.next();
  }
}

/**
 * Exits ecasound. Before calling the final exit system
 * call, all static global resources are freed. This
 * is done to ensure that destructors are properly 
 * called.
 */
void ecasound_clean_exit(int n) {
  // std::cerr << "Clean exit..." << std::endl;
  ecadebug->flush();

  // std::cerr << "Killing control..." << std::endl;
  if (ecasound_pointer_to_ecacontrol != 0) {
    delete ecasound_pointer_to_ecacontrol;
    ecasound_pointer_to_ecacontrol = 0;
  }

  if (ecasound_pointer_to_ecasession != 0) {
    delete ecasound_pointer_to_ecasession;
    ecasound_pointer_to_ecasession = 0;
  }
  
  // std::cerr << "Exit..." << std::endl;
  exit(n);
}

void* ecasound_watchdog_thread(void *)
{
  sigset_t signalset;
  int signalno;

  // std::cerr << "Watchdog-thread created, pid=" << getpid() << "." << std::endl;

  sigemptyset(&signalset);

  /* handle the following signals explicitly */
  sigaddset(&signalset, SIGTERM);
  sigaddset(&signalset, SIGINT);
  sigaddset(&signalset, SIGHUP);
  sigaddset(&signalset, SIGPIPE);

  /* block until a signal received */
  sigwait(&signalset, &signalno);
  
  std::cerr << "Ecasound watchdog-thread received signal " << signalno << ". Exiting.." << std::endl << std::endl;

  ecasound_clean_exit(ecasound_return_signal);

  /* never reached */
  return(0);
}

/**
 * Sets up a signal mask with sigaction() that blocks 
 * all common signals, and then launces an watchdog
 * thread that waits on the blocked signals using
 * sigwait().
 */
void ecasound_setup_signals(void) {
  pthread_t watchdog;

  /* man pthread_sigmask:
   *  "...signal actions and signal handlers, as set with
   *   sigaction(2), are shared between all threads"
   */

  struct sigaction blockaction;
  blockaction.sa_handler = SIG_IGN;
  sigemptyset(&blockaction.sa_mask);
  blockaction.sa_flags = 0;

  /* ignore the following signals */
  sigaction(SIGTERM, &blockaction, 0);
  sigaction(SIGINT, &blockaction, 0);
  sigaction(SIGHUP, &blockaction, 0);
  sigaction(SIGPIPE, &blockaction, 0);

  int res = pthread_create(&watchdog, NULL, ecasound_watchdog_thread, NULL);
  if (res != 0) {
    std::cerr << "Warning! Unable to create watchdog thread." << std::endl;
  }
}

/**
 * Prints the ecasound banner.
 */
void ecasound_print_header(std::ostream* dostream) {
  *dostream << "****************************************************************************\n";
  *dostream << "*";
#if defined USE_NCURSES || defined USE_TERMCAP
  if (dostream == &std::cout) {
    setupterm((char *)0, 1, (int *)0);
    putp(tigetstr("bold"));
  }
#endif
  *dostream << "               ecasound v" 
	    << ecasound_library_version
	    << " (C) 1997-2001 Kai Vehmanen                 ";
#if defined USE_NCURSES || defined USE_TERMCAP
  if (dostream == &std::cout) {
    putp(tigetstr("sgr0"));
  }
#endif
  *dostream << "\n";
  *dostream << "****************************************************************************\n";
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator ();
char **fileman_completion ();

void ecasound_init_readline_support(void) {
  /* for conditional parsing of ~/.inputrc file. */
  rl_readline_name = "ecasound";

  /* we want to attempt completion first */
  rl_attempted_completion_function = (CPPFunction *)ecasound_completion;
}

/**
 * Attempt to complete on the contents of TEXT.  START and END bound the
 * region of rl_line_buffer that contains the word to complete.  TEXT is
 * the word to complete.  We can use the entire contents of rl_line_buffer
 * in case we want to do some simple parsing.  Return the array of matches,
 * or NULL if there aren't any.
 */
char** ecasound_completion (char *text, int start, int end) {
  char **matches;
  matches = (char **)NULL;

  /* complete only the first command, otherwise complete files in 
   * the current directory */
  if (start == 0)
    matches = completion_matches (text, (CPFunction *)ecasound_command_generator);

  return (matches);
}

/**
 * Generator function for command completion.  STATE lets us know whether
 * to start from scratch; without any state (i.e. STATE == 0), then we
 * start at the top of the list.
 */
char* ecasound_command_generator (char* text, int state) {
  static int list_index, len;
  static const std::map<std::string,int>& map_ref = ECA_IAMODE_PARSER::registered_commands();
  static std::map<std::string,int>::const_iterator p;
  static std::string cmd;

  /* If this is a new word to complete, initialize now.  This includes
   * saving the length of TEXT for efficiency, and initializing the index
   * variable to 0
   */
  if (!state) {
      list_index = 0;
      p = map_ref.begin();
      len = strlen (text);
      // std::cerr << "First:" << p->first << ",";
  }
  /* Return the next name which partially matches from the command list */
  while (p != map_ref.end()) {
      cmd = p->first;
      list_index++;
      // std::cerr << "Cmd:" << cmd << " (" << list_index << "),";
      ++p;
      if (p != map_ref.end()) {
	// std::cerr << text << " = " << cmd << "\n";
	string hyphenstr = string_search_and_replace(text, '_', '-');
	if (strncmp(hyphenstr.c_str(), cmd.c_str(), len) == 0) {
  	  // std::cerr << "Len: " << len << " - compare returns: " << cmd.compare(text, 0, len) << ".\n";
	  return(strdup(cmd.c_str()));
	}
      }
  }
  // std::cerr << "NULL";
  return ((char *)0);
}

#endif /* defined USE_NCURSES || defined USE_TERMCAP */
