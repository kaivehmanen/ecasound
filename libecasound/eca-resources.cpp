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
#include <cstdlib> /* getenv */

#include "eca-resources.h"
#include "eca-version.h"

ECA_RESOURCES::ECA_RESOURCES(void) { 
  std::string ecasound_prefix (ECASOUND_PREFIX_DIR);
  std::string ecasound_resource_path = ecasound_prefix + "/share/ecasound";
  
  globalrc_rep.resource_file(ecasound_resource_path + "/ecasoundrc");
  globalrc_rep.load();

  char* home_dir = getenv("HOME");
  if (home_dir != NULL) {
    std::string user_ecasoundrc_path = std::string(home_dir) + "/";

    user_resource_directory_rep = user_ecasoundrc_path + "ecasound-conf";

    userrc_rep.resource_file(user_ecasoundrc_path + ".ecasoundrc");
    userrc_rep.load();
  }

  set_defaults();
}

ECA_RESOURCES::~ECA_RESOURCES(void) { 
  if (userrc_rep.is_modified() == true) {
    userrc_rep.resource("ecasound-version", ECASOUND_LIBRARY_VERSION);
    userrc_rep.save();
  }
}

/**
 * Set resource 'tag' value to 'value'. If value wasn't 
 * previously defined, it's added.
 */
void ECA_RESOURCES::resource(const std::string& tag, const std::string& value) {
  userrc_rep.resource(tag, value);
}

/**
 * Returns value of resource 'tag'. Priority is given
 * to user-specified resources over global resources.
 */
std::string ECA_RESOURCES::resource(const std::string& tag) const {
  if (tag == "user-resource-directory") 
    return(user_resource_directory_rep);
  
  if (userrc_rep.has(tag))
    return(userrc_rep.resource(tag));
  
  if (globalrc_rep.has(tag))
    return(globalrc_rep.resource(tag));

  return("");
}

/**
 * Returns true if resource 'tag' is 'true', otherwise false
 */
bool ECA_RESOURCES::boolean_resource(const std::string& tag) const { 
  if (userrc_rep.has(tag)) {
    return(userrc_rep.boolean_resource(tag));
  }
  else if (globalrc_rep.has(tag)) {
    return(globalrc_rep.boolean_resource(tag));
  }

  return(false);
}
  
/**
 * Whether resource 'tag' is specified in the resource file
 */
bool ECA_RESOURCES::has(const std::string& tag) const {
  if (globalrc_rep.has(tag) || userrc_rep.has(tag)) return(true);
  return(false);
}

void ECA_RESOURCES::set_defaults(void) {

  /**
   * FIXME: defaults now set in '(topsrcdir)/ecasoundrc.in';
   *        remove this block once the new system has 
   *        been tested sufficiently
   *
   * --> big comment start

  std::string ecasound_prefix (ECASOUND_PREFIX_DIR);
  std::string ecasound_plugin_path = ecasound_prefix + "/lib/ecasound-plugins";
  std::string ecasound_resource_path = ecasound_prefix + "/share/ecasound";
  std::string ecasound_ladspa_path = ecasound_prefix + "/lib/ladspa";

  if (globalrc_rep.has("midi-device") != true) resource("midi-device","rawmidi,/dev/midi");

  if (globalrc_rep.has("default-output") != true) resource("default-output","/dev/dsp");
  if (globalrc_rep.has("default-samplerate") != true) resource("default-samplerate","44100");
  if (globalrc_rep.has("default-to-precise-sample-rates") != true) resource("default-to-precise-sample-rates","false");
  if (globalrc_rep.has("default-to-interactive-mode") != true) resource("default-to-interactive-mode","false");

  // bmode-defaults: 1) buffersize, 2) raisedprio, 3) schedprio, 4) db, 5) db-bufsize, 6) maxintbuf,
  if (globalrc_rep.has("bmode-defaults-nonrt") != true) 
    resource("bmode-defaults-nonrt","1024,false,50,false,100000,true");
  if (globalrc_rep.has("bmode-defaults-rt") != true) 
    resource("bmode-defaults-rt","1024,true,50,true,100000,true");
  if (globalrc_rep.has("bmode-defaults-rtlowlatency") != true) 
    resource("bmode-defaults-rtlowlatency","256,true,50,true,100000,false");

  if (globalrc_rep.has("resource-directory") != true) resource("resource-directory", ecasound_resource_path);
  if (globalrc_rep.has("resource-file-genosc-envelopes") != true) resource("resource-file-genosc-envelopes","generic_oscillators");
  if (globalrc_rep.has("resource-file-effect-presets") != true) resource("resource-file-effect-presets","effect_presets");
  // if (globalrc_rep.has("user-resource-directory") != true) resource("user-resource-directory", std::string(getenv("HOME")) + "/" + "ecasound-config");

  if (globalrc_rep.has("ext-cmd-text-editor") != true) resource("ext-text-editor","pico");
  if (globalrc_rep.has("ext-cmd-text-editor-use-getenv") != true) resource("ext-text-editor-use-getenv","true");
  if (globalrc_rep.has("ext-cmd-wave-editor") != true) resource("ext-wave-editor","ecawave");

  if (globalrc_rep.has("ext-cmd-mp3-input-cmd") != true) resource("ext-mp3-input-cmd","mpg123 --stereo -r %s -b 0 -q -s -k %o %f");
  if (globalrc_rep.has("ext-cmd-mp3-output-cmd") != true) resource("ext-mp3-output-cmd", "lame -b 128 -s %S -x -S - %f");

  if (globalrc_rep.has("ext-cmd-ogg-input-cmd") != true) resource("ext-ogg-input-cmd","ogg123 -d raw --file=%F %f");
  if (globalrc_rep.has("ext-cmd-ogg-output-cmd") != true) resource("ext-ogg-output-cmd", "oggenc -b 128 --raw --raw-bits=%b --raw-chan=%c --raw-rate=%s --output=%f -");

  if (globalrc_rep.has("ext-cmd-mikmod-cmd") != true) resource("ext-mikmod-cmd","mikmod -d stdout -o 16s -q -f %s -p 0 --noloops %f");
  if (globalrc_rep.has("ext-cmd-timidity-cmd") != true) resource("ext-timidity-cmd", "timidity -Or1S -id -s %s -o - %f");

  if (globalrc_rep.has("internal-plugin-directory") != true) resource("internal-plugin-directory", ecasound_plugin_path);
  if (globalrc_rep.has("ladspa-plugin-directory") != true) resource("ladspa-plugin-directory", ecasound_ladspa_path);

  * <-- big comment ends
  */ 
}
