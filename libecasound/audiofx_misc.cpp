// ------------------------------------------------------------------------
// audiofx_misc.cpp: Miscellanous effect processing routines.
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

#include <iostream>
#include <cmath>

#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#include "samplebuffer_iterators.h"
#include "eca-operator.h"
#include "audiofx_misc.h"
#include "eca-logger.h"
#include "eca-error.h"

EFFECT_DCFIX::EFFECT_DCFIX (CHAIN_OPERATOR::parameter_t delta_left,
			    CHAIN_OPERATOR::parameter_t delta_right)
{

  deltafix_rep[0] = delta_left;
  deltafix_rep[1] = delta_right;
}

EFFECT_DCFIX::EFFECT_DCFIX (const EFFECT_DCFIX& x)
{
  for(int nm = 0; nm < 2; nm++) {
    deltafix_rep[nm] = x.deltafix_rep[nm];
  }
  i_rep = x.i_rep;
}

void EFFECT_DCFIX::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
  switch (param) {
  case 1: 
    deltafix_rep[0] = value;
    break;
  case 2: 
    deltafix_rep[1] = value;
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_DCFIX::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(deltafix_rep[0]);
  case 2: 
    return(deltafix_rep[1]);
  }
  return(0.0);
}

void EFFECT_DCFIX::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);
}

void EFFECT_DCFIX::init(SAMPLE_BUFFER *insample) { i_rep.init(insample); }

void EFFECT_DCFIX::process(void)
{
  for(int n = 0; n < 2; n++) {
    i_rep.begin(n);
    while(!i_rep.end()) {
      *i_rep.current() = *i_rep.current() + deltafix_rep[n];
      i_rep.next();
    }
  }
}

const int EFFECT_PITCH_SHIFT::resample_low_limit = 8;

EFFECT_PITCH_SHIFT::EFFECT_PITCH_SHIFT (const EFFECT_PITCH_SHIFT& x)
{
  pmod_rep = x.pmod_rep;
  target_rate_rep = x.target_rate_rep;
  sbuf_repp = 0;
}

void EFFECT_PITCH_SHIFT::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
  switch (param) {
  case 1: 
    double lowlimit = 1.0f / EFFECT_PITCH_SHIFT::resample_low_limit * 100.0f;
    if (value <= lowlimit) {
      ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		    "(audiofx_misc) WARNING! Shift-% must be greater than " +
		    kvu_numtostr(lowlimit) + 
		    "%! Limiting to the low-limit.");
      pmod_rep = lowlimit;
    }
    else {
      pmod_rep = value;
    }
    if (sbuf_repp != 0) {
      target_rate_rep = static_cast<long int>(samples_per_second() * 100.0 / pmod_rep);
    }
    else {
      target_rate_rep = 0;
    }
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_PITCH_SHIFT::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(pmod_rep);
  }
  return(0.0);
}

void EFFECT_PITCH_SHIFT::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
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

void EFFECT_PITCH_SHIFT::init(SAMPLE_BUFFER *insample)
{
  sbuf_repp = insample;
  target_rate_rep = static_cast<long int>(samples_per_second() * 100.0 / pmod_rep);

  long int lowlimit = sbuf_repp->length_in_samples() * EFFECT_PITCH_SHIFT::resample_low_limit; 
  sbuf_repp->reserve_length_in_samples(lowlimit);
  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audiofx) setting resampling lowlimit to " + 
		kvu_numtostr(lowlimit) + " bytes.");

  sbuf_repp->resample_init_memory(samples_per_second(), target_rate_rep);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audiofx) resampling from " +
		                         kvu_numtostr(samples_per_second()) + 
		                         " to " + 
		                         kvu_numtostr(target_rate_rep) + "."); 
}

void EFFECT_PITCH_SHIFT::release(void)
{
  sbuf_repp = 0;
}

void EFFECT_PITCH_SHIFT::process(void)
{
  sbuf_repp->resample(samples_per_second(), target_rate_rep); 
}

long int EFFECT_PITCH_SHIFT::output_samples(long int i_samples) const
{
  DBC_CHECK(sbuf_repp != 0);
  return(static_cast<long int>(static_cast<double>(target_rate_rep) /
			       samples_per_second() *
			       i_samples));
}

EFFECT_AUDIO_STAMP::EFFECT_AUDIO_STAMP(void) 
  : sbuf_repp(0) 
{
}

EFFECT_AUDIO_STAMP::EFFECT_AUDIO_STAMP(const EFFECT_AUDIO_STAMP& arg) 
  : sbuf_repp(0) 
{
}

void EFFECT_AUDIO_STAMP::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
  switch (param) {
  case 1: 
    set_id(static_cast<int>(value));
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_AUDIO_STAMP::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(static_cast<parameter_t>(id()));
  }
  return(0.0);
}

void EFFECT_AUDIO_STAMP::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
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

void EFFECT_AUDIO_STAMP::init(SAMPLE_BUFFER *insample)
{
  sbuf_repp = insample;
}

void EFFECT_AUDIO_STAMP::release(void)
{
  sbuf_repp = 0;
}

void EFFECT_AUDIO_STAMP::process(void)
{
  store(sbuf_repp);
  // std::cerr << "Storing audio stamp with id " << id() << "." << std::endl;
}
