// ------------------------------------------------------------------------
// eca_resources.cpp: User settings (~/.ecasoundrc)
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

#include <map>
#include <string>
#include <cstdlib>

#include "eca-resources.h"

ECA_RESOURCES::ECA_RESOURCES(void) { 
  set_resource_file(string(getenv("HOME")) + "/" + ".ecasoundrc");
  set_defaults(); 
}

void ECA_RESOURCES::set_defaults(void) {
  resource("midi-device","/dev/midi");
  resource("default-output","/dev/dsp");
  resource("default-buffersize","1024");
  resource("default-samplerate","44100");
  resource("default-to-interactive-mode","false");
  resource("default-to-raisepriority","false");
  resource("default-to-double-buffering","false");
  resource("default-double-buffer-size","131072");
  resource("default-to-precise-sample-rates","false");
  resource("resource-directory","/usr/local/share/ecasound");
  resource("resource-file-genosc-envelopes","generic_oscillators");
  resource("resource-file-single-effect-presets","singlechain_effect_presets");
  resource("resource-file-multi-effect-presets","multichain_effect_presets");

  resource("ext-text-editor","pico");
  resource("ext-text-editor-use-getenv","true");
  resource("ext-wave-editor","snd");

  resource("ext-mpg123-path","mpg123");
  resource("ext-mpg123-args","-b 0");

  resource("ext-lame-path","lame");
  resource("ext-lame-args","-b 128");

  resource("ext-mikmod-path","mikmod");
  resource("ext-mikmod-args","-p 0 --noloops");

  set_modified_state(false);
}




