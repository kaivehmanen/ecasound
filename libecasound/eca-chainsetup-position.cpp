// ------------------------------------------------------------------------
// eca-chainsetup-position.cpp: Global chainsetup position
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
#include "eca-chainsetup-position.h"

ECA_CHAINSETUP_POSITION::ECA_CHAINSETUP_POSITION(void)
{
  length_set_rep = false;
  looping_rep = false;
  curpos_rep = 0;
  length_rep = 0;
}

void ECA_CHAINSETUP_POSITION::length_in_samples(SAMPLE_SPECS::sample_pos_t pos)
{ 
  length_rep = pos;
  length_set_rep = true;
  if (pos == 0) length_set_rep = false;
}

void ECA_CHAINSETUP_POSITION::change_position_exact(double seconds)
{
 change_position(static_cast<SAMPLE_SPECS::sample_pos_t>(seconds * samples_per_second()));
}

void ECA_CHAINSETUP_POSITION::set_position_exact(double seconds)
{
 set_position(static_cast<SAMPLE_SPECS::sample_pos_t>(seconds * samples_per_second()));
}

void ECA_CHAINSETUP_POSITION::length_in_seconds(double pos_in_seconds)
{ 
  length_in_samples(static_cast<SAMPLE_SPECS::sample_pos_t>(pos_in_seconds * samples_per_second()));
}

long int ECA_CHAINSETUP_POSITION::length_in_seconds(void) const
{
  return(length_rep / samples_per_second());
}

long int ECA_CHAINSETUP_POSITION::position_in_seconds(void) const
{
  return(curpos_rep / samples_per_second());
}

double ECA_CHAINSETUP_POSITION::length_in_seconds_exact(void) const {
  return(static_cast<double>(length_rep) / samples_per_second());
}

double ECA_CHAINSETUP_POSITION::position_in_seconds_exact(void) const {
  return(static_cast<double>(curpos_rep) / samples_per_second());
}

void ECA_CHAINSETUP_POSITION::set_samples_per_second(SAMPLE_SPECS::sample_rate_t new_value)
{
  double ratio (new_value);
  ratio /= samples_per_second();
  set_position(static_cast<SAMPLE_SPECS::sample_pos_t>(position_in_samples() * ratio));
  if (length_set_rep == true) {
    length_in_samples(static_cast<SAMPLE_SPECS::sample_pos_t>(length_in_samples() * ratio));
  }
  ECA_SAMPLERATE_AWARE::set_samples_per_second(new_value);
}
