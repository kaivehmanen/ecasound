// ------------------------------------------------------------------------
// eca-controller-map.h: Dynamic register for controllers
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
  omap.register_object(id_string, object);
}

const map<string,string>& ECA_CONTROLLER_MAP::registered_objects(void) const {
  return(omap.registered_objects());
}

GENERIC_CONTROLLER* ECA_CONTROLLER_MAP::object(const string& keyword) const {
  return(dynamic_cast<GENERIC_CONTROLLER*>(omap.object(keyword)));
}

string ECA_CONTROLLER_MAP::object_identifier(const GENERIC_CONTROLLER* object) const {
  return(omap.object_identifier(object));
}
