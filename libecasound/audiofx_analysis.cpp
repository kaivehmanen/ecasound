// ------------------------------------------------------------------------
// audiofx_analysis.cpp: Signal analyzing.
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
#include <kvu_numtostr.h>

#include "samplebuffer_iterators.h"
#include "audiofx_analysis.h"

#include "eca-logger.h"
#include "eca-error.h"

EFFECT_ANALYSIS::~EFFECT_ANALYSIS(void)
{
}

const int EFFECT_ANALYZE::range_count = 16;
const SAMPLE_SPECS::sample_t EFFECT_ANALYZE::clip_amplitude = SAMPLE_SPECS::max_amplitude - SAMPLE_SPECS::max_amplitude / 16384.0f; // max-(max/2^15)

EFFECT_ANALYZE::EFFECT_ANALYZE (void) { 
  reset_stats();
  cumulativemode_rep = false;
}

std::string EFFECT_ANALYZE::status_entry(int range) const {
  std::string res;
  for(int n = 0; n < channels(); n++) {
    res += "\t ";
    res += kvu_numtostr(ranges[range][n]);
    if (cumulativemode_rep == true) {
      res += ",";
      res += kvu_numtostr(100.0f *  
			  ranges[range][n] / 
			  num_of_samples[n]) + "%";
      res += "\t";
    }
  }
  return(res);
} 

