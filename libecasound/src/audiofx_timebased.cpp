// ------------------------------------------------------------------------
// audiofx_timebased.cpp: Routines for time-based effects.
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

#include <string>

#include <kvutils.h>

#include "eca-debug.h"

#include "samplebuffer_iterators.h"
#include "audiofx_timebased.h"

EFFECT_DELAY::EFFECT_DELAY (DYNAMIC_PARAMETERS::parameter_type delay_time, int surround_mode, 
			    int num_of_delays, DYNAMIC_PARAMETERS::parameter_type mix_percent) 
{
  laskuri = 0.0;

  set_parameter(1, delay_time);
  set_parameter(2, surround_mode);
  set_parameter(3, num_of_delays);
  set_parameter(4, mix_percent);
}

DYNAMIC_PARAMETERS::parameter_type EFFECT_DELAY::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(dtime / (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() * 1000.0);
  case 2:
    return(surround);
  case 3:
    return(dnum);
  case 4:
    return(mix * 100.0);
  }
  return(0.0);
}

void EFFECT_DELAY::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1:
    {
      dtime = value * (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() / 1000;
      vector<vector<SINGLE_BUFFER> >::iterator p = buffer.begin();
      while(p != buffer.end()) {
	vector<SINGLE_BUFFER>::iterator q = p->begin();
	while(q != p->end()) {
	  if (q->size() > dtime) {
	    q->resize(dtime);
	    laskuri = dtime;
	  }
	  ++q;
	}
	++p;
      }
      break;
    }

  case 2: 
    surround = value;
    break;

  case 3: 
    {
      if (value != 0.0) dnum = value;
      else dnum = 1.0;
      vector<vector<SINGLE_BUFFER> >::iterator p = buffer.begin();
      while(p != buffer.end()) {
	p->resize(dnum);
	++p;
      }
      laskuri = 0;
      break;
    }

  case 4:
    mix = value / 100.0;
    break;
  }
}

void EFFECT_DELAY::init(SAMPLE_BUFFER* insample) {
  l.init(insample);
  r.init(insample);

  set_channels(insample->number_of_channels());
  set_samples_per_second(insample->sample_rate());

  buffer.resize(2, vector<SINGLE_BUFFER> (dnum));
}

void EFFECT_DELAY::process(void) {
  l.begin(SAMPLE_SPECS::ch_left);
  r.begin(SAMPLE_SPECS::ch_right);

  while(!l.end() && !r.end()) {
    SAMPLE_SPECS::sample_type temp2_left = 0.0;
    SAMPLE_SPECS::sample_type temp2_right = 0.0;

    for(int nm2 = 0; nm2 < dnum; nm2++) {
      SAMPLE_SPECS::sample_type temp_left = 0.0;
      SAMPLE_SPECS::sample_type temp_right = 0.0;

      if (laskuri >= dtime * (nm2 + 1)) {
 
	switch ((int)surround) {
	case 0: 
	  {
	    // ---
	    // surround
	    temp_left = buffer[SAMPLE_SPECS::ch_left][nm2].front();
	    temp_right = buffer[SAMPLE_SPECS::ch_right][nm2].front();
	    break;
	  }

	case 1: 
	  {
	    // ---
	    // surround
	    temp_left = buffer[SAMPLE_SPECS::ch_right][nm2].front();
	    temp_right = buffer[SAMPLE_SPECS::ch_left][nm2].front();
	    break;
	  }
	case 2: 
	  {
	    if (nm2 % 2 == 0) {
	      temp_left = (buffer[SAMPLE_SPECS::ch_left][nm2].front()
			   + 
			   buffer[SAMPLE_SPECS::ch_right][nm2].front()) / 2.0;
	      temp_right = 0.0;
	    }
	    else {
	      temp_right = (buffer[SAMPLE_SPECS::ch_left][nm2].front()
			   + 
			   buffer[SAMPLE_SPECS::ch_right][nm2].front()) / 2.0;
	      temp_left = 0.0;
	    }
	    break;
	}
	} // switch
	buffer[SAMPLE_SPECS::ch_left][nm2].pop_front();
	buffer[SAMPLE_SPECS::ch_right][nm2].pop_front();
      }
      buffer[SAMPLE_SPECS::ch_left][nm2].push_back(*l.current());
      buffer[SAMPLE_SPECS::ch_right][nm2].push_back(*r.current());

      temp2_left += temp_left / dnum;
      temp2_right += temp_right / dnum;

    }
    *l.current() = (*l.current() * (1.0 - mix)) + (temp2_left * mix);
    *r.current() = (*r.current() * (1.0 - mix)) + (temp2_right * mix);

    l.next();
    r.next();

    if (laskuri < dtime * dnum) laskuri++;
  }
}

EFFECT_MULTITAP_DELAY::EFFECT_MULTITAP_DELAY (DYNAMIC_PARAMETERS::parameter_type delay_time, 
					      int num_of_delays, 
					      DYNAMIC_PARAMETERS::parameter_type mix_percent)
  : 
  delay_index(0),
  filled (0),
  buffer (0) {
  set_parameter(1, delay_time);
  set_parameter(2, num_of_delays);
  set_parameter(3, mix_percent);
}

