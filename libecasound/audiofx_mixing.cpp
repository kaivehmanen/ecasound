// ------------------------------------------------------------------------
// audiofx_mixing.cpp: Effects for channel mixing and routing
// Copyright (C) 1999-2002 Kai Vehmanen
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

#include "samplebuffer_iterators.h"
#include "audiofx_mixing.h"

EFFECT_MIXING::~EFFECT_MIXING(void)
{
}

EFFECT_CHANNEL_COPY::EFFECT_CHANNEL_COPY (parameter_t from, 
					  parameter_t to)
{
  set_parameter(1, from);
  set_parameter(2, to);
}

int EFFECT_CHANNEL_COPY::output_channels(int i_channels) const
{
  int c = static_cast<int>(to_channel > from_channel ? to_channel : from_channel);
  ++c;
  return(c > i_channels ? c : i_channels);
}

void EFFECT_CHANNEL_COPY::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  pd->default_value = 1;
  pd->description = get_parameter_name(param);
  pd->bounded_above = false;
  pd->upper_bound = 0.0f;
  pd->bounded_below = true;
  pd->lower_bound = 1.0f;
  pd->toggled = false;
  pd->integer = true;
  pd->logarithmic = false;
  pd->output = false;
}

void EFFECT_CHANNEL_COPY::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
  switch (param) {
  case 1: 
    from_channel = static_cast<ch_type>(value);
    DBC_CHECK(from_channel > 0);
    from_channel--;
    break;
  case 2: 
    to_channel = static_cast<ch_type>(value);
    DBC_CHECK(to_channel > 0);
    to_channel--;
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_CHANNEL_COPY::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(from_channel + 1);
  case 2: 
    return(to_channel + 1);
  }
  return(0.0);
}

void EFFECT_CHANNEL_COPY::init(SAMPLE_BUFFER *insample) { 
  f_iter.init(insample);
  t_iter.init(insample);
}

void EFFECT_CHANNEL_COPY::process(void)
{
  f_iter.begin(from_channel);
  t_iter.begin(to_channel);
  while(!f_iter.end() && !t_iter.end()) {
    *t_iter.current() = *f_iter.current();
    f_iter.next();
    t_iter.next();
  }
}

EFFECT_MIX_TO_CHANNEL::EFFECT_MIX_TO_CHANNEL (parameter_t to)
{
  set_parameter(1, to);
}

int EFFECT_MIX_TO_CHANNEL::output_channels(int i_channels) const
{
  int c = static_cast<int>(to_channel);
  ++c;
  return(c > i_channels ? c : i_channels);
}

void EFFECT_MIX_TO_CHANNEL::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  pd->default_value = 1;
  pd->description = get_parameter_name(param);
  pd->bounded_above = false;
  pd->upper_bound = 0.0f;
  pd->bounded_below = true;
  pd->lower_bound = 1.0f;
  pd->toggled = false;
  pd->integer = true;
  pd->logarithmic = false;
  pd->output = false;
}

void EFFECT_MIX_TO_CHANNEL::set_parameter(int param, CHAIN_OPERATOR::parameter_t value) {
  switch (param) {
  case 1: 
    to_channel = static_cast<ch_type>(value);
    DBC_CHECK(to_channel > 0);
    to_channel--;
    break;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_MIX_TO_CHANNEL::get_parameter(int param) const
{ 
  switch (param) {
  case 1: 
    return(to_channel + 1);
  }
  return(0.0);
}

void EFFECT_MIX_TO_CHANNEL::init(SAMPLE_BUFFER *insample)
{
  i.init(insample);
  t_iter.init(insample);
  channels = insample->number_of_channels();
}

void EFFECT_MIX_TO_CHANNEL::process(void)
{
  i.begin();
  t_iter.begin(to_channel);
  while(!t_iter.end() && !i.end()) {
    sum = SAMPLE_SPECS::silent_value;
    for (int n = 0; n < channels; n++) {
      if (i.end()) break;
      sum += (*i.current(n));
    }
    *t_iter.current() = sum / channels;
    i.next();
    t_iter.next();
  }
}
