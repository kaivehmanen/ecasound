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
#include "resource-file.h"
#include "eca-preset-map.h"

vector<string> ECA_PRESET_MAP::preset_map;

const vector<string>& ECA_PRESET_MAP::available_presets(void) {
  static bool defaults_registered = false;
  if (defaults_registered == false) {
    defaults_registered = true;

    ECA_RESOURCES ecarc;
    ecadebug->msg(ECA_DEBUG::system_objects,"(global-preset) Opening sc-preset file.");
    string filename =
      ecarc.resource("resource-directory") + "/" + ecarc.resource("resource-file-effect-presets");

    RESOURCE_FILE rc (filename);
    ECA_PRESET_MAP::preset_map = rc.keywords();
  }
  return(ECA_PRESET_MAP::preset_map);
}

bool ECA_PRESET_MAP::has(const string& preset_name) {
  const vector<string>& t = ECA_PRESET_MAP::available_presets();
  if (find(t.begin(), t.end(), preset_name) == t.end())
    return(false);

  return(true);
}
