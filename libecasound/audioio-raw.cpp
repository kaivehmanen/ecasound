// ------------------------------------------------------------------------
// audioio-raw.cpp: Raw/headerless audio file format input/output
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
#include <string>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <sys/stat.h>
#include <unistd.h>

#include "samplebuffer.h"
#include "audioio-types.h"
#include "audioio-raw.h"

#include "eca-error.h"
#include "eca-debug.h"

RAWFILE::RAWFILE(const string& name, bool double_buffering) {
  label(name);
  double_buffering_rep = double_buffering;
  fio_repp = 0;
}

RAWFILE::~RAWFILE(void) { close(); }

void RAWFILE::format_query(void) {
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

void RAWFILE::open(void) { 
  switch(io_mode()) {
  case io_read:
    {
      if (label() == "stdin" || label().at(0) == '-') {
	fio_repp = new ECA_FILE_IO_STREAM();
	fio_repp->open_stdin();
      }
      else {
	if (double_buffering_rep) fio_repp = new ECA_FILE_IO_MMAP();
	else fio_repp = new ECA_FILE_IO_STREAM();
	fio_repp->open_file(label(),"rb");
      }
      break;
    }
  case io_write: 
    {
      fio_repp = new ECA_FILE_IO_STREAM();
      if (label() == "stdout" || label().at(0) == '-') {
	cerr << "(audioio-raw) Outputting to standard output [r].\n";
	fio_repp->open_stdout();
      }
      else {
	fio_repp->open_file(label(),"wb");
      }
      break;
    }
  case io_readwrite: 
    {
      fio_repp = new ECA_FILE_IO_STREAM();
      if (label() == "stdout" || label().at(0) == '-') {
	cerr << "(audioio-raw) Outputting to standard output [rw].\n";
	fio_repp->open_stdout();
      }
      else {
	fio_repp->open_file(label(),"r+b", false);
	if (fio_repp->file_mode() == "") {
	  fio_repp->open_file(label(),"w+b", true);
	}
      }
    }
  }
  set_length_in_bytes();
  toggle_open_state(true);
  seek_position();
}

void RAWFILE::close(void) {
  if (fio_repp != 0) {
    fio_repp->close_file();
    delete fio_repp;
  }
  toggle_open_state(false);
}

bool RAWFILE::finished(void) const {
 if (fio_repp->is_file_error() ||
     !fio_repp->is_file_ready()) 
   return true;

 return false;
}

long int RAWFILE::read_samples(void* target_buffer, long int samples) {
  fio_repp->read_to_buffer(target_buffer, frame_size() * samples);
  return(fio_repp->file_bytes_processed() / frame_size());
}

void RAWFILE::write_samples(void* target_buffer, long int samples) {
  fio_repp->write_from_buffer(target_buffer, frame_size() * samples);  
}

void RAWFILE::seek_position(void) {
  if (is_open()) {
    fio_repp->set_file_position(position_in_samples() * frame_size());
  }
}

void RAWFILE::set_length_in_bytes(void) {
  long int savetemp = fio_repp->get_file_position();

  fio_repp->set_file_position_end();
  length_in_samples(fio_repp->get_file_position() / frame_size());

  fio_repp->set_file_position(savetemp);
}

