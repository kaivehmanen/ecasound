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

#include <kvutils/message_item.h>

#include "samplebuffer_iterators.h"
#include "audiofx_analysis.h"

#include "eca-debug.h"
#include "eca-error.h"

EFFECT_ANALYZE::EFFECT_ANALYZE (void) { max = 0; }

string EFFECT_ANALYZE::status(void) const {
  MESSAGE_ITEM otemp;
  otemp << "(audiofx) -- Printing volume statistics --\n";
  for(int nm = 0; nm < static_cast<int>(ranges.size()); nm++) {
    otemp.setprecision(2);
    otemp << "(audiofx) Vol-range: ";
    otemp << nm << "\t L: ";
    assert(SAMPLE_SPECS::ch_left < ranges[nm].size());
    otemp << ranges[nm][SAMPLE_SPECS::ch_left] << " (";
    otemp << ((CHAIN_OPERATOR::parameter_type)ranges[nm][SAMPLE_SPECS::ch_left] / (CHAIN_OPERATOR::parameter_type)num_of_samples[SAMPLE_SPECS::ch_left] * 100.0);
    otemp << ") \t\t";
    otemp << "R: ";
    assert(SAMPLE_SPECS::ch_right < ranges[nm].size());
    otemp << ranges[nm][SAMPLE_SPECS::ch_right] << " (";
    otemp << ((CHAIN_OPERATOR::parameter_type)ranges[nm][SAMPLE_SPECS::ch_right] / (CHAIN_OPERATOR::parameter_type)num_of_samples[SAMPLE_SPECS::ch_right] * 100.0);
    otemp << ").\n";
  
  }
  otemp << "(audiofx) -- End of statistics (counters reseted)--\n";
  otemp.setprecision(5);
  otemp << "(audiofx) Max amplitude " << max;
  otemp << "; signal can be amplified by " << max_multiplier() * 100.0;
  otemp << "%.";

  for(int nm = 0; nm < static_cast<int>(ranges.size()); nm++)
    for(int ch = 0; ch < static_cast<int>(ranges[0].size()); ch++)
      ranges[nm][ch] = 0;
  for(int ch = 0; ranges.size() > 0 && ch < static_cast<int>(ranges[0].size()); ch++)
    num_of_samples[ch] = 0;

  max = 0;

  return(otemp.to_string());
}

CHAIN_OPERATOR::parameter_type EFFECT_ANALYZE::max_multiplier(void) const { 
  CHAIN_OPERATOR::parameter_type kerroin;
  
  if (max != 0.0) kerroin = SAMPLE_SPECS::max_amplitude / max;
  else kerroin = 0.0;

  if (kerroin < 1.0) kerroin = 1.0;

  return(kerroin);
}

void EFFECT_ANALYZE::init(SAMPLE_BUFFER* insample) {
  i.init(insample);
  if (insample->number_of_channels() < 2) {
    num_of_samples.resize(2, 0);
    ranges.resize(range_count, vector<unsigned long int> (2));
  }
  else {
    num_of_samples.resize(insample->number_of_channels(), 0);
    ranges.resize(range_count, vector<unsigned long int> (insample->number_of_channels()));
  }
}

void EFFECT_ANALYZE::process(void) {
  i.begin();
  while(!i.end()) {
    if (fabs(*i.current() > max)) max = fabs(*i.current());
    num_of_samples[i.channel()]++;
    if (*i.current() <  SAMPLE_SPECS::max_amplitude * -7.0/8.0) ranges[0][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * -6.0/8.0) ranges[1][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * -5.0/8.0) ranges[2][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * -4.0/8.0) ranges[3][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * -3.0/8.0) ranges[4][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * -2.0/8.0) ranges[5][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * -1.0/8.0) ranges[6][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::silent_value) ranges[7][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * 1.0/8.0) ranges[8][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * 2.0/8.0) ranges[9][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * 3.0/8.0) ranges[10][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * 4.0/8.0) ranges[11][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * 5.0/8.0) ranges[12][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * 6.0/8.0) ranges[13][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude * 7.0/8.0) ranges[14][i.channel()]++;
    else if (*i.current() < SAMPLE_SPECS::max_amplitude) ranges[15][i.channel()]++;
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

CHAIN_OPERATOR::parameter_type EFFECT_DCFIND::get_deltafix(int channel) const { 
  SAMPLE_SPECS::sample_type deltafix;

  if (channel >= pos_sum.size() ||
      channel >= neg_sum.size()) return(0.0);

  if (pos_sum[channel] > neg_sum[channel]) deltafix = -(pos_sum[channel] - neg_sum[channel]) / num_of_samples[channel];
  else deltafix = (neg_sum[channel] - pos_sum[channel]) / num_of_samples[channel];

  return((CHAIN_OPERATOR::parameter_type)deltafix); 
}

void EFFECT_DCFIND::init(SAMPLE_BUFFER *insample) {
  i.init(insample);

  pos_sum.resize(insample->number_of_channels());
  neg_sum.resize(insample->number_of_channels());
  num_of_samples.resize(insample->number_of_channels());
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
