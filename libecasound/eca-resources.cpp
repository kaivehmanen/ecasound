// ------------------------------------------------------------------------
// eca_resources.cpp: User settings (~/.ecasoundrc)
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include "eca-debug.h"
#include "eca-resources.h"
#include "eca-version.h"

ECA_RESOURCES::ECA_RESOURCES(void) { 
  std::string ecasound_prefix (ECA_PREFIX);
  std::string ecasound_resource_path = ecasound_prefix + "/share/ecasound";
  
  globalrc_rep.resource_file(ecasound_resource_path + "/ecasoundrc");
  globalrc_rep.load();
  if (globalrc_rep.keywords().size() == 0) {
    ecadebug->msg(ECA_DEBUG::info, "(eca-resources) Warning! Global resource file '" + ecasound_resource_path + "/ecasoundrc" + "' not available! Ecasound may not function properly!");
  }

  char* home_dir = getenv("HOME");
  if (home_dir != NULL) {
    std::string user_ecasoundrc_path = std::string(home_dir) + "/.ecasound";

    user_resource_directory_rep = user_ecasoundrc_path;

    userrc_rep.resource_file(user_ecasoundrc_path + "/ecasoundrc");
    userrc_rep.load();
    if (userrc_rep.has("user-resource-directory") == true) {
      ecadebug->msg(ECA_DEBUG::info, "(eca-resources) Warning! Old resource data found in '" + user_ecasoundrc_path + ". You can reset configuration parameters by removing the old rc-file.");
    }
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

void ECA_RESOURCES::set_defaults(void)
{
  /**
   * FIXME: defaults now set in '(topsrcdir)/ecasoundrc.in';
   *        remove this block once the new system has 
   *        been tested sufficiently
   */
}
