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
#include <regex.h>

#include "eca-object-map.h"
#include "eca-debug.h"

void ECA_OBJECT_MAP::register_object(const string& keyword, ECA_OBJECT* object) {
  object_map[keyword] = object;
  object_keyword_map[keyword] = object->name();
}

const map<string,string>& ECA_OBJECT_MAP::registered_objects(void) const {
  return(object_keyword_map);
}

ECA_OBJECT* ECA_OBJECT_MAP::object(const string& keyword, bool use_regexp) const {
  map<string,ECA_OBJECT*>::const_iterator p = object_map.begin();
  regex_t preg;
  ECA_OBJECT* object = 0;
  while(p != object_map.end()) {
    if (use_regexp == true) {
      regcomp(&preg, p->first.c_str(), REG_NOSUB);
      if (regexec(&preg, keyword.c_str(), 0, 0, 0) == 0) {
	ecadebug->msg(ECA_DEBUG::system_objects, "(eca-object-map) match: " + p->first + "-" + keyword);
	object = p->second;
      }
      regfree(&preg);
    }
    else if (p->first == keyword) {
      object = p->second;
    }
    ++p;
  }
  return(object);
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

ECA_OBJECT_MAP::~ECA_OBJECT_MAP (void) { }
