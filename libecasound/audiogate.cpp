// ------------------------------------------------------------------------
// audiogate.cpp: Signal gates.
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

#include <kvutils/kvu_numtostr.h>

#include "samplebuffer_functions.h"
#include "audiogate.h"
#include "eca-debug.h"

void GATE_BASE::process(void) {
  analyze(target);
  if (is_open() == false) {
//      target->make_silent();
    target->length_in_samples(0);
  }
}

void GATE_BASE::init(SAMPLE_BUFFER* sbuf) { 
  target = sbuf;
}

void TIME_CROP_GATE::analyze(SAMPLE_BUFFER* sbuf) {
  if (curtime >= btime) {
    if (btime == etime) open_gate();
    else if (curtime < etime) open_gate();
    else close_gate();
  }
  else 
    close_gate();

  curtime += sbuf->length_in_seconds();
}

TIME_CROP_GATE::TIME_CROP_GATE (CHAIN_OPERATOR::parameter_type open_at, CHAIN_OPERATOR::parameter_type duration) {
  btime = open_at;
  etime = btime + duration;
  curtime = 0.0;
  
  ecadebug->msg("(audiogate) Time crop gate created; opens at " +
		kvu_numtostr(btime) + " seconds and stays open for " +
		kvu_numtostr(duration) + " seconds.\n");
}

CHAIN_OPERATOR::parameter_type TIME_CROP_GATE::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(btime);
  case 2: 
    return(etime - btime);
  }
  return(0.0);
}

void TIME_CROP_GATE::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 
    btime = value;
    curtime = 0.0;
    break;
  case 2: 
    etime = btime + value;
    break;
  }
}

THRESHOLD_GATE::THRESHOLD_GATE (CHAIN_OPERATOR::parameter_type threshold_openlevel, 
				CHAIN_OPERATOR::parameter_type threshold_closelevel,
				bool use_rms) {
  openlevel = threshold_openlevel / 100.0;
  closelevel = threshold_closelevel / 100.0;
  rms = use_rms;

  is_opened = is_closed = false;

  if (rms) {
    ecadebug->msg("(audiogate) Threshold gate created; open threshold " +
		  kvu_numtostr(openlevel * 100) + "%, close threshold " +
		  kvu_numtostr(closelevel * 100) + "%, using RMS volume.");
  }
  else {
    ecadebug->msg("(audiogate) Threshold gate created; open threshold " +
		  kvu_numtostr(openlevel * 100) + "%, close threshold " +
		  kvu_numtostr(closelevel * 100) + "%, using peak volume.");
  }
}

void THRESHOLD_GATE::analyze(SAMPLE_BUFFER* sbuf) {
  if (rms == true) avolume = SAMPLE_BUFFER_FUNCTIONS::RMS_volume(*sbuf) /
		     SAMPLE_SPECS::max_amplitude;
  else avolume = SAMPLE_BUFFER_FUNCTIONS::average_amplitude(*sbuf) / SAMPLE_SPECS::max_amplitude;

  if (is_opened == false) {
    if (avolume > openlevel) { 
      open_gate();
      ecadebug->msg(ECA_DEBUG::user_objects, "(audiogate) Threshold gate opened.");
      is_opened = true;
    }
  }
  else if (is_closed == false) {
    if (avolume < closelevel) { 
      close_gate();
      ecadebug->msg(ECA_DEBUG::user_objects, "(audiogate) Threshold gate closed.");
      is_closed = true;
    }
  }
}

CHAIN_OPERATOR::parameter_type THRESHOLD_GATE::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(openlevel * 100.0);
  case 2: 
    return(closelevel * 100.0);
  case 3: 
    if (rms) return(1.0);
    else return(0.0);
  }
  return(0.0);
}

void THRESHOLD_GATE::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 
    openlevel = value / 100.0;
    break;
  case 2: 
    closelevel = value / 100.0;
    break;
  case 3: 
    if (value != 0) rms = true;
    else rms = false;
    break;
  }
}



