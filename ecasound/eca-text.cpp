// ------------------------------------------------------------------------
// eca-text.cpp: Textmode user-interface routines for ecasound.
// Copyright (C) 1999-2001 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <signal.h>
#include <cstdio>

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

static ECA_ENGINE* global_pointer_to_ecaengine = 0; 
static bool global_engine_deleted = false;
static ECA_SESSION* global_pointer_to_ecasession = 0; 
static bool global_session_deleted = false;
static ECA_CONTROL* global_pointer_to_ecacontrol = 0; 
static bool global_control_deleted = false;

TEXTDEBUG textdebug;
static int global_error_no = 0;

void clean_exit(int n);

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

  try { /* FIXME: remove this at some point */

    COMMAND_LINE cline (argc, argv);
    parse_command_line(cline);

    bool debug_to_cerr = true;
    if (cline.has("-D") != true) {
      debug_to_cerr = false;
      attach_debug_object(&textdebug);  
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
	print_header(&cerr);
      else
	print_header(&cout);
    }

    try {
      ECA_SESSION* session = new ECA_SESSION(cline);

      global_pointer_to_ecasession = session; // used only for signal handling!
      if (session->is_interactive()) {
#if defined USE_NCURSES || defined USE_TERMCAP
	start_iactive_readline(session);
#else
	start_iactive(session);
#endif
      }
      else {
	if (session->is_selected_chainsetup_connected() == true) {
	  ECA_ENGINE* epros = new ECA_ENGINE(session);
	  global_pointer_to_ecaengine = epros;
	  epros->exec();
	}
      }
    }
    catch(ECA_ERROR& e) {
      /* problems with ECA_SESSION constructor (...parsing 'cline' options) */
      cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
      clean_exit(127);
    }

  }
  catch(...) {
    cerr << "---\nCaught an unknown exception! (1)\n";
    cerr << "This is a severe programming error that should be reported!\n";
    global_error_no = 1;
  }

  ecadebug->flush();

//    cerr << "Normal exit..." << endl;

  clean_exit(global_error_no);
}

/**
 * Parses the command lines options in 'cline'.
 */
