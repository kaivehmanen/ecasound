// ------------------------------------------------------------------------
// eca-audio-object-map: Dynamic register for audio objects their 
//                       id-substrings
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

#include "audioio.h"
#include "eca-audio-object-map.h"

void ECA_AUDIO_OBJECT_MAP::register_object(const string& id_string,
					     AUDIO_IO* object) {
  omap.register_object(id_string, object);
}

const map<string,string>& ECA_AUDIO_OBJECT_MAP::registered_objects(void) const {
  return(omap.registered_objects());
}

AUDIO_IO* ECA_AUDIO_OBJECT_MAP::object(const string& keyword) const {
  return(dynamic_cast<AUDIO_IO*>(omap.object(keyword)));
}

string ECA_AUDIO_OBJECT_MAP::object_identifier(const AUDIO_IO* object) const {
  return(object_identifier(object));
}
