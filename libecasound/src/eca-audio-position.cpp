// ------------------------------------------------------------------------
// eca-audioposition.cpp: Base class for representing position and length
//                        of a audio stream.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include "eca-audio-position.h"

ECA_AUDIO_POSITION::ECA_AUDIO_POSITION(const ECA_AUDIO_FORMAT& fmt) : ECA_AUDIO_FORMAT(fmt) {
  position_in_samples_rep = 0;
  length_in_samples_rep = 0;
}

int ECA_AUDIO_POSITION::length_in_seconds(void) const { 
  return((int)ceil((double)length_in_samples() /
		   (double)samples_per_second()));  
}

double ECA_AUDIO_POSITION::length_in_seconds_exact(void) const { 
  return((double)length_in_samples() / (double)samples_per_second());  
}

void ECA_AUDIO_POSITION::length_in_samples(long pos) {
  length_in_samples_rep = pos;
}

void ECA_AUDIO_POSITION::length_in_seconds(int pos_in_seconds) { 
  length_in_seconds((double)pos_in_seconds); 
}

void ECA_AUDIO_POSITION::length_in_seconds(double pos_in_seconds) {
  length_in_samples(pos_in_seconds * samples_per_second());  
}

long ECA_AUDIO_POSITION::position_in_samples(void) const {
  return(position_in_samples_rep);
}

int ECA_AUDIO_POSITION::position_in_seconds(void) const { 
  return((int)ceil((double)position_in_samples() /
		   (double)samples_per_second())); 
}

double ECA_AUDIO_POSITION::position_in_seconds_exact(void) const {
  return((double)position_in_samples() / (double)samples_per_second()); 
}

void ECA_AUDIO_POSITION::position_in_samples(long pos) {
  position_in_samples_rep = pos;
  if (position_in_samples_rep < 0) {
    position_in_samples_rep = 0;
  }
}

void ECA_AUDIO_POSITION::position_in_samples_advance(long pos) {
  position_in_samples_rep += pos;
}

void ECA_AUDIO_POSITION::position_in_seconds(int pos_in_seconds) { 
  position_in_seconds((double)pos_in_seconds); 
}

void ECA_AUDIO_POSITION::position_in_seconds(double pos_in_seconds) {
  position_in_samples(pos_in_seconds * samples_per_second());
}

void ECA_AUDIO_POSITION::seek_first(void) { seek_position_in_samples(0); }
void ECA_AUDIO_POSITION::seek_last(void) { seek_position_in_samples(length_in_samples()); }

void ECA_AUDIO_POSITION::seek_position_in_samples(long pos_in_samples) { 
  position_in_samples(pos_in_samples);
  seek_position();
}

void ECA_AUDIO_POSITION::seek_position_in_samples_advance(long pos) { seek_position_in_samples(position_in_samples() + pos); }
void ECA_AUDIO_POSITION::seek_position_in_seconds(double pos_in_seconds) {
  seek_position_in_samples(pos_in_seconds *
			   samples_per_second());
}