DYNAMIC_PARAMETERS::parameter_type EFFECT_MULTITAP_DELAY::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(dtime / (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() * 1000.0);
  case 2:
    return(dnum);
  case 3:
    return(mix * 100.0);
  }
  return(0.0);
}

void EFFECT_MULTITAP_DELAY::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1:
    {
      dtime = static_cast<long int>(value * (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() / 1000);
      assert(buffer.size() == filled.size());
      for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
	if ((dtime * dnum) > static_cast<int>(buffer[n].size())) {
	  buffer[n].resize(dtime * dnum);
	}
	delay_index = dtime * dnum - 1;
	filled[n] = false;
      }
      break;
    }

  case 2: 
    {
      if (value != 0.0) dnum = static_cast<long int>(value);
      else dnum = 1;
      assert(buffer.size() == filled.size());
      for(int n = 0; n < static_cast<int>(buffer.size()); n++) {
	if ((dtime * dnum) > static_cast<int>(buffer[n].size())) {
	  buffer[n].resize(dtime * dnum);
	}
	filled[n] = false;
      }
      delay_index = dtime * dnum - 1;
      break;
    }

  case 3:
    mix = value / 100.0;
    break;
  }
}

void EFFECT_MULTITAP_DELAY::init(SAMPLE_BUFFER* insample) {
  i.init(insample);

  set_channels(insample->number_of_channels());
  set_samples_per_second(insample->sample_rate());

  delay_index = dtime * dnum - 1;

  filled.resize(channels(), bool (false));
  buffer.resize(channels(), vector<SAMPLE_SPECS::sample_type> (dtime *
							       dnum));
}

void EFFECT_MULTITAP_DELAY::process(void) {
  long int len = dtime * dnum;
  i.begin();
  while(!i.end()) {
    SAMPLE_SPECS::sample_type temp1 = 0.0;
    for(int nm2 = 0; nm2 < dnum; nm2++) {
      if (filled[nm2] == true) {
	assert((delay_index + nm2 * dtime) % len >= 0);
	assert((delay_index + nm2 * dtime) % len < len);
	temp1 += buffer[i.channel()][(delay_index + nm2 * dtime) % len];
      }
    }
    buffer[i.channel()][delay_index] = *i.current();
    *i.current() = (*i.current() * (1.0 - mix)) + (temp1 * mix / dnum);
    if (i.channel() + 1 == channels()) {
      --delay_index;
      for(int nm2 = 0; nm2 < dnum; nm2++) {
	if (delay_index < len - dtime * nm2) filled[nm2] = true;
      }
      if (delay_index == -1) delay_index = len - 1;
    }
    i.next();
  }
}

EFFECT_FAKE_STEREO::EFFECT_FAKE_STEREO (DYNAMIC_PARAMETERS::parameter_type delay_time) {
   set_parameter(1, delay_time);
}

DYNAMIC_PARAMETERS::parameter_type EFFECT_FAKE_STEREO::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(dtime / (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() * 1000.0);
  }
  return(0.0);
}

void EFFECT_FAKE_STEREO::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1:
    dtime = value * (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() / 1000;
    vector<deque<SAMPLE_SPECS::sample_type> >::iterator p = buffer.begin();
    while(p != buffer.end()) {
      if (p->size() > dtime) {
	p->resize(dtime);
      }
      ++p;
    }
    break;
  }
}

void EFFECT_FAKE_STEREO::init(SAMPLE_BUFFER* insample) {
  l.init(insample);
  r.init(insample);

  set_channels(insample->number_of_channels());
  set_samples_per_second(insample->sample_rate());

  buffer.resize(2);
}

void EFFECT_FAKE_STEREO::process(void) {
  l.begin(SAMPLE_SPECS::ch_left);
  r.begin(SAMPLE_SPECS::ch_right);
  while(!l.end() && !r.end()) {
    SAMPLE_SPECS::sample_type temp_left = 0;
    SAMPLE_SPECS::sample_type temp_right = 0;
    if (buffer[SAMPLE_SPECS::ch_left].size() >= dtime) {
      temp_left = buffer[SAMPLE_SPECS::ch_left].front();
      temp_right = buffer[SAMPLE_SPECS::ch_right].front();

      temp_right = (temp_left + temp_right) / 2.0;
      temp_left = (*l.current() + *r.current()) / 2.0;

      buffer[SAMPLE_SPECS::ch_left].pop_front();
      buffer[SAMPLE_SPECS::ch_right].pop_front();
    }
    else {
      temp_left = (*l.current() + *r.current()) / 2.0;
      temp_right = 0.0;
    }
    buffer[SAMPLE_SPECS::ch_left].push_back(*l.current());
    buffer[SAMPLE_SPECS::ch_right].push_back(*r.current());

    *l.current() = temp_left;
    *r.current() = temp_right;

    l.next();
    r.next();
  }
}

EFFECT_REVERB::EFFECT_REVERB (DYNAMIC_PARAMETERS::parameter_type delay_time, int surround_mode, 
			      DYNAMIC_PARAMETERS::parameter_type feedback_percent) 
{
  set_parameter(1, delay_time);
  set_parameter(2, surround_mode);
  set_parameter(3, feedback_percent);
}

