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

#include <vector>
#include <string>

#include "eca-controller-map.h"
#include "generic-controller.h"
#include "ctrl-source.h"
#include "midi-cc.h"
#include "osc-gen.h"
#include "osc-sine.h"
#include "linear-envelope.h"
#include "two-stage-linear-envelope.h"

map<string, GENERIC_CONTROLLER*> ECA_CONTROLLER_MAP::object_map;
map<string, string> ECA_CONTROLLER_MAP::object_prefix_map;

void ECA_CONTROLLER_MAP::register_object(const string& id_string,
					 GENERIC_CONTROLLER* object) {
  object->map_parameters();
  object_map[id_string] = object;
  object_prefix_map[object->name()] = id_string;
}

void ECA_CONTROLLER_MAP::register_default_objects(void) { 
  static bool defaults_registered = false;
  if (defaults_registered) return;
  defaults_registered = true;

  register_object("kf", new GENERIC_CONTROLLER(new GENERIC_OSCILLATOR()));
  register_object("kl", new GENERIC_CONTROLLER(new LINEAR_ENVELOPE()));
  register_object("kl2", new GENERIC_CONTROLLER(new TWO_STAGE_LINEAR_ENVELOPE()));
  register_object("km", new GENERIC_CONTROLLER(new MIDI_CONTROLLER()));
  register_object("kos", new GENERIC_CONTROLLER(new SINE_OSCILLATOR()));
}

