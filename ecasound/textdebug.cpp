// ------------------------------------------------------------------------
// textdebug.cpp: Implementation of console logging subsystem.
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
#include <string>

#include <eca-logger-interface.h>
#include "textdebug.h"

#if defined ECA_USE_NCURSES || defined ECA_USE_TERMCAP
#ifdef ECA_HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#include <ncurses/term.h>
#else
#include <curses.h>
#include <term.h>
#endif
#endif

using namespace std;

void TEXTDEBUG::stream(std::ostream* dos)
{
  dostream_repp = dos;
}

std::ostream* TEXTDEBUG::stream(void) { 
  return(dostream_repp); 
}

void TEXTDEBUG::do_flush(void) 
{
  dostream_repp->flush();
}

void TEXTDEBUG::do_msg(ECA_LOGGER::Msg_level_t level, const std::string& module_name, const std::string& log_message)
{
  if (is_log_level_set(level) == true) {
    if (level == ECA_LOGGER::subsystems) {
#if defined ECA_USE_NCURSES || defined ECA_USE_TERMCAP
      *dostream_repp << "- [ ";
      putp(tigetstr("bold"));
#endif
    }

    if (is_log_level_set(ECA_LOGGER::module_names) == true) {
      *dostream_repp << module_name << ": ";
    }
    
    *dostream_repp << log_message;

    if (level == ECA_LOGGER::subsystems) {
#if defined ECA_USE_NCURSES || defined ECA_USE_TERMCAP
      putp(tigetstr("sgr0"));
      *dostream_repp << " ] ";
#else
      *dostream_repp << " ] ";
#endif
      if (log_message.size() < 70) {
	for (unsigned char n = 0; n < (69 - log_message.size()); n++) *dostream_repp << "-";
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
