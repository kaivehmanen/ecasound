// ------------------------------------------------------------------------
// audiofx_amplitude.cpp: Amplitude effects and dynamic processors.
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

#include <kvu_message_item.h>

#include "samplebuffer_iterators.h"
#include "audiofx_amplitude.h"

#include "eca-logger.h"
#include "eca-error.h"

EFFECT_AMPLITUDE::~EFFECT_AMPLITUDE(void)
{
}

EFFECT_AMPLIFY::EFFECT_AMPLIFY (parameter_t multiplier_percent)
{
  set_parameter(1, multiplier_percent);
}

EFFECT_AMPLIFY::~EFFECT_AMPLIFY(void)
{
}

void EFFECT_AMPLIFY::set_parameter(int param, parameter_t value) {
  switch (param) {
  case 1: 
    gain = value / 100.0;
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_AMPLIFY::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(gain * 100.0);
  }
  return(0.0);
}

void EFFECT_AMPLIFY::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);
}

void EFFECT_AMPLIFY::init(SAMPLE_BUFFER* sbuf) { i.init(sbuf); }

void EFFECT_AMPLIFY::process(void) {
  i.begin();
  while(!i.end()) {
    *i.current() = *i.current() *  gain;
    i.next();
  }
}

EFFECT_AMPLIFY_CLIPCOUNT::EFFECT_AMPLIFY_CLIPCOUNT (parameter_t multiplier_percent, int max_clipped) {
  set_parameter(1, multiplier_percent);
  set_parameter(2, max_clipped);
  num_of_clipped = 0;
}

void EFFECT_AMPLIFY_CLIPCOUNT::set_parameter(int param, parameter_t value) {
  switch (param) {
  case 1: 
    gain = value / 100.0;
    break;
  case 2:
    maxnum_of_clipped = (int)value;
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_AMPLIFY_CLIPCOUNT::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(gain * 100.0);
  case 2:
    return(maxnum_of_clipped);
  }
  return(0.0);
}

void EFFECT_AMPLIFY_CLIPCOUNT::init(SAMPLE_BUFFER* sbuf) { i.init(sbuf); }

void EFFECT_AMPLIFY_CLIPCOUNT::process(void) {
  i.begin();
  while(!i.end()) {
    *i.current() = *i.current() *  gain;
    if (*i.current() > SAMPLE_SPECS::impl_max_value ||
	*i.current() < SAMPLE_SPECS::impl_min_value) {
      num_of_clipped++;
    }
    else {
      num_of_clipped = 0;
    }
    i.next();
  }

  if (num_of_clipped > maxnum_of_clipped && maxnum_of_clipped != 0) {
    MESSAGE_ITEM otemp;
    otemp.setprecision(0);
    otemp << "(audiofx_amplitude) WARNING! Signal is clipping! ";
    otemp << num_of_clipped;
    otemp << " consecutive clipped samples.";
    ECA_LOG_MSG(ECA_LOGGER::info, otemp.to_string());
  }
}

void EFFECT_AMPLIFY_CLIPCOUNT::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);
}

EFFECT_AMPLIFY_CHANNEL::EFFECT_AMPLIFY_CHANNEL (parameter_t multiplier_percent, int channel) {
  set_parameter(1, multiplier_percent);
  set_parameter(2, channel);
}

void EFFECT_AMPLIFY_CHANNEL::set_parameter(int param, parameter_t value) {
  switch (param) {
    case 1: 
      gain = value / 100.0;
      break;
      
    case 2: 
      if (value > 0) {
	channel_rep = static_cast<int>(value);
	channel_rep--;
	break;
      }
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_AMPLIFY_CHANNEL::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(gain * 100.0);

  case 2: 
    return(static_cast<parameter_t>(channel_rep + 1));
  }
  return(0.0);
}

void EFFECT_AMPLIFY_CHANNEL::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);

  switch (param) {
  case 2: 
    {
      pd->default_value = 1.0f;
      pd->bounded_above = false;
      pd->bounded_below = true;
      pd->lower_bound = 1.0f;
      pd->integer = true;
    }
  }
}

void EFFECT_AMPLIFY_CHANNEL::init(SAMPLE_BUFFER *insample) { i.init(insample); }

void EFFECT_AMPLIFY_CHANNEL::process(void) {
  i.begin(channel_rep);
  while(!i.end()) {
    *i.current() = *i.current() * gain;
    i.next();
  }
}

EFFECT_LIMITER::EFFECT_LIMITER (parameter_t limiting_percent)
{
  set_parameter(1, limiting_percent);
}

EFFECT_LIMITER::~EFFECT_LIMITER(void)
{
}

void EFFECT_LIMITER::set_parameter(int param, parameter_t value) {
  switch (param) {
  case 1:
    limit_rep = value / 100.0;
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_LIMITER::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(limit_rep * 100.0);
  }
  return(0.0);
}