DYNAMIC_PARAMETERS::parameter_type EFFECT_REVERB::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(dtime / (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() * 1000.0);
  case 2:
    return(surround);
  case 3:
    return(feedback * 100.0);
  }
  return(0.0);
}

void EFFECT_REVERB::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1: 
    {
      dtime = value * (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() / 1000;
      vector<deque<SAMPLE_SPECS::sample_type> >::iterator p = buffer.begin();
      while(p != buffer.end()) {
	if (p->size() > dtime) {
	  p->resize(dtime);
	}
	++p;
      }
      break;
    }

  case 2: 
    surround = value;
    break;

  case 3: 
    feedback = value / 100.0;
    break;
  }
}

void EFFECT_REVERB::init(SAMPLE_BUFFER* insample) {
  l.init(insample);
  r.init(insample);

  set_channels(insample->number_of_channels());
  set_samples_per_second(insample->sample_rate());

  buffer.resize(2);
}

void EFFECT_REVERB::process(void) {
  l.begin(SAMPLE_SPECS::ch_left);
  r.begin(SAMPLE_SPECS::ch_right);
  while(!l.end() && !r.end()) {
    SAMPLE_SPECS::sample_type temp_left = 0.0;
    SAMPLE_SPECS::sample_type temp_right = 0.0;
    if (buffer[SAMPLE_SPECS::ch_left].size() >= dtime) {
      temp_left = buffer[SAMPLE_SPECS::ch_left].front();
      temp_right = buffer[SAMPLE_SPECS::ch_right].front();
      
      if (surround == 0) {
	*l.current() = (*l.current() * (1 - feedback)) + (temp_left *  feedback);
	*r.current() = (*r.current() * (1 - feedback)) + (temp_right * feedback);
      }
      else {
	*l.current() = (*l.current() * (1 - feedback)) + (temp_right *  feedback);
	*r.current() = (*r.current() * (1 - feedback)) + (temp_left * feedback);
      }
      buffer[SAMPLE_SPECS::ch_left].pop_front();
      buffer[SAMPLE_SPECS::ch_right].pop_front();
    }
    else {
	*l.current() = (*l.current() * (1 - feedback));
	*r.current() = (*r.current() * (1 - feedback));
    }
    buffer[SAMPLE_SPECS::ch_left].push_back(*l.current());
    buffer[SAMPLE_SPECS::ch_right].push_back(*r.current());

    l.next();
    r.next();
  }
}

DYNAMIC_PARAMETERS::parameter_type EFFECT_FLANGER::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(dtime / (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() * 1000.0);

  case 2: 
    return(vartime);

  case 3: 
    return(feedback * 100.0);

  case 4: 
    return(lfo.get_parameter(1));
  }
  return(0.0);
}

void EFFECT_FLANGER::set_parameter(int param, DYNAMIC_PARAMETERS::parameter_type value) {
  switch (param) {
  case 1:
    {
      dtime = static_cast<long int>(value * (DYNAMIC_PARAMETERS::parameter_type)samples_per_second() / 1000);
      vector<vector<SAMPLE_SPECS::sample_type> >::iterator p = buffer.begin();
      while(p != buffer.end()) {
	if (dtime > static_cast<long int>(p->size())) {
	  p->resize(dtime);
	}
	++p;
      }
      delay_index = dtime - 1;
      filled = false;
      break;
    }

  case 2: 
    vartime = value;
    break;

  case 3: 
    feedback = value / 100.0;
    break;

  case 4:
    lfo.set_parameter(1, value);
    break;
  }
}

void EFFECT_FLANGER::init(SAMPLE_BUFFER* insample) {
  i.init(insample);
  lfo.init(1.0 /
	   insample->sample_rate());

  set_channels(insample->number_of_channels());
  set_samples_per_second(insample->sample_rate());
  delay_index = dtime - 1;
  filled = false;

  buffer.resize(channels(), vector<SAMPLE_SPECS::sample_type> (dtime));
}

void EFFECT_FLANGER::process(void) {
  i.begin();
  parameter_type p = vartime * lfo.value();
  while(!i.end()) {
    SAMPLE_SPECS::sample_type temp1 = 0.0;
    if (filled == true) {
      assert((delay_index + static_cast<long int>(p)) % dtime >= 0);
      assert((delay_index + static_cast<long int>(p)) % dtime < static_cast<long int>(buffer[i.channel()].size()));
      temp1 = buffer[i.channel()][(delay_index + static_cast<long int>(p)) % dtime];
//        cerr << "b: "
//  	   << (delay_index + static_cast<long int>(p)) % dtime
//  	   << "," << p << ".\n";
    }
    *i.current() = (*i.current() * (1.0 - feedback)) + (temp1 * feedback);
    buffer[i.channel()][delay_index] = *i.current();
    if (i.channel() + 1 == channels()) {
      p = vartime * lfo.value();
      --delay_index;
      if (delay_index == -1) {
	delay_index = dtime - 1;
	filled = true;
      }
    }
    i.next();
  }
}
