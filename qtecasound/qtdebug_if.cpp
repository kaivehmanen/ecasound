// ------------------------------------------------------------------------
// qtdebug_if.cpp: Qt-interface to ecasound debug-routines.
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

#include <string>
#include <eca-debug.h>

#include "qtdebug_if.h"

COMMAND_QUEUE qtdebug_queue;

void QTDEBUG_IF::control_flow(const string& part) {
  if ((get_debug_level() & ECA_DEBUG::module_flow) != ECA_DEBUG::module_flow) return;
   qtdebug_queue.push_back("--- [ <b> " + part + "</b> ] ---");
}

void QTDEBUG_IF::msg(int level, const string& info) {
  if ((get_debug_level() & level) != level) return;
  qtdebug_queue.push_back(info);
}

QTDEBUG_IF::QTDEBUG_IF(void) { }
QTDEBUG_IF::~QTDEBUG_IF(void) { 
  if (get_debug_level() > 0) {
    while(qtdebug_queue.cmds_available() == true) {
      //      cerr << qtdebug_queue.front() << "\n";
      qtdebug_queue.pop_front();
    }
  }
}

