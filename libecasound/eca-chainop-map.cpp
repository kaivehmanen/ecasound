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

#include <string>

#include "eca-chainop-map.h"
#include "eca-static-object-maps.h"

void ECA_CHAIN_OPERATOR_MAP::register_object(const string& id_string,
					     CHAIN_OPERATOR* object) {
  eca_chain_operator_map.register_object(id_string, object);
}

const map<string,string>& ECA_CHAIN_OPERATOR_MAP::registered_objects(void) {
  return(eca_chain_operator_map.registered_objects());
}

CHAIN_OPERATOR* ECA_CHAIN_OPERATOR_MAP::object(const string& keyword) {
  return(dynamic_cast<CHAIN_OPERATOR*>(eca_chain_operator_map.object(keyword)));
}

string ECA_CHAIN_OPERATOR_MAP::object_identifier(const CHAIN_OPERATOR* object) {
  return(eca_chain_operator_map.object_identifier(object));
}
