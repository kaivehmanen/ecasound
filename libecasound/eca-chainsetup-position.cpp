// ------------------------------------------------------------------------
// eca-chainsetup-position.cpp: Global chainsetup position
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

ECA_CHAINSETUP_POSITION::ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_t srate) {
  srate_rep = srate;
  length_set_rep = false;
  looping_rep = false;
  curpos_rep = 0;
  length_rep = 0;
}

void ECA_CHAINSETUP_POSITION::length_in_samples(SAMPLE_SPECS::sample_pos_t pos) { 
  length_rep = pos;
  length_set_rep = true;
  if (pos == 0) length_set_rep = false;
}

void ECA_CHAINSETUP_POSITION::change_position_exact(double seconds) {
 change_position(static_cast<SAMPLE_SPECS::sample_pos_t>(seconds * srate_rep));
}

void ECA_CHAINSETUP_POSITION::set_position_exact(double seconds) {
 set_position(static_cast<SAMPLE_SPECS::sample_pos_t>(seconds * srate_rep));
}

void ECA_CHAINSETUP_POSITION::length_in_seconds(double pos_in_seconds) { 
  length_in_samples(pos_in_seconds * srate_rep);
}

long int ECA_CHAINSETUP_POSITION::length_in_seconds(void) const {
  return(length_rep / srate_rep);
}

long int ECA_CHAINSETUP_POSITION::position_in_seconds(void) const {
  return(curpos_rep / srate_rep);
}

double ECA_CHAINSETUP_POSITION::length_in_seconds_exact(void) const {
  return(static_cast<double>(length_rep) / srate_rep);
}

double ECA_CHAINSETUP_POSITION::position_in_seconds_exact(void) const {
  return(static_cast<double>(curpos_rep) / srate_rep);
}
