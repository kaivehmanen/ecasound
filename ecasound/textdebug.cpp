// ------------------------------------------------------------------------
// textdebug.cpp: Implementation of console logging subsystem.
// Copyright (C) 1999-2002,2004-2005 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 2
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
#include <algorithm>

#include <eca-logger-interface.h>
#include <kvu_utils.h>

#include "textdebug.h"

#ifdef ECA_USE_NCURSES_H
#include <ncurses.h>
#include <term.h> /* for setupterm() */
#elif ECA_USE_NCURSES_NCURSES_H
#include <ncurses/ncurses.h>
#include <ncurses/term.h> /* for setupterm() */
#elif ECA_USE_CURSES_H
#include <curses.h>
#include <term.h> /* for setupterm() */
#endif

using namespace std;

/**
 * Set terminal width used in pretty-printing ecasound console output.
 *
 * Value of 79 guarantees that output is readable even in 80x25 terminal mode.
 */
const static int tb_terminal_width = 74;

/**
 * Wraps text 'msg' by adding <newline> + "... " breaks so that none 
 * of the lines exceed 'width' characteds.
 */
static string tb_wrap(const string& msg, int width, int offset)
{
  int counter = offset;
  vector<string> vec = kvu_string_to_vector(msg, ' ');
  string result;
  for(vector<string>::const_iterator p = vec.begin(); p != vec.end(); ) {
    /* check if adding the next token would go over the limit */
    if ((counter + static_cast<int>(p->size())) > width) {
      result += "\n... ";
      counter = 0;
    }
    result += *p;
    /* check if line already contains a newline */
    if (find(p->begin(), p->end(), '\n') != p->end()) {
      counter = 0;
    }
    else {
      counter += p->size();
    }
    ++p;
    if (p != vec.end()) {
      result += " ";
    }
  }
  return result;
}

void TEXTDEBUG::stream(std::ostream* dos)
{
  dostream_repp = dos;
}

std::ostream* TEXTDEBUG::stream(void)
{
  return dostream_repp;
}

void TEXTDEBUG::do_flush(void) 
{
  dostream_repp->flush();
}

void TEXTDEBUG::do_msg(ECA_LOGGER::Msg_level_t level, const std::string& module_name, const std::string& log_message)
{
  if (is_log_level_set(level) == true) {
    int offset = 0;

    if (level == ECA_LOGGER::subsystems) {
#if defined(ECA_USE_NCURSES_H) || defined(ECA_USE_NCURSES_NCURSES_H) || defined(ECA_USE_CURSES_H)
      *dostream_repp << "- [ ";
      putp(tigetstr("bold"));
      offset += 4;
#endif
    }
    else if (is_log_level_set(ECA_LOGGER::module_names) == true &&
	     level != ECA_LOGGER::eiam_return_values) {
      *dostream_repp << "(" 
		     << std::string(module_name.begin(), 
				    find(module_name.begin(), module_name.end(), '.'))
		     << ") ";
      offset += module_name.size() + 2;
    }
    
    *dostream_repp << tb_wrap(log_message, tb_terminal_width, offset);

    if (level == ECA_LOGGER::subsystems) {
#if defined(ECA_USE_NCURSES_H) || defined(ECA_USE_NCURSES_NCURSES_H) || defined(ECA_USE_CURSES_H)
      putp(tigetstr("sgr0"));
      *dostream_repp << " ] ";
#else
      *dostream_repp << " ] ";
#endif
      if (log_message.size() < static_cast<int>(tb_terminal_width)) {
	for (unsigned char n = 0; n < (tb_terminal_width - log_message.size() - 1); n++) *dostream_repp << "-";
      }
    }
  
    *dostream_repp << endl;
  }
}

TEXTDEBUG::TEXTDEBUG(void)
{
  dostream_repp = &std::cout;
}

TEXTDEBUG::~TEXTDEBUG(void)
{
  flush();
}
