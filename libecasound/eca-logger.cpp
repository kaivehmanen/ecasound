// ------------------------------------------------------------------------
// eca-logger.cpp: A logging subsystem implemented as a singleton class
// Copyright (C) 2002 Kai Vehmanen
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

#include <kvu_dbc.h>
#include <kvu_locks.h>

#include "eca-logger-interface.h"
#include "eca-logger-default.h"
#include "eca-logger.h"

ECA_LOGGER_INTERFACE* ECA_LOGGER::interface_impl_repp = 0;
pthread_mutex_t ECA_LOGGER::lock_rep = PTHREAD_MUTEX_INITIALIZER;

ECA_LOGGER_INTERFACE& ECA_LOGGER::instance(void)
{
  //
  // Note! Below we use the Double-Checked Locking Pattern
  //       to protect against concurrent access

  if (ECA_LOGGER::interface_impl_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_LOGGER::lock_rep);
    if (ECA_LOGGER::interface_impl_repp == 0) {
      ECA_LOGGER::interface_impl_repp = new ECA_LOGGER_DEFAULT();
    }
  }
  return(*interface_impl_repp);
}

void ECA_LOGGER::attach_logger(ECA_LOGGER_INTERFACE* logger)
{
  int oldloglevel = -1;
  if (interface_impl_repp != 0) {
    oldloglevel = interface_impl_repp->get_log_level_bitmask();
  }
  ECA_LOGGER::detach_logger();
  if (ECA_LOGGER::interface_impl_repp == 0) {
    KVU_GUARD_LOCK guard(&ECA_LOGGER::lock_rep);
    if (ECA_LOGGER::interface_impl_repp == 0) {
      ECA_LOGGER::interface_impl_repp = logger;
      if (oldloglevel != -1) {
	logger->set_log_level_bitmask(oldloglevel);
      }
    }
  }
  DBC_ENSURE(ECA_LOGGER::interface_impl_repp == logger);
}

/**
 * Detaches the current logger implementation.
 */
void ECA_LOGGER::detach_logger(void)
{
  if (ECA_LOGGER::interface_impl_repp != 0) {
    KVU_GUARD_LOCK guard(&ECA_LOGGER::lock_rep);
    if (ECA_LOGGER::interface_impl_repp != 0) {
      delete ECA_LOGGER::interface_impl_repp;
      ECA_LOGGER::interface_impl_repp = 0;
    }
  }
  DBC_ENSURE(ECA_LOGGER::interface_impl_repp == 0);
}
