// ------------------------------------------------------------------------
// audiofx_analysis.cpp: Classes for signal analysis
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

#include <cmath>

#include <kvu_dbc.h>
#include <kvu_message_item.h>
#include <kvu_numtostr.h>

#include "samplebuffer_iterators.h"
#include "audiofx_analysis.h"

#include "eca-logger.h"
#include "eca-error.h"

/* 'max-(max/2^15)' */
const SAMPLE_SPECS::sample_t EFFECT_ANALYSIS::clip_amplitude =
                                SAMPLE_SPECS::max_amplitude - 
                                SAMPLE_SPECS::max_amplitude / 16384.0f;

const int EFFECT_VOLUME_BUCKETS::range_count = 16;

EFFECT_ANALYSIS::~EFFECT_ANALYSIS(void)
{
}


EFFECT_VOLUME_BUCKETS::EFFECT_VOLUME_BUCKETS (void)
{
  reset_stats();
  cumulativemode_rep = false;
  lock_repp = new pthread_mutex_t;
  int res = pthread_mutex_init(lock_repp, NULL);
  DBC_CHECK(res == 0);
}

EFFECT_VOLUME_BUCKETS::~EFFECT_VOLUME_BUCKETS (void)
{
  delete lock_repp;
}

void EFFECT_VOLUME_BUCKETS::status_entry(int range, std::string& otemp) const
{
  /* note: is called with 'lock_repp' taken */

  for(int n = 0; n < channels(); n++) {
    otemp += "\t " + kvu_numtostr(ranges[range][n]);
    if (cumulativemode_rep == true) {
      otemp += ",";
      otemp += kvu_numtostr(100.0f *  
			    ranges[range][n] / 
			    num_of_samples[n], 3) + "%\t";
    }
  }
} 

void EFFECT_VOLUME_BUCKETS::reset_stats(void)
{
  for(unsigned int nm = 0; nm < ranges.size(); nm++)
    for(unsigned int ch = 0; ch < ranges[nm].size(); ch++)
      ranges[nm][ch] = 0;
  for(unsigned int nm = 0; nm < num_of_samples.size(); nm++)
    num_of_samples[nm] = 0;
  max_pos = max_neg = 0.0f;
  max_pos_period = max_neg_period = 0.0f;
  clipped_pos_period =  clipped_neg_period = 0;
  clipped_pos = clipped_neg = 0;
}

