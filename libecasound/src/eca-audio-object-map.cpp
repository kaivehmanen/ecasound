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

#include <vector>
#include <string>

#include "audioio.h"

#include "eca-audio-object-map.h"
#include "eca-debug.h"

void ECA_AUDIO_OBJECT_MAP::register_object(const string& id_string,
					   AUDIO_IO* object) {
  object->map_parameters();
  object_names.push_back(object->name());
  object_map[id_string] = object;
  object_prefix_map[object->name()] = id_string;
}

const vector<string>& ECA_AUDIO_OBJECT_MAP::registered_objects(void) const {
  return(object_names);
}

AUDIO_IO* ECA_AUDIO_OBJECT_MAP::object(const string& keyword) const {
  map<string, AUDIO_IO*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (keyword.find(p->first) != string::npos) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(e-a-o-m) match: " + p->first + "-" + keyword);
      return(p->second);
    }
    ++p;
  }
  return(0);
}

string ECA_AUDIO_OBJECT_MAP::object_identifier(const AUDIO_IO* object) const {
  assert(object != 0);
  map<string, string>::const_iterator p = object_prefix_map.begin();
  while(p != object_prefix_map.end()) {
    if (p->first == object->name())
      return(p->second);
    ++p;
  }
  return("");
}
