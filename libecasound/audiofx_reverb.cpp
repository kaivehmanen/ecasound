// ------------------------------------------------------------------------
// audiofx_reverb.cpp: Reverb effect
// Copyright (C) 2000 Stefan Fendt <stefan@lionfish.ping.de>,
//                    Kai Vehmanen <kaiv@wakkanet.fi> (C++ version)
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

#include <cstdlib>
#include "samplebuffer_iterators.h"
#include "sample-specs.h"
#include "audiofx_reverb.h"

ADVANCED_REVERB::ADVANCED_REVERB (parameter_type roomsize,
				  parameter_type feedback_percent, 
				  parameter_type wet_percent) {
  srate_rep = SAMPLE_SPECS::sample_rate_default;
  set_parameter(1, roomsize);
  set_parameter(2, feedback_percent);
  set_parameter(3, wet_percent);
}

CHAIN_OPERATOR::parameter_type ADVANCED_REVERB::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(roomsize_rep);
  case 2:
    return(feedback_rep * 100.0);
  case 3:
    return(wet_rep * 100.0);
  }
  return(0.0);
}

void ADVANCED_REVERB::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 
    roomsize_rep = value;
    break;

  case 2: 
    if (value == 0) 
      feedback_rep = 0.001;
    else
      feedback_rep = value / 100.0;
    break;

  case 3: 
    wet_rep = value / 100.0;
    break;
  }
  if (param == 1 || param == 2) {
    vector<CHANNEL_DATA>::iterator p = cdata.begin();
    while(p != cdata.end()) {
      p->dpos[0] = static_cast<long int>(roomsize_rep * srate_rep / 333);
      p->mul[0] = 0.035;
      p->bufferpos_rep = 0;
      for(int i = 1; i < 64; i++) {
	p->dpos[i] = p->dpos[i-1] + (rand() & 511);
	p->mul[i] = p->mul[i-1] * (1 - 1 / feedback_rep / 1000);
      }
      ++p;
    }
  }
}

void ADVANCED_REVERB::init(SAMPLE_BUFFER *insample) {
  i_channels.init(insample);
  srate_rep = insample->sample_rate();
  cdata.resize(insample->number_of_channels());
  vector<CHANNEL_DATA>::iterator p = cdata.begin();
  while(p != cdata.end()) {
    p->dpos[0] = static_cast<long int>(roomsize_rep * srate_rep / 333);
    p->mul[0] = 0.035;
    p->bufferpos_rep = 0;
    for(int i = 1; i < 64; i++) {
      p->dpos[i] = p->dpos[i-1] + (rand() & 511);
      p->mul[i] = p->mul[i-1] * (1 - 1 / feedback_rep / 1000);
    }
    ++p;
  }
}

void ADVANCED_REVERB::process(void) {
  i_channels.begin();
  while(!i_channels.end()) {
    cdata[i_channels.channel()].bufferpos_rep++;
    cdata[i_channels.channel()].bufferpos_rep &= 65535;

    double old_value = cdata[i_channels.channel()].oldvalue;
    cdata[i_channels.channel()].buffer[cdata[i_channels.channel()].bufferpos_rep] = *i_channels.current() + old_value;

    old_value = 0.0;
    for(int i = 0; i < 64; i++) {
      old_value +=
	static_cast<float>(cdata[i_channels.channel()].buffer[(cdata[i_channels.channel()].bufferpos_rep - cdata[i_channels.channel()].dpos[i]) & 65535] * cdata[i_channels.channel()].mul[i]);
    }

    /**
     * This is just a very simple high-pass-filter to remove offsets
     * which can accour during calculation of the echos
     */
    cdata[i_channels.channel()].lpvalue = cdata[i_channels.channel()].lpvalue * 0.99 + old_value * 0.01;
    old_value = old_value - cdata[i_channels.channel()].lpvalue;

    /**
     * This is a simple lowpass to make the apearence of the reverb 
     * more realistic... (Walls do not reflect high frequencies very
     * well at all...) 
     */
    cdata[i_channels.channel()].oldvalue = cdata[i_channels.channel()].oldvalue * 0.75 + old_value * 0.25;

    *i_channels.current() = cdata[i_channels.channel()].oldvalue * wet_rep + *i_channels.current() * 0.8;
    i_channels.next();
  }
}
