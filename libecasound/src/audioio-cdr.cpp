// ------------------------------------------------------------------------
// audioio-cdr.cpp: CDDA/CDR audio file format input/output
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
#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <sys/stat.h>
#include <unistd.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-cdr.h"

#include "eca-error.h"
#include "eca-debug.h"

CDRFILE::CDRFILE(const string& name) {
  
  label(name);

  set_channels(2);
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_be);
  set_samples_per_second(44100);
}

CDRFILE::~CDRFILE(void) {
  close();
}

void CDRFILE::format_query(void) {
  // --------
  // require:
  assert(!is_open());
  // --------

  struct stat temp;
  stat(label().c_str(), &temp);
  length_in_samples(temp.st_size / frame_size());

  // -------
  // ensure:
  assert(!is_open());
  // -------
}

void CDRFILE::open(void) throw(ECA_ERROR*) { 
  // --------
  // require:
  assert(!is_open());
  // --------

  switch(io_mode()) {
  case io_read:
    {
      fobject=fopen(label().c_str(),"rb");
      if (!fobject)
	throw(new ECA_ERROR("AUDIOIO-CDR", "Can't open " + label() + " for reading."));
      set_length_in_bytes();
      break;
    }
  case io_write: 
    {
      fobject=fopen(label().c_str(),"wb");
      if (!fobject) 
	throw(new ECA_ERROR("AUDIOIO-CDR","Can't open " + label() + " for writing."));
      break;
    }
  case io_readwrite:
    {
      fobject=fopen(label().c_str(),"r+b");
      if (!fobject) {
	fobject=fopen(label().c_str(),"w+b");
	if (!fobject)
	  throw(new ECA_ERROR("AUDIOIO-CDR","Can't open " + label() + " for read-wre."));
      }
      set_length_in_bytes();
      break;
    }
  }
  toggle_open_state(true);
  seek_position();
}

void CDRFILE::close(void) { 
  if (io_mode() != io_read)
    pad_to_sectorsize();

  fclose(fobject);
  toggle_open_state(false);
}

bool CDRFILE::finished(void) const {
 if (ferror(fobject) ||
     feof(fobject))
   return true;

 return false;
}

long int CDRFILE::read_samples(void* target_buffer, long int samples) {
  return(fread(target_buffer, frame_size(), samples, fobject));
}

void CDRFILE::write_samples(void* target_buffer, long int samples) {
  fwrite(target_buffer, frame_size(), samples, fobject);
}

void CDRFILE::seek_position(void) {
  if (is_open())
    fseek(fobject, position_in_samples() * frame_size(), SEEK_SET);
}

void CDRFILE::pad_to_sectorsize(void) {
  int padsamps = CDRFILE::sectorsize - ((length_in_samples() *
					frame_size()) % CDRFILE::sectorsize);

  if (padsamps == CDRFILE::sectorsize) {
    return;
  }
  for(int n = 0; n < padsamps; n++) fputc(0,fobject);
  assert(ftell(fobject) %  CDRFILE::sectorsize == 0);
}

void CDRFILE::set_length_in_bytes(void) {
  long int save = ftell(fobject);
  fseek(fobject,0,SEEK_END);
  length_in_samples(ftell(fobject) / frame_size());
  fseek(fobject,save,SEEK_SET);
}
