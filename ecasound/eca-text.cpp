// ------------------------------------------------------------------------
// eca-text.cpp: Textmode user-interface routines for ecasound.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <signal.h>

#include <kvutils.h>

#include <eca-iamode-parser.h>
#include <eca-controller.h>
#include <eca-session.h>
#include <eca-main.h>
#include <eca-version.h>
#include <eca-debug.h>
#include <eca-error.h>

#define READLINE_LIBRARY
#include <../readline-4.0/readline.h>
#include <../readline-4.0/history.h>

#include "textdebug.h"
#include "eca-text.h"

ECA_SESSION* global_pointer_to_ecaparams = 0; 
bool global_session_deleted = false;
TEXTDEBUG textdebug;

int main(int argc, char *argv[])
{

  ECA_SESSION* ecaparams = 0;
  attach_debug_object(&textdebug);  
  ecadebug->set_debug_level(0);

  struct sigaction es_handler;
  es_handler.sa_handler = signal_handler;
  sigemptyset(&es_handler.sa_mask);
  es_handler.sa_flags = 0;
  es_handler.sa_restorer = 0;

  sigaction(SIGTERM, &es_handler, 0);
  sigaction(SIGINT, &es_handler, 0);
  sigaction(SIGQUIT, &es_handler, 0);
  sigaction(SIGABRT, &es_handler, 0);

  try {
    COMMAND_LINE cline = COMMAND_LINE (argc, argv);

    if (cline.has("-o:stdout") ||
	cline.has("stdout") || 
	cline.has('q'))
      ecadebug->disable();
    
    ecadebug->msg(print_header());

    ecaparams = new ECA_SESSION(cline);
    global_pointer_to_ecaparams = ecaparams;  // used only for signal handling! 
    if (ecaparams->is_interactive()) start_iactive_readline(ecaparams);
    else {
      ECA_PROCESSOR epros (ecaparams);
      epros.exec();
    }
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\nCaught an unknown exception!\n";
  }

  try {
    if (global_session_deleted == false) {
      global_session_deleted = true;
      if (ecaparams != 0) delete ecaparams;
    }
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\nCaught an unknown exception!\n";
  }
  //  ecaparams->~ECA_SESSION();
  ecadebug->flush();
    
  return(0); // everything ok!
}

void signal_handler(int signum) {
  cerr << "Unexpected interrupt... cleaning up.\n";
  if (global_session_deleted == false) {
    global_session_deleted = true;
    if (global_pointer_to_ecaparams != 0) delete global_pointer_to_ecaparams;
  }
  //  delete global_pointer_to_ecaparams;
  //  global_pointer_to_ecaparams->~ECAPARAMS();
  remove(ecasound_lockfile.c_str());
  exit(0);
}

string print_header(void) 
{
  MESSAGE_ITEM mout;

  mout << "****************************************************************************\n";
  mout << "*                [1mecasound " << ecasound_version << " (C) 1997-1999 Kai Vehmanen[0m            *\n";
  mout << "****************************************************************************\n";
  
  return(mout.to_string());
}

void start_iactive(ECA_SESSION* param) {
  ECA_CONTROLLER ctrl(param);

  //  if (ctrl.is_connected()) ctrl.start(true);

  string cmd;
  try {
    do {
      if (cmd.size() > 0) {
	ctrl.command(cmd);
      }
      cout << "ecasound ('h' for help)>\n";
    }
    while(getline(cin,cmd));
  }
  catch(int n) {
    if (n == ECA_QUIT) 
      ecadebug->msg(1, "(eca-text) Exiting...");
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\nCaught an unknown exception!\n";
  }
}

void start_iactive_readline(ECA_SESSION* param) {
  ECA_CONTROLLER ctrl(param);

  char* cmd;
  init_readline_support();
  do {
    cmd = readline("ecasound ('h' for help)> ");
    //      cmd = readline();
    if (cmd != 0) {
      add_history(cmd);
      try {
	ctrl.command(string(cmd));
      }
      catch(int n) {
	if (n == ECA_QUIT) {
	  cerr << "---\nExiting...\n";
	  free(cmd);
	  exit(0);
	}
      }
      catch(ECA_ERROR* e) {
	cerr << "---\nERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
	if (e->error_action() == ECA_ERROR::stop) {
	  free(cmd);
	  exit(0);
	}
      }
      catch(...) {
	cerr << "---\nCaught an unknown exception!\n";
	free(cmd);
	exit(0);
      }
      free(cmd);
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
  static const map<string,int>& map_ref = ECA_IAMODE_PARSER::registered_commands();
  static map<string,int>::const_iterator p;
  static string cmd;

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
