// ------------------------------------------------------------------------
// eca-controller-dump.cpp: Class for dumping status information to 
//                          a standard output stream
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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <kvutils.h>

#include "audioio.h"
#include "eca-chainop.h"
#include "eca-controller-dump.h"

void ECA_CONTROLLER_DUMP::dump_status(void) { 
  dump("dump-status", engine_status());
}

void ECA_CONTROLLER_DUMP::dump_position(void) { 
  dump("dump-position", kvu_numtostr(position_in_seconds_exact()));
}

void ECA_CONTROLLER_DUMP::dump_length(void) { 
  dump("dump-length", kvu_numtostr(length_in_seconds_exact()));
}

void ECA_CONTROLLER_DUMP::dump_chainsetup_status(void) { 
  if (is_connected() == true) 
    dump("dump-cs-status", "connected");
  else if (is_selected() == true) 
    dump("dump-cs-status", "selected");
  else
    dump("dump-cs-status", "");
}

void ECA_CONTROLLER_DUMP::dump_selected_chain(void) { 
  const vector<string>& t = selected_chains();
  if (t.empty() == false) {
    dump("dump-c-selected", vector_to_string(t, ","));
  }
  else
    dump("dump-c-selected", "");
}

void ECA_CONTROLLER_DUMP::dump_selected_audio_object(void) { 
  AUDIO_IO* t = get_audio_object();
  if (t != 0) {
    dump("dump-aio-selected", t->label());
  }
  else
    dump("dump-aio-selected", "");
}

void ECA_CONTROLLER_DUMP::dump_audio_object_position(void) { 
  AUDIO_IO* t = get_audio_object();
  if (t != 0) {
    dump("dump-aio-position", kvu_numtostr(t->position_in_seconds_exact()));
  }
  else
    dump("dump-aio-position", "");
}

void ECA_CONTROLLER_DUMP::dump_audio_object_length(void) { 
  AUDIO_IO* t = get_audio_object();
  if (t != 0) {
    dump("dump-aio-length", kvu_numtostr(t->length_in_seconds_exact()));
  }
  else
    dump("dump-aio-length", "");
}

void ECA_CONTROLLER_DUMP::dump_audio_object_open_state(void) { 
  AUDIO_IO* t = get_audio_object();
  if (t != 0) {
    if (t->is_open() == true) 
      dump("dump-aio-open-state", "open");
    else
      dump("dump-aio-open-state", "closed");
  }
  else
    dump("dump-aio-open-state", "");
}

void ECA_CONTROLLER_DUMP::dump_chain_operator_value(int chainop, int param) { 
  CHAIN_OPERATOR* t = get_chain_operator(chainop);
  if (t != 0) {
    dump("dump-cop-value", kvu_numtostr(t->get_parameter(param)));
  }
  else
    dump("dump-cop-value", "");
}