string EFFECT_VOLUME_BUCKETS::status(void) const
{
  int res = pthread_mutex_lock(lock_repp);
  DBC_CHECK(res == 0);

  status_rep = "(audiofx) -- Amplitude statistics -----------------------------\n";
  status_rep += "Range, pos/neg, count,(%), ch1...n";

  status_rep += "\nPos   -1.0 dB: "; status_entry(0, status_rep);
  status_rep += "\nPos   -2.0 dB: "; status_entry(1, status_rep);
  status_rep += "\nPos   -4.0 dB: "; status_entry(2, status_rep);
  status_rep += "\nPos   -8.0 dB: "; status_entry(3, status_rep);
  status_rep += "\nPos  -16.0 dB: "; status_entry(4, status_rep);
  status_rep += "\nPos  -32.0 dB: "; status_entry(5, status_rep);
  status_rep += "\nPos  -64.0 dB: "; status_entry(6, status_rep);
  status_rep += "\nPos -inf.0 dB: "; status_entry(7, status_rep);
  status_rep += "\nNeg -inf.0 dB: "; status_entry(8, status_rep);
  status_rep += "\nNeg  -64.0 dB: "; status_entry(9, status_rep);
  status_rep += "\nNeg  -32.0 dB: "; status_entry(10, status_rep);
  status_rep += "\nNeg  -16.0 dB: "; status_entry(11, status_rep);
  status_rep += "\nNeg   -8.0 dB: "; status_entry(12, status_rep);
  status_rep += "\nNeg   -4.0 dB: "; status_entry(13, status_rep);
  status_rep += "\nNeg   -2.0 dB: "; status_entry(14, status_rep);
  status_rep += "\nNeg   -1.0 dB: "; status_entry(15, status_rep);

  status_rep += "\n(audiofx) Peak amplitude, period: pos=" + kvu_numtostr(max_pos_period,5) + " neg=" + kvu_numtostr(max_neg_period,5) + ".\n";
  status_rep += "(audiofx) Peak amplitude, all   : pos=" + kvu_numtostr(max_pos,5) + " neg=" + kvu_numtostr(max_neg,5) + ".\n";
  status_rep += "(audiofx) Clipped samples, period: pos=" + kvu_numtostr(clipped_pos_period) + " neg=" + kvu_numtostr(clipped_neg_period) + ".\n";
  status_rep += "(audiofx) Clipped samples, all   : pos=" + kvu_numtostr(clipped_pos) + " neg=" + kvu_numtostr(clipped_neg) + ".\n";
  status_rep += "(audiofx) Max gain without clipping, all: " + kvu_numtostr(max_multiplier(),5) + ".\n";

  if (cumulativemode_rep == true)
    status_rep += "(audiofx) -- End of statistics --------------------------------\n";
  else
    status_rep += "(audiofx) -- End of statistics (periodical counters reseted) --\n";

  if (cumulativemode_rep != true) {
    for(unsigned int nm = 0; nm < ranges.size(); nm++)
      for(unsigned int ch = 0; ch < ranges[nm].size(); ch++)
	ranges[nm][ch] = 0;
    for(unsigned int nm = 0; nm < num_of_samples.size(); nm++)
      num_of_samples[nm] = 0;
    max_pos_period = max_neg_period = 0.0f;
    clipped_pos_period =  clipped_neg_period = 0;
  }

  res = pthread_mutex_unlock(lock_repp);
  DBC_CHECK(res == 0);

  return(status_rep);
}

void EFFECT_VOLUME_BUCKETS::parameter_description(int param, 
						  struct PARAM_DESCRIPTION *pd) const
{
  switch(param) {
  case 1: 
    pd->default_value = 0;
    pd->description = get_parameter_name(param);
    pd->bounded_above = true;
    pd->upper_bound = 1.0;
    pd->bounded_below = true;
    pd->lower_bound = 0.0f;
    pd->toggled = true;
    pd->integer = true;
    pd->logarithmic = false;
    pd->output = false;
    break;

  case 2: 
    pd->default_value = 1.0f;
    pd->description = get_parameter_name(param);
    pd->bounded_above = false;
    pd->upper_bound = 0.0f;
    pd->bounded_below = false;
    pd->lower_bound = 0.0f;
    pd->toggled = false;
    pd->integer = false;
    pd->logarithmic = false;
    pd->output = true;
    break;
  }
}

void EFFECT_VOLUME_BUCKETS::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
  switch (param) {
  case 1: 
    if (value != 0)
      cumulativemode_rep = true;
    else
      cumulativemode_rep = false;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_VOLUME_BUCKETS::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    if (cumulativemode_rep == true) return(1.0);
    
  case 2:
    return(max_multiplier());
  }
  return(0.0);
}

CHAIN_OPERATOR::parameter_t EFFECT_VOLUME_BUCKETS::max_multiplier(void) const
{
  parameter_t k;
  SAMPLE_SPECS::sample_t max_peak = max_pos;
  if (max_neg > max_pos) max_peak = max_neg;
  if (max_peak != 0.0f) k = SAMPLE_SPECS::max_amplitude / max_peak;
  else k = 0.0f;
  if (k < 1.0f) k = 1.0f;
  return(k);
}

void EFFECT_VOLUME_BUCKETS::init(SAMPLE_BUFFER* insample)
{
  int res = pthread_mutex_lock(lock_repp);
  DBC_CHECK(res == 0);

  i.init(insample);
  set_channels(insample->number_of_channels());
  num_of_samples.resize(channels(), 0);
  ranges.resize(range_count, std::vector<unsigned long int> (channels()));

  res = pthread_mutex_unlock(lock_repp);
  DBC_CHECK(res == 0);
}