void EFFECT_ANALYZE::reset_stats(void) {
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

string EFFECT_ANALYZE::status(void) const {
  MESSAGE_ITEM otemp;
  otemp << "(audiofx) -- Amplitude statistics -----------------------------\n";
  otemp << "Range, pos/neg, count,(%), ch1...n";
  otemp.setprecision(3);
  otemp << "\nPos   -1.0 dB: " << status_entry(0);
  otemp << "\nPos   -2.0 dB: " << status_entry(1);
  otemp << "\nPos   -4.0 dB: " << status_entry(2);
  otemp << "\nPos   -8.0 dB: " << status_entry(3);
  otemp << "\nPos  -16.0 dB: " << status_entry(4);
  otemp << "\nPos  -32.0 dB: " << status_entry(5);
  otemp << "\nPos  -64.0 dB: " << status_entry(6);
  otemp << "\nPos -inf.0 dB: " << status_entry(7);
  otemp << "\nNeg -inf.0 dB: " << status_entry(8);
  otemp << "\nNeg  -64.0 dB: " << status_entry(9);
  otemp << "\nNeg  -32.0 dB: " << status_entry(10);
  otemp << "\nNeg  -16.0 dB: " << status_entry(11);
  otemp << "\nNeg   -8.0 dB: " << status_entry(12);
  otemp << "\nNeg   -4.0 dB: " << status_entry(13);
  otemp << "\nNeg   -2.0 dB: " << status_entry(14);
  otemp << "\nNeg   -1.0 dB: " << status_entry(15);

  otemp.setprecision(5);
  otemp << "\n(audiofx) Peak amplitude, period: pos=" << max_pos_period << " neg=" << max_neg_period << ".\n";
  otemp << "(audiofx) Peak amplitude, all   : pos=" << max_pos << " neg=" << max_neg << ".\n";
  otemp << "(audiofx) Clipped samples, period: pos=" << clipped_pos_period << " neg=" << clipped_neg_period << ".\n";
  otemp << "(audiofx) Clipped samples, all   : pos=" << clipped_pos << " neg=" << clipped_neg << ".\n";
  otemp << "(audiofx) Max gain without clipping, all: " << max_multiplier() << ".\n";
  if (cumulativemode_rep == true)
    otemp << "(audiofx) -- End of statistics --------------------------------\n";
  else
    otemp << "(audiofx) -- End of statistics (periodical counters reseted) --\n";

  if (cumulativemode_rep != true) {
    for(unsigned int nm = 0; nm < ranges.size(); nm++)
      for(unsigned int ch = 0; ch < ranges[nm].size(); ch++)
	ranges[nm][ch] = 0;
    for(unsigned int nm = 0; nm < num_of_samples.size(); nm++)
      num_of_samples[nm] = 0;
    max_pos_period = max_neg_period = 0.0f;
    clipped_pos_period =  clipped_neg_period = 0;
  }
  return(otemp.to_string());
}

void EFFECT_ANALYZE::parameter_description(int param, 
					   struct PARAM_DESCRIPTION *pd) {
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

void EFFECT_ANALYZE::set_parameter(int param, CHAIN_OPERATOR::parameter_t value) {
  switch (param) {
  case 1: 
    if (value != 0)
      cumulativemode_rep = true;
    else
      cumulativemode_rep = false;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_ANALYZE::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    if (cumulativemode_rep == true) return(1.0);
    
  case 2:
    return(max_multiplier());
  }
  return(0.0);
}

CHAIN_OPERATOR::parameter_t EFFECT_ANALYZE::max_multiplier(void) const { 
  parameter_t k;
  SAMPLE_SPECS::sample_t max_peak = max_pos;
  if (max_neg > max_pos) max_peak = max_neg;
  if (max_peak != 0.0f) k = SAMPLE_SPECS::max_amplitude / max_peak;
  else k = 0.0f;
  if (k < 1.0f) k = 1.0f;
  return(k);
}

void EFFECT_ANALYZE::init(SAMPLE_BUFFER* insample) {
  i.init(insample);
  set_channels(insample->number_of_channels());
  num_of_samples.resize(channels(), 0);
  ranges.resize(range_count, std::vector<unsigned long int> (channels()));
}

void EFFECT_ANALYZE::process(void) {
  i.begin();
  while(!i.end()) {
    num_of_samples[i.channel()]++;
    if (*i.current() >= 0) {
      if (*i.current() > max_pos) max_pos = *i.current();
      if (*i.current() > max_pos_period) max_pos_period = *i.current();
      if (*i.current() > SAMPLE_SPECS::max_amplitude * 0.891f) {
	if (*i.current() >= clip_amplitude) {
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
	if (*i.current() <= -clip_amplitude) {
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
}

EFFECT_DCFIND::EFFECT_DCFIND (void) { }

string EFFECT_DCFIND::status(void) const {
    MESSAGE_ITEM mitem;
    mitem.setprecision(5);
    mitem << "(audiofx) Optimal value for DC-adjust: ";
    mitem << get_deltafix(SAMPLE_SPECS::ch_left) << " (left), ";
    mitem << get_deltafix(SAMPLE_SPECS::ch_right) << " (right).";
    return(mitem.to_string());
}

string EFFECT_DCFIND::parameter_names(void) const {
  std::vector<std::string> t;
  for(int n = 0; n < channels(); n++) {
    t.push_back("result-offset-ch" + kvu_numtostr(n));
  }
  return(kvu_vector_to_string(t, ","));
}

CHAIN_OPERATOR::parameter_t EFFECT_DCFIND::get_deltafix(int channel) const { 
  SAMPLE_SPECS::sample_t deltafix;

  if (channel >= static_cast<int>(pos_sum.size()) ||
      channel >= static_cast<int>(neg_sum.size())) return(0.0);

  if (pos_sum[channel] > neg_sum[channel]) deltafix = -(pos_sum[channel] - neg_sum[channel]) / num_of_samples[channel];
  else deltafix = (neg_sum[channel] - pos_sum[channel]) / num_of_samples[channel];

  return((CHAIN_OPERATOR::parameter_t)deltafix); 
}

void EFFECT_DCFIND::parameter_description(int param, 
					  struct PARAM_DESCRIPTION *pd) {
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
				 CHAIN_OPERATOR::parameter_t value) { }

CHAIN_OPERATOR::parameter_t EFFECT_DCFIND::get_parameter(int param) const {
  return(get_deltafix(param));
}

void EFFECT_DCFIND::init(SAMPLE_BUFFER *insample) {
  i.init(insample);
  set_channels(insample->number_of_channels());
  pos_sum.resize(channels());
  neg_sum.resize(channels());
  num_of_samples.resize(channels());
}

void EFFECT_DCFIND::process(void) {
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
