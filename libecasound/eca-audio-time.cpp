// ------------------------------------------------------------------------
// eca-audio-time.cpp: Generic class for representing time in audio 
//                     environment.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <kvutils/kvu_numtostr.h>

#include "eca-audio-time.h"

ECA_AUDIO_TIME::ECA_AUDIO_TIME(long int samples, long int sample_rate) {
  set_samples_per_second(sample_rate);
  set_samples(samples);
}

ECA_AUDIO_TIME::ECA_AUDIO_TIME(double time_in_seconds) {
  set_seconds(time_in_seconds);
}

ECA_AUDIO_TIME::ECA_AUDIO_TIME(format_type type, const string& time) {
  set(type, time);
}

ECA_AUDIO_TIME::ECA_AUDIO_TIME(void) : samples_rep(0), sample_rate_rep(44100) { }

void ECA_AUDIO_TIME::set(format_type type, const string& time) {
  switch(type) 
    {
    case format_hour_min_sec: { }
    case format_min_sec: { }
    case format_seconds: { samples_rep = static_cast<long int>(sample_rate_rep * atof(time.c_str())); }
    case format_samples: { samples_rep = atol(time.c_str()); }

    default: { }
    }
}

void ECA_AUDIO_TIME::set_seconds(double seconds) {
  samples_rep = static_cast<long int>(seconds * sample_rate_rep);
}

void ECA_AUDIO_TIME::set_samples(long int samples) { samples_rep = samples; } 
void ECA_AUDIO_TIME::set_samples_per_second(long int srate)  { sample_rate_rep = srate; } 
  
string ECA_AUDIO_TIME::to_string(format_type type) const {
  switch(type) 
    {
    case format_hour_min_sec: 
      { 
	return("");
      }
    case format_min_sec: 
      {
	return("");
      }
    case format_seconds: { return(kvu_numtostr(seconds(), 3)); }
    case format_samples: { return(kvu_numtostr(samples_rep)); }

    default: { }
    }
  return("");
}

double ECA_AUDIO_TIME::seconds(void) const {
  return(static_cast<double>(samples_rep) / sample_rate_rep);
}

long int ECA_AUDIO_TIME::samples_per_second(void) const  { return(sample_rate_rep); }
long int ECA_AUDIO_TIME::samples(void) const { return(samples_rep); }
