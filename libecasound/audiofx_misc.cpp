// ------------------------------------------------------------------------
// audiofx_misc.cpp: Miscellanous effect processing routines.
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <iostream>
#include <cmath>

#include <kvutils/kvu_numtostr.h>

#include "samplebuffer_iterators.h"
#include "eca-operator.h"
#include "audiofx_misc.h"
#include "eca-debug.h"
#include "eca-error.h"

EFFECT_DCFIX::EFFECT_DCFIX (CHAIN_OPERATOR::parameter_type delta_left, CHAIN_OPERATOR::parameter_type delta_right) {

  deltafix_rep[0] = delta_left;
  deltafix_rep[1] = delta_right;
}

EFFECT_DCFIX::EFFECT_DCFIX (const EFFECT_DCFIX& x) {
  for(int nm = 0; nm < 2; nm++) {
    deltafix_rep[nm] = x.deltafix_rep[nm];
  }
  i_rep = x.i_rep;
}

void EFFECT_DCFIX::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 
    deltafix_rep[0] = value;
    break;
  case 2: 
    deltafix_rep[1] = value;
    break;
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_DCFIX::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(deltafix_rep[0]);
  case 2: 
    return(deltafix_rep[1]);
  }
  return(0.0);
}

void EFFECT_DCFIX::parameter_description(int param, struct PARAM_DESCRIPTION *pd)
{
  OPERATOR::parameter_description(param, pd);
}

void EFFECT_DCFIX::init(SAMPLE_BUFFER *insample) { i_rep.init(insample); }

void EFFECT_DCFIX::process(void) {
  for(int n = 0; n < 2; n++) {
    i_rep.begin(n);
    while(!i_rep.end()) {
      *i_rep.current() = *i_rep.current() + deltafix_rep[n];
      i_rep.next();
    }
  }
}

EFFECT_PITCH_SHIFT::EFFECT_PITCH_SHIFT (const EFFECT_PITCH_SHIFT& x) {
  pmod_rep = x.pmod_rep;
  target_rate_rep = x.target_rate_rep;
  sbuf_repp = 0;
}

void EFFECT_PITCH_SHIFT::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 
    if (pmod_rep <= 0) {
      ecadebug->msg(ECA_DEBUG::user_objects, "(audiofx_misc) WARNING! Shift-% must be greater that 0! Using the default 100%.");
      pmod_rep = 100.0;
    }
    else
      pmod_rep = value;
    break;
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_PITCH_SHIFT::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(pmod_rep);
  }
  return(0.0);
}

void EFFECT_PITCH_SHIFT::parameter_description(int param, struct PARAM_DESCRIPTION *pd)
{
  OPERATOR::parameter_description(param, pd);

  switch(param) 
    {
    case 1: 
      pd->default_value = 100.0f;
      pd->bounded_above = true;
      pd->upper_bound = 500.0f;
      pd->bounded_below = true;
      pd->lower_bound = 25.0f;
      break;
    }
}

void EFFECT_PITCH_SHIFT::init(SAMPLE_BUFFER *insample) { 
  sbuf_repp = insample;
  target_rate_rep = static_cast<long int>(sbuf_repp->sample_rate() * 100.0 / pmod_rep);
  ecadebug->msg(ECA_DEBUG::user_objects, "(audiofx) resampling from " +
		                         kvu_numtostr(sbuf_repp->sample_rate()) + 
		                         " to " + 
		                         kvu_numtostr(target_rate_rep) + "."); 
}

void EFFECT_PITCH_SHIFT::process(void) { 
  target_rate_rep = static_cast<long int>(sbuf_repp->sample_rate() * 100.0 / pmod_rep);
  sbuf_repp->resample_with_memory(sbuf_repp->sample_rate(), target_rate_rep); 
}

long int EFFECT_PITCH_SHIFT::output_samples(long int i_samples) {
  assert(sbuf_repp != 0);
  return(static_cast<long int>(static_cast<double>(target_rate_rep) /
			       sbuf_repp->sample_rate() *
			       i_samples));
}

void EFFECT_AUDIO_STAMP::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 
    set_id(static_cast<int>(value));
    break;
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_AUDIO_STAMP::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(static_cast<parameter_type>(id()));
  }
  return(0.0);
}

void EFFECT_AUDIO_STAMP::parameter_description(int param, struct PARAM_DESCRIPTION *pd)
{
  OPERATOR::parameter_description(param, pd);
  switch(param) 
    {
    case 1: 
      pd->default_value = 1;
      pd->description = get_parameter_name(param);
      pd->bounded_above = false;
      pd->bounded_below = true;
      pd->lower_bound = 1;
      pd->integer = true;
      break;
    }
}

void EFFECT_AUDIO_STAMP::init(SAMPLE_BUFFER *insample) { 
  sbuf_repp = insample;
}

void EFFECT_AUDIO_STAMP::process(void) {
  store(sbuf_repp);
//    cerr << "Storing audio stamp with id " << id() << "." << endl;
}
