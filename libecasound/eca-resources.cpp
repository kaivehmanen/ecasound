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

#include "plugin-paths.h"
#include "eca-resources.h"

ECA_RESOURCES::ECA_RESOURCES(void) :
  RESOURCE_FILE(string(getenv("HOME")) + "/" + ".ecasoundrc")
{ 
  set_defaults(); 
}

ECA_RESOURCES::~ECA_RESOURCES(void) { }

void ECA_RESOURCES::set_defaults(void) {
  if (has("midi-device") != true) resource("midi-device","/dev/midi");
  if (has("default-output") != true) resource("default-output","/dev/dsp");
  if (has("default-buffersize") != true) resource("default-buffersize","1024");
  if (has("default-samplerate") != true) resource("default-samplerate","44100");
  if (has("default-to-interactive-mode") != true) resource("default-to-interactive-mode","false");
  if (has("default-to-raisepriority") != true) resource("default-to-raisepriority","false");
  if (has("default-schedpriority") != true) resource("default-schedpriority","50");
  if (has("default-to-double-buffering") != true) resource("default-to-double-buffering","false");
  if (has("default-double-buffer-size") != true) resource("default-double-buffer-size","100000");
  if (has("default-to-precise-sample-rates") != true) resource("default-to-precise-sample-rates","false");
  if (has("resource-directory") != true) resource("resource-directory", ecasound_resource_path);
  if (has("resource-file-genosc-envelopes") != true) resource("resource-file-genosc-envelopes","generic_oscillators");
  if (has("resource-file-effect-presets") != true) resource("resource-file-effect-presets","effect_presets");

  if (has("ext-text-editor") != true) resource("ext-text-editor","pico");
  if (has("ext-text-editor-use-getenv") != true) resource("ext-text-editor-use-getenv","true");
  if (has("ext-wave-editor") != true) resource("ext-wave-editor","ecawave");

  if (has("ext-mp3-input-cmd") != true) resource("ext-mp3-input-cmd","mpg123 --stereo -r %s -b 0 -q -s -k %o %f");
  if (has("ext-mp3-output-cmd") != true) resource("ext-mp3-output-cmd", "lame -b 128 -x -S - %f");

  if (has("ext-ogg-input-cmd") != true) resource("ext-ogg-input-cmd","ogg123 -d wav -o file:%F %f");
  if (has("ext-ogg-output-cmd") != true) resource("ext-ogg-output-cmd", "vorbize --raw --write=%f");

  if (has("ext-mikmod-cmd") != true) resource("ext-mikmod-cmd","mikmod -d stdout -o 16s -q -f %s -p 0 --noloops %f");
  if (has("ext-timidity-cmd") != true) resource("ext-timidity-cmd", "timidity -Or1S -id -s %s -o - %f");

  if (has("internal-plugin-directory") != true) resource("internal-plugin-directory", ecasound_plugin_path);
  if (has("ladspa-plugin-directory") != true) resource("ladspa-plugin-directory", ecasound_ladspa_path);
  save();
}
