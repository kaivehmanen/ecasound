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

#include "eca-resources.h"
#include "eca-preset-map.h"
#include "global-preset.h"

ECA_PRESET_MAP::ECA_PRESET_MAP(void) {
  ECA_RESOURCES ecarc;
  string filename =
    ecarc.resource("resource-directory") + "/" + ecarc.resource("resource-file-effect-presets");

  RESOURCE_FILE preset_file;
  preset_file.resource_file(filename);
  preset_file.load();
  const vector<string>& pmap = preset_file.keywords();
  vector<string>::const_iterator p = pmap.begin();
  while(p != pmap.end()) {
    object_keyword_map[*p] = *p;
    ++p;
  }
}

void ECA_PRESET_MAP::register_object(const string& keyword,
				     PRESET* object) {
  object_map[keyword] = object;
  object_keyword_map[keyword] = object->name();
}

void ECA_PRESET_MAP::unregister_object(const string& keyword) {
  object_map[keyword] = 0;
  object_keyword_map[keyword] = "";
}

const map<string,string>& ECA_PRESET_MAP::registered_objects(void) const {
  return(object_keyword_map);
}

/**
 * Returns the first object that matches 'keyword'. Regular 
 * expressions are not used. For practical reasons a non-const 
 * pointer is returned. However, in most cases the returned 
 * object should be cloned before actual use. In other words, 
 * the returned pointer refers to the object stored in the object 
 * map.
 */
ECA_OBJECT* ECA_PRESET_MAP::object(const string& keyword, bool use_regex) const {
  map<string, PRESET*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (p->first == keyword) 
      return(dynamic_cast<PRESET*>(p->second));
    ++p;
  }
  PRESET* gp = 0;
  try {
    gp = dynamic_cast<PRESET*>(new GLOBAL_PRESET(keyword));
    if (gp != 0) {
      object_map[keyword] = gp;
      object_keyword_map[keyword] = gp->name();
    }
  }
  catch(...) { gp = 0; }
  return(gp);
}

string ECA_PRESET_MAP::object_identifier(const PRESET* object) const {
  assert(object != 0);
  return(object->name());
}

void ECA_PRESET_MAP::flush(void) { 
  map<string, PRESET*>::iterator p = object_map.begin();
  while(p != object_map.end()) {
    p->second = 0;
    ++p;
  }
}

ECA_PRESET_MAP::~ECA_PRESET_MAP (void) { 
  map<string, PRESET*>::iterator p = object_map.begin();
  while(p != object_map.end()) {
    delete p->second;
    p->second = 0;
    ++p;
  }
}
