// ------------------------------------------------------------------------
// eca-logger-default.cpp: Default logging subsystem implementation.
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

#ifndef INCLUDE_ECA_LOGGER_DEFAULT_H
#define INCLUDE_ECA_LOGGER_DEFAULT_H

#include <iostream>
#include <string>

#include "eca-logger-interface.h"

/**
 * Default logging subsystem implementation.
 *
 * @author Kai Vehmanen
 */
class ECA_LOGGER_DEFAULT : public ECA_LOGGER_INTERFACE {
  
public:

  virtual void do_msg(ECA_LOGGER::Msg_level_t level, const std::string& module_name, const std::string& log_message);
  virtual void do_flush(void);
  virtual void do_log_level_changed(void);
  virtual ~ECA_LOGGER_DEFAULT(void);
};

#endif /* INCLUDE_ECA_LOGGER_DEFAULT_H */
