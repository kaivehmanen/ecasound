// ------------------------------------------------------------------------
// eca-control-interface.cpp: C++ implementation of the Ecasound
//                            Control Interface
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include "eca-session.h"
#include "eca-control.h"

#include "eca-control-interface.h"

/**
 * Class constructor.
 */
ECA_CONTROL_INTERFACE::ECA_CONTROL_INTERFACE (void) { 
  session_repp = new ECA_SESSION();
  control_repp = new ECA_CONTROL(session_repp);
}

/**
 * Desctructor.
 */
ECA_CONTROL_INTERFACE::~ECA_CONTROL_INTERFACE (void) { 
  delete session_repp;
  delete control_repp;
}

/**
 * Parse string mode command and act accordingly.
 */
void ECA_CONTROL_INTERFACE::command(const string& cmd) { 
  try {
    control_repp->command(cmd);
  }
  catch(int n) {
    cerr << "Caught an exception (1)!" << endl;
  }
  catch(ECA_ERROR& e) {
    cerr << "Caught an exception (2)!" << endl;
  }
  catch(...) {
    cerr << "Caught an exceptiong (3)!" << endl;
  }
}
