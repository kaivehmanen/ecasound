// ------------------------------------------------------------------------
// audiofx.cpp: Generel effect processing routines.
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

#include <cmath>

#include <kvutils.h>

#include "samplebuffer_iterators.h"
#include "audiofx.h"
#include "eca-debug.h"
#include "eca-error.h"

EFFECT_DCFIX::EFFECT_DCFIX (DYNAMIC_PARAMETERS::parameter_type delta_left, DYNAMIC_PARAMETERS::parameter_type delta_right) {

  deltafix[0] = delta_left;
  deltafix[1] = delta_right;
}

EFFECT_DCFIX::EFFECT_DCFIX (const EFFECT_DCFIX& x) {
  for(int nm = 0; nm < 2; nm++) {
    deltafix[nm] = x.deltafix[nm];
  }
}

void EFFECT_DCFIX::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1: 
    deltafix[0] = value;
    break;
  case 2: 
    deltafix[1] = value;
    break;
  }
}

DYNAMIC_PARAMETERS::parameter_type EFFECT_DCFIX::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(deltafix[0]);
  case 2: 
    return(deltafix[1]);
  }
  return(0.0);
}

void EFFECT_DCFIX::init(SAMPLE_BUFFER *insample) { i.init(insample); }

void EFFECT_DCFIX::process(void) {
  for(int n = 0; n < 2; n++) {
    i.begin(n);
    while(!i.end()) {
      *i.current() = *i.current() + deltafix[n];
      i.next();
    }
  }
}

void EFFECT_PITCH_SHIFT::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1: 
    pmod = value;
    break;
  }
}

DYNAMIC_PARAMETERS::parameter_type EFFECT_PITCH_SHIFT::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(pmod);
  }
  return(0.0);
}

void EFFECT_PITCH_SHIFT::init(SAMPLE_BUFFER *insample) { 
  sbuf = insample;
  target_rate = static_cast<long int>(sbuf->sample_rate() * 100.0 / pmod);
  ecadebug->msg(ECA_DEBUG::user_objects, "(audiofx) resampling from " +
		                         kvu_numtostr(sbuf->sample_rate()) + 
		                         " to " + 
		                         kvu_numtostr(target_rate) + "."); 
}

void EFFECT_PITCH_SHIFT::process(void) { 
  sbuf->resample_to(target_rate); 
}

long int EFFECT_PITCH_SHIFT::output_samples(long int i_samples) {
  assert(sbuf != 0);
  return(static_cast<long int>(static_cast<double>(target_rate) /
			       sbuf->sample_rate() *
			       i_samples));
}
