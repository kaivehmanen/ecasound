// ------------------------------------------------------------------------
// osc-sine.cpp: Sine oscillator
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
//
// This program is fre software; you can redistribute it and/or modify
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

#include <cmath>

#include <kvutils.h>

#include "oscillator.h"
#include "osc-sine.h"
#include "eca-debug.h"

DYNAMIC_PARAMETERS::parameter_type SINE_OSCILLATOR::value(void) {
  curval = (sin(phase) + 1.0) / 2.0;
  phase += phasemod * step_length();
  return(curval);
}

SINE_OSCILLATOR::SINE_OSCILLATOR (double freq, double initial_phase) :
  OSCILLATOR(freq, initial_phase) {

  set_parameter(1, get_parameter(1));
  set_parameter(2, get_parameter(2));
}

void SINE_OSCILLATOR::init(DYNAMIC_PARAMETERS::parameter_type phasestep) {
  step_length(phasestep);

  MESSAGE_ITEM otemp;
  otemp << "(osc-sine) Sine oscillator created; frequency ";
  otemp.setprecision(1);
  otemp << frequency();
  otemp << " and initial phase of "; 
  otemp << phase_offset() << ".";
  ecadebug->msg(otemp.to_string());
}

void SINE_OSCILLATOR::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1: 
    frequency(value);
    L = 1.0 / frequency();   // length of one wave in seconds
    phasemod = 2.0 * M_PI / L; 
    break;

  case 2: 
    phase_offset(static_cast<double>(value * M_PI));
    phase = phase_offset();
    break;
  }
}

DYNAMIC_PARAMETERS::parameter_type SINE_OSCILLATOR::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(frequency());
  case 2: 
    return(phase_offset() / M_PI);
  }
  return(0.0);
}




