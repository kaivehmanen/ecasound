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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cmath>
#include <cstdio>
#include <string>
#include <cstring>
#include <cassert>
#include <climits> /* LONG_MAX */
#include <sys/stat.h> /* stat() */
#include <unistd.h> /* stat() */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* off_t */
#else
typedef long int off_t;
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

long int CDRFILE::read_samples(void* target_buffer, long int samples)
{
  return(std::fread(target_buffer, frame_size(), samples, fobject));
}

void CDRFILE::write_samples(void* target_buffer, long int samples) {
  std::fwrite(target_buffer, frame_size(), samples, fobject);
}

void CDRFILE::seek_position(void) {
  DBC_CHECK(curpos_rep >= 0);
  if (is_open()) {
    off_t curpos_rep = position_in_samples() * frame_size();
/* fseeko doesn't seem to work with glibc 2.1.x */
#if _LARGEFILE_SOURCE
    DBC_CHECK(curpos_rep < 9223372036854775807LL); /* 2^63-1 */
    if (curpos_rep > LONG_MAX) {
      // std::cerr << "(audioio-cdr) seeking from 0 to " << LONG_MAX << std::endl;
      int res = std::fseek(fobject, LONG_MAX, SEEK_SET);
      if (res == 0) {
	// std::cerr << "(audioio-cdr) fw-seeking from " << LONG_MAX << " to " << curpos_rep << std::endl; 
	int res2 = std::fseek(fobject, static_cast<long int>(curpos_rep - LONG_MAX), SEEK_CUR);
	if (res2 != 0) {
	  ecadebug->msg(ECA_DEBUG::info, "(audioio-cdr) fseek() error2! (lfs).");
	}
      }
      else {
	ecadebug->msg(ECA_DEBUG::info, "(audioio-cdr) fseek() error1! (lfs).");
      }
    }
    else {
      // std::cerr << "(audioio-cdr) seeking from 0 to " << curpos_rep << std::endl; 
      int res = std::fseek(fobject, static_cast<long int>(curpos_rep), SEEK_SET);
      if (res == -1) {
	ecadebug->msg(ECA_DEBUG::info, "(audioio-cdr) fseek() error3! (lfs).");
      }
    }
#else
    DBC_CHECK(sizeof(long int) == sizeof(off_t));
    std::fseek(fobject, curpos_rep, SEEK_SET);
#endif
  }
}

void CDRFILE::pad_to_sectorsize(void) {
  int padsamps = CDRFILE::sectorsize - ((length_in_samples() *
					frame_size()) % CDRFILE::sectorsize);

  if (padsamps == CDRFILE::sectorsize) {
    return;
  }
  for(int n = 0; n < padsamps; n++) ::fputc(0,fobject);

  DBC_DECLARE(long int endpos);
  DBC_DECLARE(endpos = std::ftell(fobject));
  DBC_CHECK((endpos %  CDRFILE::sectorsize) == 0);
}

void CDRFILE::set_length_in_bytes(void) {
  struct stat temp;
  stat(label().c_str(), &temp);
  length_in_samples(temp.st_size / frame_size());
}
