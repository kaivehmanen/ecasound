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

#include "eca-chainop.h"
#include "audiofx.h"
#include "audiofx_amplitude.h"
#include "audiofx_analysis.h"
#include "audiofx_filter.h"
#include "audiofx_rcfilter.h"
#include "audiofx_mixing.h"
#include "audiofx_timebased.h"
#include "audiogate.h"

#include "eca-chainop-map.h"

map<string, CHAIN_OPERATOR*> ECA_CHAIN_OPERATOR_MAP::object_map;
map<string, string> ECA_CHAIN_OPERATOR_MAP::object_prefix_map;

void ECA_CHAIN_OPERATOR_MAP::register_object(const string& id_string,
				      CHAIN_OPERATOR* object) {
  object->map_parameters();
  object_map[id_string] = object;
  object_prefix_map[object->name()] = id_string;
}

void ECA_CHAIN_OPERATOR_MAP::register_default_objects(void) { 
  static bool defaults_registered = false;
  if (defaults_registered) return;
  defaults_registered = true;

  register_object("ea", new EFFECT_AMPLIFY());
  register_object("eac", new EFFECT_AMPLIFY_CHANNEL());
  register_object("eaw", new EFFECT_AMPLIFY_CLIPCOUNT());
  register_object("ec", new EFFECT_COMPRESS());
  register_object("eca", new ADVANCED_COMPRESSOR());
  register_object("ef1", new EFFECT_RESONANT_BANDPASS());
  register_object("ef3", new EFFECT_RESONANT_LOWPASS());
  register_object("ef4", new EFFECT_RC_LOWPASS_FILTER());
  register_object("efa", new EFFECT_ALLPASS_FILTER());
  register_object("efb", new EFFECT_BANDPASS());
  register_object("efc", new EFFECT_COMB_FILTER());
  register_object("efh", new EFFECT_HIGHPASS());
  register_object("efi", new EFFECT_INVERSE_COMB_FILTER());
  register_object("efl", new EFFECT_LOWPASS());
  register_object("efr", new EFFECT_BANDREJECT());
  register_object("efs", new EFFECT_RESONATOR());
  register_object("ei", new EFFECT_PITCH_SHIFT());
  register_object("enm", new EFFECT_NOISEGATE());
  register_object("epp", new EFFECT_NORMAL_PAN());
  register_object("erc", new EFFECT_CHANNEL_COPY());
  register_object("erm", new EFFECT_MIX_TO_CHANNEL());
  register_object("etd", new EFFECT_DELAY());
  register_object("etf", new EFFECT_FAKE_STEREO());
  register_object("etl", new EFFECT_FLANGER());
  register_object("etm", new EFFECT_MULTITAP_DELAY());
  register_object("etr", new EFFECT_REVERB());
  register_object("ev", new EFFECT_ANALYZE());
  register_object("ezf", new EFFECT_DCFIND());
  register_object("ezx", new EFFECT_DCFIX());
  register_object("gc", new TIME_CROP_GATE());
  register_object("ge", new THRESHOLD_GATE());
}
