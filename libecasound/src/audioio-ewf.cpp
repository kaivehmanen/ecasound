// ------------------------------------------------------------------------
// audioio-ewf.cpp: Ecasound wave format input/output
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

#include <algorithm>
#include <string>
#include <iostream.h>
#include <fstream.h>
#include <cmath>

#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio-ewf.h"

#include "eca-error.h"
#include "eca-debug.h"
  
EWFFILE::EWFFILE (const string& name, const SIMODE mode, const
		  ECA_AUDIO_FORMAT& fmt) throw(ECA_ERROR*) 
  :  AUDIO_IO(name, mode, fmt), ewfname(name) {
  // ---
  // Prepare data file (.wav) for processing.
  // ---
  string::const_iterator e = find(name.begin(), name.end(), '.');

  if (e == name.end()) {
    throw(new ECA_ERROR("AUDIOIO-EWF", "Invalid file name; unable to open file.",ECA_ERROR::retry));
    return;
  }
  wavename = string(name.begin(), e);
  wavename = wavename + ".wav";

  ecadebug->msg(1, "AUDIOIO-EWF: Opening EWF data file" + wavename);
  wobject = new WAVEFILE(wavename, mode, fmt);
    
  if (mode == si_read || mode == si_readwrite) {
    // --
    // Read info from .ewf file.
    read_ewf_parameters();
  }
  else
    sample_offset = 0;
        
  // ---
  // Initialize attributes.
  // ---
  wave_object_active = false;
}

EWFFILE::~EWFFILE(void) {
  if (is_open()) close();
  write_ewf_parameters();
  delete wobject;
}

void EWFFILE::open(void) {
  wobject->open();
  seek_position();
  toggle_open_state(true);
}

void EWFFILE::close(void) {
  wobject->close();
  toggle_open_state(false);
}

void EWFFILE::read_buffer(SAMPLE_BUFFER* sbuf) {
  if (wave_object_active == false && position_in_samples() >= sample_offset) {
    ecadebug->msg(2, "AUDIOIO-EWF: wave_object activated" + ewfname);
    wave_object_active = true;
    wobject->seek_position_in_samples(position_in_samples()
				      - sample_offset);
  }

  if (wave_object_active == true) {
    wobject->read_buffer(sbuf);
  }
  else {
    sbuf->make_silent();
  }

  position_in_samples_advance(sbuf->length_in_samples());
}

void EWFFILE::write_buffer(SAMPLE_BUFFER* sbuf) {
  if (wave_object_active == false) {
    wave_object_active = true;
    sample_offset = position_in_samples();
    MESSAGE_ITEM m;
    m << "AUDIOIO-EWF: found sample_offset " << sample_offset << ".";
    ecadebug->msg(5, m.to_string());
  }
  
  wobject->write_buffer(sbuf);
  position_in_samples_advance(sbuf->length_in_samples());
  extend_position();
}

void EWFFILE::seek_position(void) {
  if (is_open()) {
    if (position_in_samples() >= sample_offset) {
      wobject->seek_position_in_samples(position_in_samples() - sample_offset);
    }
    else {
      wave_object_active = false;
      wobject->seek_first();
    }
  }
}

void EWFFILE::read_ewf_parameters(void) {
  ifstream fin(ewfname.c_str());
  char c;
  while (fin.get(c)) {
    if (c == '=') break;
    ecadebug->msg(5, "AUDIOIO-EWF: r_ewf_params(), found a = sign");
  }
  fin >> sample_offset;
  //    ecadebug->msg(5, "AUDIOIO-EWF: r_ewf_params(), read a sample_offset of", sample_offset);
}

void EWFFILE::write_ewf_parameters(void) {
  ofstream fout(ewfname.c_str());
  fout << "sample_offset = ";
  fout << sample_offset;
  fout.close();
}
