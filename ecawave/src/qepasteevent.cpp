// ------------------------------------------------------------------------
// qepasteevent.cpp: Paste clipboard contents into at the specified position
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

#include "qepasteevent.h"

QEPasteEvent::QEPasteEvent(ECA_CONTROL* ctrl,
			   const string& input,
			   const string& output,
			   long int start_pos)
  : QEBlockingEvent(ctrl),
    ectrl(ctrl) {

  status_info("Copying data from clipboard...");
  init("pasteevent", "default");
  set_input(input);
  set_input_position(0);
  set_default_audio_format(input);
  set_output(output);
  set_output_position(start_pos);
}
