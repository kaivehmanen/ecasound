// ------------------------------------------------------------------------
// audioio.cpp: Routines common for all audio IO-devices.
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

#include <cmath>
#include <string>

#include <kvutils.h>

#include "eca-error.h"
#include "audioio.h"
#include "samplebuffer.h"

#include "eca-debug.h"

string AUDIO_IO::format_info(void) const {
  MESSAGE_ITEM otemp;

  otemp << "(audio-io) Format " << format_string();
  otemp << ", channels " << channels();
  otemp << ", srate " << samples_per_second() << ".";
  
  return(otemp.to_string());
}

string AUDIO_IO::status(void) const {
  if (is_realtime()) {
    return("realtime-device.");
  }
  else {
    MESSAGE_ITEM mitem;
    mitem.setprecision(3);
    mitem << "position (" << position_in_seconds_exact();
    mitem << "/" << length_in_seconds_exact();
    mitem << ") seconds.";
    return(mitem.to_string());
  }
}

