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

#include "sample-specs.h"
#include "samplebuffer.h"
#include "audiofx.h"

EFFECT_BASE::EFFECT_BASE(void) : 
  srate_rep(SAMPLE_SPECS::sample_rate_default),
  channels_rep(SAMPLE_SPECS::channel_count_default) { }

EFFECT_BASE::~EFFECT_BASE(void) { }

void EFFECT_BASE::init(SAMPLE_BUFFER* sbuf) {
  set_samples_per_second(sbuf->sample_rate());
  set_channels(sbuf->number_of_channels());
}

long int EFFECT_BASE::samples_per_second(void) const { return(srate_rep); }
int EFFECT_BASE::channels(void) const { return(channels_rep); }

void EFFECT_BASE::set_samples_per_second(long int v) { 
  if (samples_per_second() != v) {
    std::vector<parameter_t> old_values (number_of_params());
    for(int n = 0; n < number_of_params(); n++) {
      old_values[n] = get_parameter(n + 1);
    }
    srate_rep = v;
    for(int n = 0; n < number_of_params(); n++) {
      set_parameter(n + 1, old_values[n]);
    }
  }
  else
    srate_rep = v;
}

void EFFECT_BASE::set_channels(int v) { channels_rep = v; }
