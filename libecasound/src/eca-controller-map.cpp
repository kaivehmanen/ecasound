// ------------------------------------------------------------------------
// eca-chainop-map: Dynamic register for chain operators
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

#include "eca-controller-map.h"

void ECA_CONTROLLER_MAP::register_object(const string& id_string,
					 GENERIC_CONTROLLER* object) {
  object->map_parameters();
  object_names.push_back(object->name());
  object_map[id_string] = object;
  object_prefix_map[object->name()] = id_string;
}

const vector<string>& ECA_CONTROLLER_MAP::registered_objects(void) const {
  return(object_names);
}

GENERIC_CONTROLLER* ECA_CONTROLLER_MAP::object(const string& keyword) const {
  map<string, GENERIC_CONTROLLER*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (p->first == keyword) 
      return(dynamic_cast<GENERIC_CONTROLLER*>(p->second));
    ++p;
  }
  return(0);
}

string ECA_CONTROLLER_MAP::object_identifier(const GENERIC_CONTROLLER* object) const {
  assert(object != 0);
  map<string, string>::const_iterator p = object_prefix_map.begin();
  while(p != object_prefix_map.end()) {
    if (p->first == object->name())
      return(p->second);
    ++p;
  }
  return("");
}
