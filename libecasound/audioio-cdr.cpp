// ------------------------------------------------------------------------
// audioio-cdr.cpp: CDDA/CDR audio file format input/output
// Copyright (C) 1999,2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

/* hack to make fseeko and fteelo work with glibc2.1.x */
#ifdef _LARGEFILE_SOURCE
  #if __GNUC__ == 2
    #if __GNUC_MINOR__ < 92
      #define _XOPEN_SOURCE 500
    #endif
  #endif
#else
#define fseeko std::fseek
#define ftello std::ftell
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cmath>
#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <sys/stat.h> /* stat() */
#include <unistd.h> /* stat() */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* off_t */
#endif

#include <kvutils/dbc.h>

#include "sample-specs.h"
#include "audioio-types.h"
#include "audioio-cdr.h"
#include "eca-debug.h"

CDRFILE::CDRFILE(const std::string& name) {
  label(name);
}

CDRFILE::~CDRFILE(void) {
  if (is_open() == true)
    close();
}

void CDRFILE::format_query(void) {
  // --------
  DBC_REQUIRE(!is_open());
  // --------

  struct stat temp;
  stat(label().c_str(), &temp);
  length_in_samples(temp.st_size / frame_size());

  // -------
  DBC_ENSURE(!is_open());
  // -------
}

void CDRFILE::open(void) throw(AUDIO_IO::SETUP_ERROR &) { 
  // --------
  DBC_REQUIRE(!is_open());
  // --------

  set_channels(2);
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_be);
  set_samples_per_second(44100);

  switch(io_mode()) {
  case io_read:
    {
      fobject = std::fopen(label().c_str(),"rb");
      if (!fobject)
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-CDR: Can't open " + label() + " for reading."));
      set_length_in_bytes();
      break;
    }
  case io_write: 
    {
      fobject = std::fopen(label().c_str(),"wb");
      if (!fobject) 
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-CDR: Can't open " + label() + " for writing."));
      break;
    }
  case io_readwrite:
    {
      fobject = std::fopen(label().c_str(),"r+b");
      if (!fobject) {
	fobject = std::fopen(label().c_str(),"w+b");
	if (!fobject)
	  throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-CDR: Can't open " + label() + " for read&write."));
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

  std::fclose(fobject);
  toggle_open_state(false);
}

bool CDRFILE::finished(void) const {
 if (std::ferror(fobject) ||
     std::feof(fobject))
   return true;

 return false;
}

long int CDRFILE::read_samples(void* target_buffer, long int samples) {
  return(std::fread(target_buffer, frame_size(), samples, fobject));
}

void CDRFILE::write_samples(void* target_buffer, long int samples) {
  std::fwrite(target_buffer, frame_size(), samples, fobject);
}

void CDRFILE::seek_position(void) {
  if (is_open()) {
    off_t newpos = position_in_samples() * frame_size();
    fseeko(fobject, newpos, SEEK_SET);
  }
}

void CDRFILE::pad_to_sectorsize(void) {
  int padsamps = CDRFILE::sectorsize - ((length_in_samples() *
					frame_size()) % CDRFILE::sectorsize);

  if (padsamps == CDRFILE::sectorsize) {
    return;
  }
  for(int n = 0; n < padsamps; n++) ::fputc(0,fobject);

  DBC_DECLARE(off_t endpos);
  DBC_DECLARE(endpos = ftello(fobject));
  DBC_CHECK((endpos %  CDRFILE::sectorsize) == 0);
}

void CDRFILE::set_length_in_bytes(void) {
  fpos_t save;
  std::fgetpos(fobject, &save);
  fseeko(fobject,0,SEEK_END);
  off_t endpos;
  endpos = ftello(fobject);
  length_in_samples(endpos / frame_size());
  std::fsetpos(fobject, &save);
}