void EFFECT_LIMITER::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);
}

void EFFECT_LIMITER::init(SAMPLE_BUFFER* sbuf) { i.init(sbuf); }

void EFFECT_LIMITER::process(void) {
  i.begin();
  while(!i.end()) {
    if (*i.current() < 0) {
      if ((-(*i.current())) > limit_rep) 
	*i.current() = -limit_rep;
    }
    else {
      if (*i.current() > limit_rep)
	*i.current() = limit_rep;
     
    }
    i.next();
  }
}

EFFECT_COMPRESS::EFFECT_COMPRESS (parameter_t compress_rate, parameter_t thold) {
  set_parameter(1, compress_rate);
  set_parameter(2, thold);

  first_time = true;
}

EFFECT_COMPRESS::EFFECT_COMPRESS (const EFFECT_COMPRESS& x) {
  crate = x.crate;
  threshold = x.threshold;
  delta = x.delta;
  ratio = x.ratio;
  first_time = x.first_time;

  lastin = x.lastin;
  lastout = x.lastout;
}

void EFFECT_COMPRESS::set_parameter(int param, parameter_t value) {
  switch (param) {
  case 1: 
    crate = pow(2.0, value / 6.0);
    break;
  case 2: 
    threshold = value / 100.0;
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_COMPRESS::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return((log (crate)) / (log (2.0f)) * 6.0);
  case 2: 
    return(threshold * 100.0);
  }
  return(0.0);
}

void EFFECT_COMPRESS::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);
}

void EFFECT_COMPRESS::init(SAMPLE_BUFFER *insample) { 
  i.init(insample);

  set_channels(insample->number_of_channels());
  set_samples_per_second(samples_per_second());

  lastin.resize(insample->number_of_channels());
  lastout.resize(insample->number_of_channels());
} 

void EFFECT_COMPRESS::process(void) {
  i.begin();
  while(!i.end()) {
    if (first_time) {
      first_time = false;
      lastin[i.channel()] = lastout[i.channel()] = *i.current();
    }
    else {
      if (fabs(*i.current()) > threshold) {
	delta = *i.current() - lastin[i.channel()];
	delta /= crate;
	new_value = lastin[i.channel()] + delta;
	ratio = new_value / lastin[i.channel()];
	new_value = lastout[i.channel()] * ratio;

	if (new_value > SAMPLE_SPECS::impl_max_value) new_value = SAMPLE_SPECS::impl_max_value;
	else if (new_value < SAMPLE_SPECS::impl_min_value) new_value = SAMPLE_SPECS::impl_min_value;

	lastin[i.channel()] = *i.current();
	*i.current() = lastout[i.channel()] = new_value;
      }
      else {
	lastin[i.channel()] = lastout[i.channel()] = *i.current();
      }
    }
    i.next();
  }
}

EFFECT_NOISEGATE::EFFECT_NOISEGATE (parameter_t thlevel_percent, parameter_t thtime, parameter_t a, parameter_t h, parameter_t r) {
  // map_parameters();

  set_parameter(1, thlevel_percent);
  set_parameter(2, thtime);
  set_parameter(3, a);
  set_parameter(4, h);
  set_parameter(5, r);
}

void EFFECT_NOISEGATE::set_parameter(int param, parameter_t value) {
  switch (param) {
  case 1: 
    th_level = SAMPLE_SPECS::max_amplitude * (value / 100.0);
    break;
  case 2: 
    th_time = (value * (parameter_t)samples_per_second() / 1000.0);
    break;
  case 3: 
    atime = (value * (parameter_t)samples_per_second() / 1000.0);
    break;
  case 4: 
    htime = (value * (parameter_t)samples_per_second() / 1000.0);
    break;
  case 5: 
    rtime = (value * (parameter_t)samples_per_second() / 1000.0);
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_NOISEGATE::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(th_level * 100.0 / (parameter_t)SAMPLE_SPECS::max_amplitude);
  case 2: 
    return(th_time * 1000.0 / (parameter_t)samples_per_second());
  case 3: 
    return(atime * 1000.0 / (parameter_t)samples_per_second());
  case 4: 
    return(htime * 1000.0 / (parameter_t)samples_per_second());
  case 5: 
    return(rtime * 1000.0 / (parameter_t)samples_per_second());
  }
  return(0.0);
}

void EFFECT_NOISEGATE::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);
}

void EFFECT_NOISEGATE::init(SAMPLE_BUFFER *insample) {
  i.init(insample);

  set_channels(insample->number_of_channels());
  set_samples_per_second(samples_per_second());

  th_time_lask.resize(insample->number_of_channels());
  attack_lask.resize(insample->number_of_channels());
  hold_lask.resize(insample->number_of_channels());
  release_lask.resize(insample->number_of_channels());
  gain.resize(insample->number_of_channels());

  ng_status.resize(insample->number_of_channels(), int(ng_waiting));
}

