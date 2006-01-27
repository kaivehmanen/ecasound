// ------------------------------------------------------------------------
// audiogate.cpp: Signal gates.
// Copyright (C) 1999-2002,2005-2006 Kai Vehmanen
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

#include <kvu_numtostr.h>

#include "samplebuffer.h"
#include "samplebuffer_functions.h"
#include "audiogate.h"
#include "eca-logger.h"

GATE_BASE::~GATE_BASE(void)
{
}

void GATE_BASE::process(void)
{
  analyze(target);
  if (is_open() == false) {
    target->length_in_samples(0);
  }
}

void GATE_BASE::init(SAMPLE_BUFFER* sbuf)
{ 
  target = sbuf;
}

void TIME_CROP_GATE::analyze(SAMPLE_BUFFER* sbuf)
{
  parameter_t etime = begtime_rep + durtime_rep;
  if (curtime_rep >= begtime_rep) {
    if (begtime_rep == etime) 
      open_gate();
    else if (curtime_rep < etime) 
      open_gate();
    else 
      close_gate();
  }
  else 
    close_gate();

  curtime_rep += static_cast<double>(sbuf->length_in_samples()) / samples_per_second();
}

TIME_CROP_GATE::TIME_CROP_GATE (CHAIN_OPERATOR::parameter_t open_at, CHAIN_OPERATOR::parameter_t duration)
{
  begtime_rep = open_at;
  durtime_rep = duration;
  curtime_rep = 0.0;
  
  ECA_LOG_MSG(ECA_LOGGER::info, "(audiogate) Time crop gate created; opens at " +
	      kvu_numtostr(begtime_rep) + " seconds and stays open for " +
	      kvu_numtostr(durtime_rep) + " seconds.\n");
}

CHAIN_OPERATOR::parameter_t TIME_CROP_GATE::get_parameter(int param) const 
{ 
  switch (param) {
  case 1: 
    return begtime_rep;
  case 2: 
    return durtime_rep;
  }
  return 0.0;
}

void TIME_CROP_GATE::set_parameter(int param, CHAIN_OPERATOR::parameter_t value) 
{
  switch (param) {
  case 1: 
    begtime_rep = value;
    curtime_rep = 0.0;
    break;
  case 2: 
    durtime_rep = value;
    break;
  }
}

THRESHOLD_GATE::THRESHOLD_GATE (CHAIN_OPERATOR::parameter_t threshold_openlevel, 
				CHAIN_OPERATOR::parameter_t threshold_closelevel,
				bool use_rms) 
{
  openlevel_rep = threshold_openlevel / 100.0;
  closelevel_rep = threshold_closelevel / 100.0;
  rms_rep = use_rms;

  is_opened_rep = is_closed_rep = false;

  if (rms_rep) {
    ECA_LOG_MSG(ECA_LOGGER::info, "(audiogate) Threshold gate created; open threshold " +
		kvu_numtostr(openlevel_rep * 100) + "%, close threshold " +
		kvu_numtostr(closelevel_rep * 100) + "%, using RMS volume.");
  }
  else {
    ECA_LOG_MSG(ECA_LOGGER::info, "(audiogate) Threshold gate created; open threshold " +
		kvu_numtostr(openlevel_rep * 100) + "%, close threshold " +
		kvu_numtostr(closelevel_rep * 100) + "%, using peak volume.");
  }
}

void THRESHOLD_GATE::analyze(SAMPLE_BUFFER* sbuf)
{
  if (rms_rep == true)
    avolume_rep = SAMPLE_BUFFER_FUNCTIONS::RMS_volume(*sbuf) / SAMPLE_SPECS::max_amplitude;
  else 
    avolume_rep = SAMPLE_BUFFER_FUNCTIONS::average_amplitude(*sbuf) / SAMPLE_SPECS::max_amplitude;

  if (is_opened_rep == false) {
    if (avolume_rep > openlevel_rep) { 
      open_gate();
      ECA_LOG_MSG(ECA_LOGGER::user_objects, "Threshold gate opened.");
      is_opened_rep = true;
    }
  }
  else if (is_closed_rep == false) {
    if (avolume_rep < closelevel_rep) { 
      close_gate();
      ECA_LOG_MSG(ECA_LOGGER::user_objects, "Threshold gate closed.");
      is_closed_rep = true;
    }
  }
}

CHAIN_OPERATOR::parameter_t THRESHOLD_GATE::get_parameter(int param) const
{ 
  switch (param) {
  case 1: 
    return openlevel_rep * 100.0;
  case 2: 
    return closelevel_rep * 100.0;
  case 3: 
    if (rms_rep) 
      return 1.0;
    else 
      return 0.0;
  }
  return 0.0;
}

void THRESHOLD_GATE::set_parameter(int param, CHAIN_OPERATOR::parameter_t value) 
{
  switch (param) {
  case 1: 
    openlevel_rep = value / 100.0;
    break;
  case 2: 
    closelevel_rep = value / 100.0;
    break;
  case 3: 
    if (value != 0) 
      rms_rep = true;
    else 
      rms_rep = false;
    break;
  }
}
