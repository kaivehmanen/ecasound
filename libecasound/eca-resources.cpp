// ------------------------------------------------------------------------
// eca_resources.cpp: User settings (~/.ecasoundrc)
// Copyright (C) 1999-2000,2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <cstdlib>

#include "eca-resources.h"

ECA_RESOURCES::ECA_RESOURCES(void) :
  RESOURCE_FILE(std::string(getenv("HOME")) + "/" + ".ecasoundrc")
{ 
  set_defaults(); 
}

ECA_RESOURCES::~ECA_RESOURCES(void) { 
  if (is_modified() == true) {
    save();
  }
}

void ECA_RESOURCES::set_defaults(void) {

  std::string ecasound_prefix (ECASOUND_PREFIX_DIR);
  std::string ecasound_plugin_path = ecasound_prefix + "/lib/ecasound-plugins";
  std::string ecasound_resource_path = ecasound_prefix + "/share/ecasound";
  std::string ecasound_ladspa_path = ecasound_prefix + "/lib/ladspa";

  if (has("midi-device") != true) resource("midi-device","rawmidi,/dev/midi");

  if (has("default-output") != true) resource("default-output","/dev/dsp");
  if (has("default-samplerate") != true) resource("default-samplerate","44100");
  if (has("default-to-precise-sample-rates") != true) resource("default-to-precise-sample-rates","false");
  if (has("default-to-interactive-mode") != true) resource("default-to-interactive-mode","false");

  /* bmode-defaults: 1) buffersize, 2) raisedprio, 3) schedprio,
   *                 4) db, 5) db-bufsize, 6) maxintbuf,   */
  if (has("bmode-defaults-nonrt") != true) 
    resource("bmode-defaults-nonrt","1024,false,50,false,100000,true");
  if (has("bmode-defaults-rt") != true) 
    resource("bmode-defaults-rt","1024,true,50,true,100000,true");
  if (has("bmode-defaults-rtlowlatency") != true) 
    resource("bmode-defaults-rtlowlatency","256,true,50,true,100000,false");

  if (has("resource-directory") != true) resource("resource-directory", ecasound_resource_path);
  if (has("resource-file-genosc-envelopes") != true) resource("resource-file-genosc-envelopes","generic_oscillators");
  if (has("resource-file-effect-presets") != true) resource("resource-file-effect-presets","effect_presets");
  if (has("user-resource-directory") != true) resource("user-resource-directory", 
						       std::string(getenv("HOME")) + "/" + "ecasound-config");

  if (has("ext-text-editor") != true) resource("ext-text-editor","pico");
  if (has("ext-text-editor-use-getenv") != true) resource("ext-text-editor-use-getenv","true");
  if (has("ext-wave-editor") != true) resource("ext-wave-editor","ecawave");

  if (has("ext-mp3-input-cmd") != true) resource("ext-mp3-input-cmd","mpg123 --stereo -r %s -b 0 -q -s -k %o %f");
  if (has("ext-mp3-output-cmd") != true) resource("ext-mp3-output-cmd", "lame -b 128 -s %S -x -S - %f");

  if (has("ext-ogg-input-cmd") != true) resource("ext-ogg-input-cmd","ogg123 -d raw --file=%F %f");
  if (has("ext-ogg-output-cmd") != true) resource("ext-ogg-output-cmd", "oggenc -b 128 --raw --raw-bits=%b --raw-chan=%c --raw-rate=%s --output=%f -");

  if (has("ext-mikmod-cmd") != true) resource("ext-mikmod-cmd","mikmod -d stdout -o 16s -q -f %s -p 0 --noloops %f");
  if (has("ext-timidity-cmd") != true) resource("ext-timidity-cmd", "timidity -Or1S -id -s %s -o - %f");

  if (has("internal-plugin-directory") != true) resource("internal-plugin-directory", ecasound_plugin_path);
  if (has("ladspa-plugin-directory") != true) resource("ladspa-plugin-directory", ecasound_ladspa_path);
}
