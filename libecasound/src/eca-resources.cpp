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

#include <string>
#include <cstdlib>

#include "eca-resources.h"

ECA_RESOURCES::ECA_RESOURCES(void) :
  RESOURCE_FILE(string(getenv("HOME")) + "/" + ".ecasoundrc")
{ 
  set_defaults(); 
}

void ECA_RESOURCES::set_defaults(void) {
  if (resource("midi-device") == "") resource("midi-device","/dev/midi");
  if (resource("default-output") == "") resource("default-output","/dev/dsp");
  if (resource("default-buffersize") == "") resource("default-buffersize","1024");
  if (resource("default-samplerate") == "") resource("default-samplerate","44100");
  if (resource("default-to-interactive-mode") == "") resource("default-to-interactive-mode","false");
  if (resource("default-to-raisepriority") == "") resource("default-to-raisepriority","false");
  if (resource("default-to-double-buffering") == "") resource("default-to-double-buffering","false");
  if (resource("default-double-buffer-size") == "") resource("default-double-buffer-size","131072");
  if (resource("default-to-precise-sample-rates") == "") resource("default-to-precise-sample-rates","false");
  if (resource("resource-directory") == "") resource("resource-directory","/usr/local/share/ecasound");
  if (resource("resource-file-genosc-envelopes") == "") resource("resource-file-genosc-envelopes","generic_oscillators");
  if (resource("resource-file-single-effect-presets") == "") resource("resource-file-single-effect-presets","singlechain_effect_presets");
  if (resource("resource-file-multi-effect-presets") == "") resource("resource-file-multi-effect-presets","multichain_effect_presets");

  if (resource("ext-text-editor") == "") resource("ext-text-editor","pico");
  if (resource("ext-text-editor-use-getenv") == "") resource("ext-text-editor-use-getenv","true");
  if (resource("ext-wave-editor") == "") resource("ext-wave-editor","ecawave");

  if (resource("ext-mpg123-path") == "") resource("ext-mpg123-path","mpg123");
  if (resource("ext-mpg123-args") == "") resource("ext-mpg123-args","-b 0");

  if (resource("ext-lame-path") == "") resource("ext-lame-path","lame");
  if (resource("ext-lame-args") == "") resource("ext-lame-args","-b 128");

  if (resource("ext-mikmod-path") == "") resource("ext-mikmod-path","mikmod");
  if (resource("ext-mikmod-args") == "") resource("ext-mikmod-args","-p 0 --noloops");
}
