// ------------------------------------------------------------------------
// qtdebug_if.cpp: Qt-interface to ecasound debug-routines.
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

#include <iostream>
#include <string>
#include <iomanip>
#include <kvutils.h>

#include <eca-debug.h>

#include "qtdebug_if.h"

COMMAND_QUEUE qtdebug_queue;

void QTDEBUG_IF::set_debug_level(int level) {
    debug_level = level;
}

void QTDEBUG_IF::control_flow(const string& part) {
  if (is_enabled() == false) return; 
   MESSAGE_ITEM m;
   m << "--- [ <b> " << part << "</b> ] ---";
   //   for (unsigned char n = 0; n < (69 - part.size()); n++)
   //     m << '-';
   qtdebug_queue.push_back(m.to_string());
}

void QTDEBUG_IF::msg(const string& info) {
  if (is_enabled() == false) return;
  MESSAGE_ITEM m;
  m << info;
    qtdebug_queue.push_back(m.to_string());
}

void QTDEBUG_IF::msg(int level, const string& info) {
  if (is_enabled() == false) return;
  if (debug_level < level) return;
  MESSAGE_ITEM m;
  if (debug_level != 0) m << "DEBUG: ";
  m << info;
  qtdebug_queue.push_back(m.to_string());
}

QTDEBUG_IF::QTDEBUG_IF(void) {
  enable();
  debug_level = 0;
}











