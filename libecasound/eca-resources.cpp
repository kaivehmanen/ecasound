// ------------------------------------------------------------------------
// eca_resources.cpp: User settings (ecasoundrc)
// Copyright (C) 1999-2002,2004,2005 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
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

#include "kvu_dbc.h"

#include "eca-logger.h"
#include "eca-resources.h"
#include "eca-version.h"
#include "resource-file.h"

using std::string;

ECA_RESOURCES::ECA_RESOURCES(void) 
  : resources_found_rep(true)
{
  string ecasound_datadir (ECA_DATADIR);
  string ecasound_resource_path = ecasound_datadir + "/ecasound";

  globalrc_repp = new RESOURCE_FILE();
  globalrc_repp->resource_file(ecasound_resource_path + "/ecasoundrc");
  globalrc_repp->load();
  if (globalrc_repp->keywords().size() == 0) {
    ECA_LOG_MSG(ECA_LOGGER::info, "WARNING: Global resource file '" + ecasound_resource_path + "/ecasoundrc" + "' not available! Ecasound may not function properly!");
    resources_found_rep = false;
  }

  userrc_repp = new RESOURCE_FILE();
  char* home_dir = getenv("HOME");
  if (home_dir != NULL) {
    string user_ecasoundrc_path = string(home_dir) + "/.ecasound";

    user_resource_directory_rep = user_ecasoundrc_path;

    userrc_repp->resource_file(user_ecasoundrc_path + "/ecasoundrc");
    userrc_repp->load();
    if (userrc_repp->has("user-resource-directory") == true) {
      ECA_LOG_MSG(ECA_LOGGER::info, "WARNING: Old resource data found in '" + user_ecasoundrc_path + ". You can reset configuration parameters by removing the old rc-file.");
    }
  }
}

ECA_RESOURCES::~ECA_RESOURCES(void)
{
  if (userrc_repp->is_modified() == true) {
    userrc_repp->resource("ecasound-version", ecasound_library_version);
    userrc_repp->save();
  }

  DBC_CHECK(globalrc_repp != 0);  
  delete globalrc_repp;
  globalrc_repp = 0;

  DBC_CHECK(userrc_repp != 0);
  delete userrc_repp;
  userrc_repp = 0;
}

/**
 * Set resource 'tag' value to 'value'. If value wasn't 
 * previously defined, it's added.
 */
void ECA_RESOURCES::resource(const string& tag, const string& value)
{
  userrc_repp->resource(tag, value);
}

/**
 * Returns value of resource 'tag'. Priority is given
 * to user-specified resources over global resources.
 */
string ECA_RESOURCES::resource(const string& tag) const
{
  if (tag == "user-resource-directory") 
    return user_resource_directory_rep;
  
  if (userrc_repp->has(tag))
    return userrc_repp->resource(tag);
  
  if (globalrc_repp->has(tag))
    return globalrc_repp->resource(tag);

  return "";
}

/**
 * Returns true if resource 'tag' is 'true', otherwise false
 */
bool ECA_RESOURCES::boolean_resource(const string& tag) const
{
  if (userrc_repp->has(tag)) {
    return userrc_repp->boolean_resource(tag);
  }
  else if (globalrc_repp->has(tag)) {
    return globalrc_repp->boolean_resource(tag);
  }

  return false;
}
  
/**
 * Whether resource 'tag' is specified in the resource file
 */
bool ECA_RESOURCES::has(const string& tag) const
{
  if (globalrc_repp->has(tag) || userrc_repp->has(tag)) return true;
  return false;
}

/**
 * Are any resource values available?
 */
bool ECA_RESOURCES::has_any(void) const
{
  return resources_found_rep;
}
