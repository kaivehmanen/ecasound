// ------------------------------------------------------------------------
// audiofx_envelope-modulation.cpp: Effects which modify/modulate signal
//                                  envelope
// Copyright (C) 2000 Rob Coker, rcs@birch.net
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

#include <kvutils/message_item.h>

#include "samplebuffer_iterators.h"
#include "audiofx_envelope_modulation.h"

#include "eca-debug.h"
#include "eca-error.h"

EFFECT_PULSE_GATE::EFFECT_PULSE_GATE (parameter_type freq_Hz,
				      parameter_type onTime_percent) {
  set_parameter(1, freq_Hz);
  set_parameter(2, onTime_percent);
  currentTime = 0.0;
}

void EFFECT_PULSE_GATE::set_parameter(int param, parameter_type value) {
  switch (param) {
  case 1: 
    if (value > 0)
      {
	period = 1.0/value; // seconds
      }
    else
      {
	MESSAGE_ITEM otemp;
	otemp << "(audiofx_envelope_modulation) WARNING! Frequency must be greater than 0! ";
	ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
      }
    break;
  case 2:
    if ((value > 0) && (value < 100))
      {
	stopTime = (value/100.0)*period; // seconds
      }
    else
      {
	MESSAGE_ITEM otemp;
	otemp << "(audiofx_envelope_modulation) WARNING! on time must be between 0 and 100 inclusive! ";
	ecadebug->msg(ECA_DEBUG::user_objects, otemp.to_string());
      }
    break;
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_PULSE_GATE::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return(1.0/period);
    break;
  case 2:
    return (stopTime/period)*100.0;
    break;
  }
  return(0.0);
}

void EFFECT_PULSE_GATE::init(SAMPLE_BUFFER* sbuf) { 
  i.init(sbuf); 
  set_samples_per_second(sbuf->sample_rate());
  set_channels(sbuf->number_of_channels());
  incrTime = 1.0/samples_per_second();
}

void EFFECT_PULSE_GATE::process(void) {
  i.begin(); // iterate through all samples, one sample-frame at a
             // time (interleaved)
  while(!i.end()) {
    currentTime += incrTime;
    if (currentTime > period)
      {
	currentTime = 0.0;
      }
    if (currentTime > stopTime)
      {
	for(int n = 0; n < channels(); n++) {
	  *i.current(n) = 0.0;
	}
      }
    i.next();
  }
}

EFFECT_PULSE_GATE_BPM::EFFECT_PULSE_GATE_BPM (parameter_type bpm,
					      parameter_type ontime_percent) {
  set_parameter(1, bpm);
  set_parameter(2, ontime_percent);
}

void EFFECT_PULSE_GATE_BPM::set_parameter(int param, parameter_type value) {
  switch (param) {
  case 1: 
    pulsegate_rep.set_parameter(1, value / 60.0);
    break;
  case 2:
    pulsegate_rep.set_parameter(2, value);
    break;
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_PULSE_GATE_BPM::get_parameter(int param) const {
  switch (param) {
  case 1: 
    return (pulsegate_rep.get_parameter(1) * 60.0);
    break;
  case 2:
    return (pulsegate_rep.get_parameter(2));
    break;
  }
  return(0.0);
}

void EFFECT_PULSE_GATE_BPM::init(SAMPLE_BUFFER* sbuf) { pulsegate_rep.init(sbuf); }
void EFFECT_PULSE_GATE_BPM::process(void) { pulsegate_rep.process(); }

EFFECT_TREMOLO::EFFECT_TREMOLO (parameter_type freq_bpm,
				      parameter_type depth_percent) {
  set_parameter(1, freq_bpm);
  set_parameter(2, depth_percent);
  currentTime = 0.0;
}

void EFFECT_TREMOLO::set_parameter(int param, parameter_type value) {
  switch (param) {
  case 1:
    if (value > 0)
      {
	freq = value/(2*60); // convert from bpm to Hz
      }
    else
      {
	MESSAGE_ITEM otemp;
	otemp << "(audiofx_envelope_modulation) WARNING! bpm must be greater than 0! ";
	ecadebug->msg(otemp.to_string());
      }
    break;
  case 2:
	depth = (value/100.0); // from percent to fraction
    break;
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_TREMOLO::get_parameter(int param) const {
  switch (param) {
  case 1:
    return freq*120;
    break;
  case 2:
    return depth*100.0;
    break;
  }
  return(0.0);
}

void EFFECT_TREMOLO::init(SAMPLE_BUFFER* sbuf) {
  i.init(sbuf);
  set_samples_per_second(sbuf->sample_rate());
  set_channels(sbuf->number_of_channels());
  incrTime = 1.0/samples_per_second();
}

void EFFECT_TREMOLO::process(void) {
  i.begin(); // iterate through all samples, one sample-frame at a
             // time (interleaved)
  while(!i.end())
  {
    currentTime += incrTime;
    double envelope = (1-depth)+depth*fabs(sin(2*3.1416*currentTime*freq));
    if (envelope < 0)
    {
       envelope = 0;
    }
  	for(int n = 0; n < channels(); n++)
  	{
	    *i.current(n) *= envelope;
    }
    i.next();
  }
}
