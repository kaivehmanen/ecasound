// ------------------------------------------------------------------------
// eca-audio-time.cpp: Generic class for representing time in audio 
//                     environment.
// Copyright (C) 2000,2007,2008 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
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
#include <cstdio>
#include <cmath>

#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "eca-audio-time.h"

ECA_AUDIO_TIME::ECA_AUDIO_TIME(SAMPLE_SPECS::sample_pos_t samples, 
			       SAMPLE_SPECS::sample_rate_t sample_rate)
{
  set_samples_per_second(sample_rate);
  set_samples(samples);
}

ECA_AUDIO_TIME::ECA_AUDIO_TIME(double time_in_seconds)
{
  set_seconds(time_in_seconds);
}

ECA_AUDIO_TIME::ECA_AUDIO_TIME(format_type type, 
			       const std::string& time)
{
  set(type, time);
}

ECA_AUDIO_TIME::ECA_AUDIO_TIME(const std::string& time)
{
  set_time_string(time);
}

ECA_AUDIO_TIME::ECA_AUDIO_TIME(void) : samples_rep(0),
				       sample_rate_rep(44100)
{
}

void ECA_AUDIO_TIME::set(format_type type, const std::string& time)
{
  switch(type) 
    {
      /* FIXME: not implemented! */
    case format_hour_min_sec: { DBC_CHECK(false); }
      /* FIXME: not implemented! */
    case format_min_sec: { DBC_CHECK(false); }
    case format_seconds: { samples_rep = static_cast<SAMPLE_SPECS::sample_pos_t>(sample_rate_rep * atof(time.c_str())); }
    case format_samples: { samples_rep = atol(time.c_str()); }

    default: { }
    }
}

void ECA_AUDIO_TIME::set_seconds(double seconds)
{
  samples_rep = static_cast<SAMPLE_SPECS::sample_pos_t>(seconds * sample_rate_rep);
}

void ECA_AUDIO_TIME::set_time_string(const std::string& time)
{
  if (time.size() > 2 &&
      time.find(time, time.size() - 2) != std::string::npos)
    ECA_AUDIO_TIME::set(format_samples, std::string(time, 0, time.size() - 2));
  else
    ECA_AUDIO_TIME::set(format_seconds, time);
}

/**
 * Sets the sample count.
 *
 * Note, this can change the value of seconds().
 */
void ECA_AUDIO_TIME::set_samples(SAMPLE_SPECS::sample_pos_t samples)
{
  samples_rep = samples;
}

/**
 * Sets samples per second.
 *
 * Note, this can change the value of seconds().
 */
void ECA_AUDIO_TIME::set_samples_per_second(SAMPLE_SPECS::sample_rate_t srate)
{
  sample_rate_rep = srate;
} 

/**
 * Sets samples per second.
 *
 * Note, this does NOT change the value of seconds()
 * like set_samples_per_second() potentially does.
 */
void ECA_AUDIO_TIME::set_samples_per_second_keeptime(SAMPLE_SPECS::sample_rate_t srate)
{
  if (sample_rate_rep != srate) {
    double time_secs = seconds();
    set_samples_per_second(srate);
    set_seconds(time_secs);
  }
}
  
std::string ECA_AUDIO_TIME::to_string(format_type type) const
{
  /* FIXME: not implemented */

  switch(type) 
    {
    case format_hour_min_sec: 
      { 
	return "";
      }
    case format_min_sec: 
      {
	return "";
      }
    case format_seconds: { return kvu_numtostr(seconds(), 3); }
    case format_samples: { return kvu_numtostr(samples_rep); }

    default: { }
    }

  return "";
}

double ECA_AUDIO_TIME::seconds(void) const
{
  return static_cast<double>(samples_rep) / sample_rate_rep;
}

SAMPLE_SPECS::sample_rate_t ECA_AUDIO_TIME::samples_per_second(void) const
{
  return sample_rate_rep;
}

SAMPLE_SPECS::sample_pos_t ECA_AUDIO_TIME::samples(void) const
{
  return samples_rep;
}