void EFFECT_VOLUME_BUCKETS::process(void)
{
  int res = pthread_mutex_trylock(lock_repp);
  if (res == 0) {
    i.begin();
    while(!i.end()) {
      num_of_samples[i.channel()]++;
      if (*i.current() >= 0) {
	if (*i.current() > max_pos) max_pos = *i.current();
	if (*i.current() > max_pos_period) max_pos_period = *i.current();
	if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.891f) {
	  if (*i.current() >= EFFECT_ANALYSIS::clip_amplitude) {
	    clipped_pos_period++; clipped_pos++;
	  }
	  ranges[0][i.channel()]++;  // 0-1dB
	}
	else if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.794f) ranges[1][i.channel()]++;  // 1-2dB
	else if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.631f) ranges[2][i.channel()]++; // 2-4dB
	else if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.398f) ranges[3][i.channel()]++; // 4-8dB
	else if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.158f) ranges[4][i.channel()]++; // 8-16dB
	else if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.025f) ranges[5][i.channel()]++; // 16-32dB
	else if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.001f) ranges[6][i.channel()]++; // 32-64dB
	else ranges[7][i.channel()]++; // 64-infdB
      }
      else {
	if (-(*i.current()) > max_neg) max_neg = -(*i.current());
	if (-(*i.current()) > max_neg_period) max_neg_period = -(*i.current());
	if (*i.current() < SAMPLE_SPECS::max_amplitude * -0.891f) {
	  if (*i.current() <= -EFFECT_ANALYSIS::clip_amplitude) {
	    clipped_neg_period++; clipped_neg++;
	  }
	  ranges[15][i.channel()]++;  // 0-1dB
	}
	else if (*i.current() < SAMPLE_SPECS::max_amplitude * -0.794f) ranges[14][i.channel()]++;  // 1-2dB
	else if (*i.current() < SAMPLE_SPECS::max_amplitude * -0.631f) ranges[13][i.channel()]++; // 2-4dB
	else if (*i.current() < SAMPLE_SPECS::max_amplitude * -0.398f) ranges[12][i.channel()]++; // 4-8dB
	else if (*i.current() < SAMPLE_SPECS::max_amplitude * -0.158f) ranges[11][i.channel()]++; // 8-16dB
	else if (*i.current() < SAMPLE_SPECS::max_amplitude * -0.025f) ranges[10][i.channel()]++; // 16-32dB
	else if (*i.current() < SAMPLE_SPECS::max_amplitude * -0.001f) ranges[9][i.channel()]++; // 32-64dB
	else ranges[8][i.channel()]++; // 64-infdB
      }
      i.next();
    }

    res = pthread_mutex_unlock(lock_repp);
    DBC_CHECK(res == 0);
  } 
  // else { std::cerr << "(audiofx_analysis) lock taken, skipping process().\n"; }
}

EFFECT_VOLUME_PEAK::EFFECT_VOLUME_PEAK (void)
{
  max_amplitude_repp = 0;
}

EFFECT_VOLUME_PEAK::~EFFECT_VOLUME_PEAK (void)
{
  if (max_amplitude_repp != 0) {
    delete[] max_amplitude_repp;
    max_amplitude_repp = 0;
  }
}

void EFFECT_VOLUME_PEAK::parameter_description(int param, 
					       struct PARAM_DESCRIPTION *pd) const
{
  if (param > 0 && param <= channels()) {
    pd->default_value = 0;
    pd->description = get_parameter_name(param);
    pd->bounded_above = false;
    pd->bounded_below = true;
    pd->lower_bound = 0.0f;
    pd->toggled = false;
    pd->integer = false;
    pd->logarithmic = false;
    pd->output = true;
  }
}

std::string EFFECT_VOLUME_PEAK::parameter_names(void) const
{
  string params;
  for(int n = 0; n < channels(); n++) {
    params += "peak-amplitude-ch" + kvu_numtostr(n + 1);
    if (n != channels()) params += ",";
  }
  return(params);
}

