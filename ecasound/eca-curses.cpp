// ------------------------------------------------------------------------
// eca-curses.cpp: Curses implementation of the console user interface.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <map>
#include <string>

#if defined ECA_USE_NCURSES || defined ECA_USE_TERMCAP

#ifdef ECA_HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#include <ncurses/term.h>
#else
#include <curses.h>
#include <term.h>
#endif

#define READLINE_LIBRARY
#include <readline.h>
#include <history.h>

#include <eca-iamode-parser.h>
#include <eca-version.h>
#include <kvu_utils.h> /* kvu_string_search_and_replace() */

#include "eca-curses.h"

#if RL_READLINE_VERSION >= 0x0402
static char** ecasound_completion (const char *text, int start, int end);
static char* ecasound_command_generator (const char* text, int state);
#else 
static char** ecasound_completion (char *text, int start, int end);
static char* ecasound_command_generator (char* text, int state);
#endif

using namespace std;

ECA_CURSES::ECA_CURSES(void)
{
  init_readline_support();
}

ECA_CURSES::~ECA_CURSES(void)
{
}

void ECA_CURSES::print(const std::string& msg)
{
  cout << msg << endl;
}

void ECA_CURSES::print_banner(void)
{
  cout << "****************************************************************************\n";
  cout << "*";
  setupterm((char *)0, 1, (int *)0);
  putp(tigetstr("bold"));
  cout << "               ecasound v" 
       << ecasound_library_version
       << " (C) 1997-2002 Kai Vehmanen                 ";
  putp(tigetstr("sgr0"));
  cout << "\n";
  cout << "****************************************************************************\n";
}

void ECA_CURSES::read_command(const string& prompt)
{
  last_cmdchar_repp = readline(const_cast<char*>(prompt.c_str()));
  if (last_cmdchar_repp != 0) {
    add_history(last_cmdchar_repp);
    last_cmd_rep = last_cmdchar_repp;
    free(last_cmdchar_repp);
  }
}

const string& ECA_CURSES::last_command(void) const
{
  return(last_cmd_rep);
}

void ECA_CURSES::init_readline_support(void)
{
  /* for conditional parsing of ~/.inputrc file. */
  rl_readline_name = "ecasound";

  /* we want to attempt completion first */
#if RL_READLINE_VERSION >= 0x0402
  rl_attempted_completion_function = (rl_completion_func_t*)ecasound_completion;
#else
  rl_attempted_completion_function = (CPPFunction *)ecasound_completion;
#endif
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator ();
char **fileman_completion ();

/**
 * Attempt to complete on the contents of TEXT.  START and END bound the
 * region of rl_line_buffer that contains the word to complete.  TEXT is
 * the word to complete.  We can use the entire contents of rl_line_buffer
 * in case we want to do some simple parsing.  Return the array of matches,
 * or NULL if there aren't any.
 */
#if RL_READLINE_VERSION >= 0x0402
char** ecasound_completion (const char *text, int start, int end)
#else
char** ecasound_completion (char *text, int start, int end)
#endif
{
  char **matches;
  matches = (char **)NULL;

  /* complete only the first command, otherwise complete files in 
   * the current directory */
  if (start == 0) {
#if RL_READLINE_VERSION >= 0x0402
    matches = rl_completion_matches (text, (rl_compentry_func_t *)ecasound_command_generator);
#else
    matches = completion_matches (text, (CPFunction *)ecasound_command_generator);
#endif
  }
  return (matches);
}

/**
 * Generator function for command completion.  STATE lets us know whether
 * to start from scratch; without any state (i.e. STATE == 0), then we
 * start at the top of the list.
 */
#if RL_READLINE_VERSION >= 0x0402
char* ecasound_command_generator (const char* text, int state)
#else
char* ecasound_command_generator (char* text, int state)
#endif
{
  static int list_index, len;
  static const map<string,int>& map_ref = ECA_IAMODE_PARSER::registered_commands();
  static map<string,int>::const_iterator p;
  static string cmd;

  /* If this is a new word to complete, initialize now.  This includes
   * saving the length of TEXT for efficiency, and initializing the index
   * variable to 0
   */
  if (!state) {
      list_index = 0;
      p = map_ref.begin();
      len = strlen (text);
  }

  /* Return the next name which partially matches from the command list */
  while (p != map_ref.end()) {
      cmd = p->first;
      list_index++;
      ++p;
      if (p != map_ref.end()) {
	string hyphenstr = kvu_string_search_and_replace(text, '_', '-');
	if (strncmp(hyphenstr.c_str(), cmd.c_str(), len) == 0) {
	  return(strdup(cmd.c_str()));
	}
      }
  }
  return ((char *)0);
}

#else

#include "eca-curses.h"

ECA_CURSES::ECA_CURSES(void) {}
ECA_CURSES::~ECA_CURSES(void) {}
void ECA_CURSES::print(const std::string& msg) {}
void ECA_CURSES::print_banner(void) {}
void ECA_CURSES::read_command(const string& promp) {}
const string& ECA_CURSES::last_command(void) const { static std::string empty; return(empty); }
void ECA_CURSES::init_readline_support(void) {}

#endif /* defined ECA_USE_NCURSES || defined ECA_USE_TERMCAP */
