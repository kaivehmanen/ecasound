// ------------------------------------------------------------------------
// eca-preset-map: Dynamic register for storing effect presets
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
#include <algorithm>

#include "eca-preset-map.h"

void ECA_PRESET_MAP::register_object(const string& id_string,
					     PRESET* object) {
  object->map_parameters();
  object_names.push_back(object->name());
  object_map[id_string] = object;
  object_prefix_map[object->name()] = id_string;
}

const vector<string>& ECA_PRESET_MAP::registered_objects(void) const {
  return(object_names);
}

PRESET* ECA_PRESET_MAP::object(const string& keyword) const {
  map<string, PRESET*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (p->first == keyword) 
      return(dynamic_cast<PRESET*>(p->second));
    ++p;
  }
  return(0);
}

string ECA_PRESET_MAP::object_identifier(const PRESET* object) const {
  assert(object != 0);
  map<string, string>::const_iterator p = object_prefix_map.begin();
  while(p != object_prefix_map.end()) {
    if (p->first == object->name())
      return(p->second);
    ++p;
  }
  return("");
}

