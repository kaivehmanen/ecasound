// ------------------------------------------------------------------------
// eca-object-map: A virtual base for dynamic object maps 
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
#include <map>

#include "eca-object-map.h"
#include "eca-debug.h"

void ECA_OBJECT_MAP::register_object(const string& keyword, ECA_OBJECT* object) {
  object_map[keyword] = object;
  object_keyword_map[object->name()] = keyword;
}

const map<string,string>& ECA_OBJECT_MAP::registered_objects(void) const {
  return(object_keyword_map);
}

ECA_OBJECT* ECA_OBJECT_MAP::object(const string& keyword) const {
  map<string,ECA_OBJECT*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if ((p->first.size() > 0 && keyword.find(p->first) != string::npos && 
	 (p->first[0] == '.' || p->first[0] == '/')) || 
	p->first == keyword) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(e-a-o-m) match: " + p->first + "-" + keyword);
      return(p->second);
    }
    ++p;
  }
  return(0);
}

string ECA_OBJECT_MAP::object_identifier(const ECA_OBJECT* object) const {
  map<string, ECA_OBJECT*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (object->name() == p->second->name()) {
      return(p->first);
    }
    ++p;
  }
  return("");
}
