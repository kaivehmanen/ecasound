// ------------------------------------------------------------------------
// eca-logger-interface.cpp: Logging subsystem interface
// Copyright (C) 2002-2004 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 2
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

#include <algorithm> /* find() */
#include <list>
#include <string>

#include <kvu_dbc.h>

#include "eca-logger-interface.h"

using namespace std;

const static int eca_l_i_default_log_history_len = 5;

/**
 * Class constructor. Initializes log level to 'disabled'.
 */
ECA_LOGGER_INTERFACE::ECA_LOGGER_INTERFACE(void) 
  : debug_value_rep(0),
    log_history_len_rep(eca_l_i_default_log_history_len)
{
}

/**
 * Class destructor.
 */
ECA_LOGGER_INTERFACE::~ECA_LOGGER_INTERFACE(void)
{
}

/**
 * Issues a generic log message.
 */
void ECA_LOGGER_INTERFACE::msg(ECA_LOGGER::Msg_level_t level, const std::string& module_name, const std::string& log_message)
{
  do_msg(level, module_name, log_message);
  /* note that we cannot archive EIAM return values as this 
   * could create a loop when the backlog itself is printed */
  if (level != ECA_LOGGER::eiam_return_values &&
      log_history_len_rep > 0) {
    log_history_rep.push_back(string("[") + ECA_LOGGER::level_to_string(level) + "] ("
			      + std::string(module_name.begin(), 
					    find(module_name.begin(), module_name.end(), '.'))
			      + ") " + log_message);
    if (static_cast<int>(log_history_rep.size()) > log_history_len_rep) {
      log_history_rep.pop_front();
      DBC_CHECK(static_cast<int>(log_history_rep.size()) == log_history_len_rep);
    }
  }
}

/**
 * Sets logging level to 'level' state to 'enabled'.
 */
void ECA_LOGGER_INTERFACE::set_log_level(ECA_LOGGER::Msg_level_t level, bool enabled)
{
  if (enabled == true) {
    debug_value_rep |= level;
  }
  else {
    debug_value_rep &= ~level;
  }
}

/**
 * Flush all log messages.
 */
void ECA_LOGGER_INTERFACE::flush(void)
{
  do_flush();
}

/**
 * Disables logging.
 * 
 * Note! Is equivalent to 
 * 'set_log_level(ECA_LOGGER_INTERFACE::disabled)'.
 */
void ECA_LOGGER_INTERFACE::disable(void)
{
  debug_value_rep = 0;
}

/**
 * Sets the log message history length.
 */
void ECA_LOGGER_INTERFACE::set_log_history_length(int len)
{
  log_history_len_rep = len;
}