void EFFECT_VOLUME_PEAK::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
}

CHAIN_OPERATOR::parameter_t EFFECT_VOLUME_PEAK::get_parameter(int param) const
{
  if (param > 0 && param <= channels()) {
    parameter_t temp = max_amplitude_repp[param - 1];
    max_amplitude_repp[param - 1] = 0.0f;
    return(temp);
  }
  return(0.0f);
}

void EFFECT_VOLUME_PEAK::init(SAMPLE_BUFFER* insample)
{
  i.init(insample);
  if (max_amplitude_repp != 0) {
    delete[] max_amplitude_repp;
    max_amplitude_repp = 0;
  }
  max_amplitude_repp = new parameter_t [insample->number_of_channels()];
  set_channels(insample->number_of_channels());
}

void EFFECT_VOLUME_PEAK::process(void)
{
  i.begin();
  while(!i.end()) {
    SAMPLE_SPECS::sample_t abscurrent = std::fabs(*i.current());
    DBC_CHECK(i.channel() >= 0);
    DBC_CHECK(i.channel() < channels());
    if (abscurrent > max_amplitude_repp[i.channel()]) {
      max_amplitude_repp[i.channel()] = std::fabs(*i.current());
    }
    i.next();
  }
}

EFFECT_DCFIND::EFFECT_DCFIND (void)
{
}

string EFFECT_DCFIND::status(void) const
{
  MESSAGE_ITEM mitem;
  mitem.setprecision(5);
  mitem << "(audiofx) Optimal value for DC-adjust: ";
  mitem << get_deltafix(SAMPLE_SPECS::ch_left) << " (left), ";
  mitem << get_deltafix(SAMPLE_SPECS::ch_right) << " (right).";
  return(mitem.to_string());
}

string EFFECT_DCFIND::parameter_names(void) const
{
  std::vector<std::string> t;
  for(int n = 0; n < channels(); n++) {
    t.push_back("result-offset-ch" + kvu_numtostr(n));
  }
  return(kvu_vector_to_string(t, ","));
}

CHAIN_OPERATOR::parameter_t EFFECT_DCFIND::get_deltafix(int channel) const
{
  SAMPLE_SPECS::sample_t deltafix;

  if (channel >= static_cast<int>(pos_sum.size()) ||
      channel >= static_cast<int>(neg_sum.size())) return(0.0);

  if (pos_sum[channel] > neg_sum[channel]) deltafix = -(pos_sum[channel] - neg_sum[channel]) / num_of_samples[channel];
  else deltafix = (neg_sum[channel] - pos_sum[channel]) / num_of_samples[channel];

  return((CHAIN_OPERATOR::parameter_t)deltafix); 
}

void EFFECT_DCFIND::parameter_description(int param, 
					  struct PARAM_DESCRIPTION *pd) const
{
  pd->default_value = 0.0f;
  pd->description = get_parameter_name(param);
  pd->bounded_above = false;
  pd->upper_bound = 0.0f;
  pd->bounded_below = false;
  pd->lower_bound = 0.0f;
  pd->toggled = false;
  pd->integer = false;
  pd->logarithmic = false;
  pd->output = true;
}

void EFFECT_DCFIND::set_parameter(int param,
				  CHAIN_OPERATOR::parameter_t value)
{
}

CHAIN_OPERATOR::parameter_t EFFECT_DCFIND::get_parameter(int param) const
{
  return(get_deltafix(param));
}

void EFFECT_DCFIND::init(SAMPLE_BUFFER *insample)
{
  i.init(insample);
  set_channels(insample->number_of_channels());
  pos_sum.resize(channels());
  neg_sum.resize(channels());
  num_of_samples.resize(channels());
}

void EFFECT_DCFIND::process(void)
{
  i.begin();
  while(!i.end()) {
    tempval = *i.current();
    if (tempval > SAMPLE_SPECS::silent_value)
      pos_sum[i.channel()] += tempval;
    else
      neg_sum[i.channel()] += fabs(tempval);
    num_of_samples[i.channel()]++;
    i.next();
  }
}
