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
#include <sys/types.h>
#include <regex.h>

#include "eca-object-map.h"
#include "eca-debug.h"

/**
 * Registers a new object-regexp pair. Map object will take care
 * of deleting the registered objects. Notice that it's possible 
 * to register the same physical object with different keywords.
 * Object map will take care that objects with multiple registered
 * keywords are destructed properly.
 */
void ECA_OBJECT_MAP::register_object(const std::string& keyword, ECA_OBJECT* object) {
  object_map[keyword] = object;
  object_keyword_map[keyword] = object->name();
}

/**
 * Unregisters object with keyword 'keyword'. Does not physically
 * delete the assigned object, because one object can be 
 * registered with multiple keywords.
 */
void ECA_OBJECT_MAP::unregister_object(const std::string& keyword) {
  object_map[keyword] = 0;
  object_keyword_map[keyword] = "";
}

/**
 * List of registered objects ('regexpr'-'object name' map).
 */
const std::map<std::string,std::string>& ECA_OBJECT_MAP::registered_objects(void) const {
  return(object_keyword_map);
}

/**
 * Returns the first object that matches the expression 'expr'.
 * If 'use_regexp' is true, regex matching is used. For practical
 * reasons a non-const pointer is returned. However, in most 
 * cases the returned object should be cloned before actual use.
 * In other words, the returned pointer refers to the object
 * stored in the object map.
 */
ECA_OBJECT* ECA_OBJECT_MAP::object(const std::string& keyword, bool use_regexp) const {
  std::map<std::string,ECA_OBJECT*>::const_iterator p = object_map.begin();
  regex_t preg;
  ECA_OBJECT* object = 0;
  while(p != object_map.end()) {
    if (use_regexp == true) {
      regcomp(&preg, p->first.c_str(), REG_NOSUB | REG_ICASE);
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

/**
 * Returns the matching keyword for 'object'.
 */
std::string ECA_OBJECT_MAP::object_identifier(const ECA_OBJECT* object) const {
  std::map<std::string, ECA_OBJECT*>::const_iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (object->name() == p->second->name()) {
      return(p->first);
    }
    ++p;
  }
  return("");
}

/**
 * Unregister all objects without physically deleting them.
 */
void ECA_OBJECT_MAP::flush(void) { 
  std::map<std::string, ECA_OBJECT*>::iterator p = object_map.begin();
  while(p != object_map.end()) {
    p->second = 0;
    ++p;
  }
}

ECA_OBJECT_MAP::~ECA_OBJECT_MAP (void) { 
  std::map<std::string, ECA_OBJECT*>::iterator p = object_map.begin();
  while(p != object_map.end()) {
    if (p->second != 0) {
      ECA_OBJECT* next_obj = p->second;
//        std::cerr << "Deleting " << next_obj->name() << "." << std::endl;
      std::map<std::string, ECA_OBJECT*>::iterator q = p;
      ++q;
      while(q != object_map.end()) {
	if (q->second != 0 &&
	    q->second == p->second) {
//  	  std::cerr << "Deleting sub-object with keyword " << q->first << "." << std::endl;
	  q->second = 0;
	}
	++q;
      }
      p->second = 0;
      delete next_obj;
    }
    ++p;
  }
}