void parse_command_line(COMMAND_LINE& cline) {
  if (cline.size() < 2) {
    // No parameters, let's give some help.
    cout << ecasound_parameter_help();
    clean_exit(0);
  }
  
  cline.begin();
  while(cline.end() == false) {
    if (cline.current() == "--version") {
      cout << "ecasound v" << ecasound_library_version << endl;
      cout << "Copyright (C) 1997-2001 Kai Vehmanen" << endl;
      cout << "Ecasound comes with ABSOLUTELY NO WARRANTY." << endl;
      cout << "You may redistribute copies of ecasound under the terms of the GNU" << endl;
      cout << "General Public License. For more information about these matters, see" << endl; 
      cout << "the file named COPYING." << endl;
      exit(0);
    }
    else if (cline.current() == "--help") {
      cout << ecasound_parameter_help();
      clean_exit(0);
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
void clean_exit(int n) {
//    cerr << "Clean exit..." << endl;
  ecadebug->flush();
  if (global_control_deleted == false) {
    global_control_deleted = true;
    if (global_pointer_to_ecacontrol != 0) {
      global_pointer_to_ecacontrol->~ECA_CONTROL();
      global_pointer_to_ecacontrol = 0;
    }
  }

  if (global_engine_deleted == false) {
    global_engine_deleted = true;
    if (global_pointer_to_ecaengine != 0) {
      global_pointer_to_ecaengine->~ECA_ENGINE();
      global_pointer_to_ecaengine = 0;
    }
  }
  if (global_session_deleted == false) {
    global_session_deleted = true;
    if (global_pointer_to_ecasession != 0) {
      global_pointer_to_ecasession->~ECA_SESSION();
      global_pointer_to_ecasession = 0;
    }
  }
  exit(n);
}

/**
 * Signal handling call back.
 */
void signal_handler(int signum) {
//    cerr << "<-- Caught a signal... cleaning up." << endl << endl;
  clean_exit(128);
}

/**
 * Prints the ecasound banner.
 */
void print_header(ostream* dostream) {
  *dostream << "****************************************************************************\n";
  *dostream << "*";
#if defined USE_NCURSES || defined USE_TERMCAP
  if (dostream == &cout) {
    setupterm((char *)0, 1, (int *)0);
    putp(tigetstr("bold"));
  }
#endif
  *dostream << "               ecasound v" 
	    << ecasound_library_version
	    << " (C) 1997-2001 Kai Vehmanen                 ";
#if defined USE_NCURSES || defined USE_TERMCAP
  if (dostream == &cout) {
    putp(tigetstr("sgr0"));
  }
#endif
  *dostream << "*\n";
  *dostream << "****************************************************************************\n";
}

/**
 * Ecasound interactive mode without ncurses.
 */
void start_iactive(ECA_SESSION* param) {
  ECA_CONTROL* ctrl = new ECA_CONTROL(param);
  global_pointer_to_ecacontrol = ctrl;

  string cmd;
  do {
    if (cmd.size() > 0) {
      try {  /* FIXME: remove this at some point */
	ctrl->command(cmd);
	ctrl->print_last_error();
	ctrl->print_last_value();
	if (cmd == "quit" || cmd == "q") {
	  cerr << "---\nExiting...\n";
	  break;
	}
      }
      catch(...) {
	cerr << "---\nCaught an unknown exception! (2)\n";
	cerr << "This is a severe programming error that should be reported!\n";
	global_error_no = 1;
      }
    }
    cout << "ecasound ('h' for help)> ";
  }
  while(getline(cin,cmd));
}

#if defined USE_NCURSES || defined USE_TERMCAP
/**
 * Ecasound interactive mode with ncurses.
 */
void start_iactive_readline(ECA_SESSION* param) {
  ECA_CONTROL* ctrl = new ECA_CONTROL(param);
  global_pointer_to_ecacontrol = ctrl;

  char* cmd = 0;
  init_readline_support();
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
	  cerr << "---\nExiting...\n";
	  free(cmd); cmd = 0;
	  break;
	}
      }
      catch(...) {
	cerr << "---\nCaught an unknown exception! (3)\n";
	cerr << "This is a severe programming error that should be reported!\n";
	global_error_no = 1;
      }
      if (cmd != 0) {
	free(cmd);
      }

    }
  }
  while(cmd != 0);
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator ();
char **fileman_completion ();

void init_readline_support(void) {
  // for conditional parsing of ~/.inputrc file.
  rl_readline_name = "ecasound";

  // we want to attempt completion first
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

  // complete only the first command, otherwise complete files in 
  // the current directory
  if (start == 0)
    matches = completion_matches (text, (CPFunction *)ecasound_command_generator);

  return (matches);
}

/**
 * Generator function for command completion.  STATE lets us know whether
 *  to start from scratch; without any state (i.e. STATE == 0), then we
 * start at the top of the list.
 */
char* ecasound_command_generator (char* text, int state) {
  static int list_index, len;
  static const std::map<std::string,int>& map_ref = ECA_IAMODE_PARSER::registered_commands();
  static std::map<std::string,int>::const_iterator p;
  static std::string cmd;

  // If this is a new word to complete, initialize now.  This includes
  // saving the length of TEXT for efficiency, and initializing the index
  // variable to 0
  if (!state) {
      list_index = 0;
      p = map_ref.begin();
      len = strlen (text);
      //      cerr << "First:" << p->first << ",";
  }
  // Return the next name which partially matches from the command list
  while (p != map_ref.end()) {
      cmd = p->first;
      list_index++;
      //      cerr << "Cmd:" << cmd << " (" << list_index << "),";
      ++p;
      if (p != map_ref.end()) {
	//	cerr << text << " = " << cmd << "\n";
	//	if (cmd.compare(text, 0, len) == cmd.size() - len) {
	string hyphenstr = string_search_and_replace(text, '_', '-');
	if (strncmp(hyphenstr.c_str(), cmd.c_str(), len) == 0) {
  	  //	  cerr << "Len: " << len << " - compare returns: " << cmd.compare(text, 0, len) << ".\n";
	  return(strdup(cmd.c_str()));
	}
      }
      //      cmd.find_first_of(text, 0, len) != string::npos)
  }
  //  cerr << "NULL";
  // no names matched, return null
  return ((char *)0);
}
#endif // defined USE_NCURSES || defined USE_TERMCAP
