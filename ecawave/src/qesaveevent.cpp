// ------------------------------------------------------------------------
// qesaveevent.cpp: Copy file to another file
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

#include <ecasound/eca-debug.h>

#include "qesaveevent.h"

QESaveEvent::QESaveEvent(ECA_CONTROLLER* ctrl,
			 const string& input,
			 const string& output)
  : QEBlockingEvent(ctrl),
    ectrl(ctrl) {

  status_info("Saving file...");
  init("saveevent");
  ectrl->add_chain("default");
  set_input(input);
  set_default_audio_format(input);
  if (input != output)
    ectrl->set_chainsetup_parameter("-x");
  set_output(output);
}
