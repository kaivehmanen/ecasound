// ------------------------------------------------------------------------
// qenonblockingevent.cpp: Virtual base for nonblocking processing events
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

#include <qprogressdialog.h>
#include <ecasound/eca-error.h>

#include "qenonblockingevent.h"

void QENonblockingEvent::stop(void) {
  // --------
  REQUIRE(ectrl->is_engine_started() == true);
  REQUIRE(is_triggered() == true);
  // --------
  try {
    ectrl->stop();
    ectrl->disconnect_chainsetup();
    toggle_triggered_state(false);
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nlibecasound error while stopping event: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }

  // --------
  ENSURE(ectrl->is_connected() == false);
  ENSURE(is_triggered() == false);
  // --------
}

long int QENonblockingEvent::position_in_samples(void) const {
  if (ectrl->is_running() == true)
    return(ectrl->position_in_samples() - ectrl->get_chainsetup()->buffersize());
  return(0);
}
