// ------------------------------------------------------------------------
// eca-control-position.cpp: Global position controller
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
#include "eca-control-position.h"

ECA_CONTROL_POSITION::ECA_CONTROL_POSITION(long int srate) {
  srate_rep = srate;
  length_set_rep = false;
  looping_rep = false;
  curpos_rep = 0;
  length_rep = 0;
}

void ECA_CONTROL_POSITION::length_in_samples(long pos) { 
  length_rep = pos;
  length_set_rep = true;
  if (pos == 0) length_set_rep = false;
}

void ECA_CONTROL_POSITION::change_position(double  samples) {
 change_position(static_cast<long>(samples * srate_rep));
}

void ECA_CONTROL_POSITION::length_in_seconds(double pos_in_seconds) { 
  length_in_samples(pos_in_seconds * srate_rep);
}

long int ECA_CONTROL_POSITION::length_in_seconds(void) const {
  return(static_cast<double>(length_rep) / srate_rep);
}

long int ECA_CONTROL_POSITION::position_in_seconds(void) const {
  return(static_cast<double>(curpos_rep) / srate_rep);
}

double ECA_CONTROL_POSITION::length_in_seconds_exact(void) const {
  return(static_cast<double>(length_rep) / srate_rep);
}

double ECA_CONTROL_POSITION::position_in_seconds_exact(void) const {
  return(static_cast<double>(curpos_rep) / srate_rep);
}