void EFFECT_NOISEGATE::process(void) {
  i.begin();
  while(!i.end()) {
    bool below = fabs(*i.current()) <= th_level;

    switch(ng_status[i.channel()]) 
      {
      case ng_waiting: 
	// ---
	// phase 1 - waiting
	// ---
	{
	  if (below) {
	    th_time_lask[i.channel()]++;
	    if (th_time_lask[i.channel()] >= th_time) {
	      th_time_lask[i.channel()] = 0.0;
	      ng_status[i.channel()] = ng_attacking;
	      ECA_LOG_MSG(ECA_LOGGER::user_objects,"(audiofx) noisegate - from waiting to attacking");
	    }
	  }
	  else {
	    th_time_lask[i.channel()] = 0;
	  }
	  break;
	}

      case ng_attacking: 
	// ---
	// phase 2 - attack
	// ---
	{
	  if (below) {
	    attack_lask[i.channel()]++;
	    gain[i.channel()] = (1.0 - (attack_lask[i.channel()] / atime));
	    if (attack_lask[i.channel()] >= atime) {
	      attack_lask[i.channel()] = 0.0;
	      ng_status[i.channel()] = ng_active;
	      gain[i.channel()] = 0.0;
	      ECA_LOG_MSG(ECA_LOGGER::user_objects,"(audiofx) noisegate - from attack to active");
	    }
	    *i.current() = *i.current() * gain[i.channel()];
	  }
	  else {
	    attack_lask[i.channel()] = 0;
	    ng_status[i.channel()] = ng_waiting;
	    ECA_LOG_MSG(ECA_LOGGER::user_objects,"(audiofx) noisegate - from attack to waiting");
	  }
	  break;
	}

      case ng_active: 
	// ---
	// phase 3 - active
	// ---
	{
	  if (below == false) {
	    ng_status[i.channel()] = ng_holding;
	    ECA_LOG_MSG(ECA_LOGGER::user_objects,"(audiofx) noisegate - from active to holding");
	  }
	  *i.current() = *i.current() * 0.0;
	  break;
	}

      case ng_holding: 
	// ---
	// phase 4 - holding
	// ---
	{
	  if (!below) {
	    hold_lask[i.channel()]++;
	    if (hold_lask[i.channel()] >= htime) {
	      hold_lask[i.channel()] = 0.0;
	      ng_status[i.channel()] = ng_releasing;
	      ECA_LOG_MSG(ECA_LOGGER::user_objects,"(audiofx) noisegate - from holding to release");
	    }
	  }
	  *i.current() = *i.current() * 0.0;
	  break;
	}

      case ng_releasing: 
	// ---
	// phase 5 - releasing
	// ---
	{
	  release_lask[i.channel()]++;
	  gain[i.channel()] = release_lask[i.channel()] / rtime;
	  if (release_lask[i.channel()] >= rtime) {
	    release_lask[i.channel()] = 0.0;
	    ng_status[i.channel()] = ng_waiting;
	    ECA_LOG_MSG(ECA_LOGGER::user_objects,"(audiofx) noisegate - from releasing to waiting");
	  }
	  *i.current() = *i.current() * gain[i.channel()];
	  break;
	}
      }

    i.next();
  }
}

EFFECT_NORMAL_PAN::EFFECT_NORMAL_PAN (parameter_t right_percent) {
  set_parameter(1, right_percent);
}

void EFFECT_NORMAL_PAN::set_parameter(int param, parameter_t value) {
  switch (param) {
  case 1: 
    right_percent_rep = value;
    if (value == 50.0) {
      l_gain = r_gain = 1.0;
    }
    else if (value < 50.0) {
      l_gain = 1.0;
      r_gain = value / 50.0;
    }
    else if (value > 50.0) {
      r_gain = 1.0;
      l_gain = (100.0 - value) / 50.0;
    }
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_NORMAL_PAN::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(right_percent_rep);
  }
  return(0.0);
}

void EFFECT_NORMAL_PAN::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  OPERATOR::parameter_description(param, pd);

  switch (param) {
  case 1: 
    {
      pd->default_value = 50.0f;
      pd->bounded_above = true;
      pd->upper_bound = 100.0f;
      pd->bounded_below = true;
      pd->lower_bound = 100.0f;
    }
  }
}

void EFFECT_NORMAL_PAN::init(SAMPLE_BUFFER *insample) { i.init(insample); }

void EFFECT_NORMAL_PAN::process(void) {
  i.begin(0);
  while(!i.end()) {
    *i.current() = *i.current() * l_gain;
    i.next();
  }

  i.begin(1);
  while(!i.end()) {
    *i.current() = *i.current() * r_gain;
    i.next();
  }
}
