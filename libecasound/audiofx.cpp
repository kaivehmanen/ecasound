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
#include "audiofx.h"

EFFECT_BASE::EFFECT_BASE(void) : 
  srate_rep(SAMPLE_SPECS::sample_rate_default),
  channels_rep(SAMPLE_SPECS::channel_count_default) { }

EFFECT_BASE::~EFFECT_BASE(void) { }

long int EFFECT_BASE::samples_per_second(void) const { return(srate_rep); }
int EFFECT_BASE::channels(void) const { return(channels_rep); }

void EFFECT_BASE::set_samples_per_second(long int v) { srate_rep = v; }
void EFFECT_BASE::set_channels(int v) { channels_rep = v; }
