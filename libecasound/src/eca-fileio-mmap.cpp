// ------------------------------------------------------------------------
// eca-fileio-mmap.cpp: mmap based file-I/O and buffering routines.
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

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "eca-error.h"
#include "eca-fileio.h"
#include "eca-fileio-mmap.h"
#include "eca-fileio-mmap-fthread.h"

ECA_FILE_IO_MMAP::~ECA_FILE_IO_MMAP(void) { }

void ECA_FILE_IO_MMAP::open_file(const string& fname, 
			    const string& fmode,
			    bool handle_errors) throw(ECA_ERROR*)
{ 
  if (fmode == "rb") {
    f1 = open(fname.c_str(), O_RDWR);
    if (!f1) {
      if (handle_errors) {
	throw(new ECA_ERROR("ECA-FILEIO", "unable to open file " + fname +
			    " in mode \"" + fmode + "\".", ECA_ERROR::retry));
      }
      mode_rep = "";
    }
    else {
      file_ready = true;
      file_ended = false;
      mode_rep = fmode;
      fposition = 0;
      flength = get_file_length();
      fmaxbsize = ecasound_fiommap_maximum_buffersize();
      ecasound_fiommap_register_fd(f1, flength);
    }
  }
  else {
    if (handle_errors) {
      throw(new ECA_ERROR("ECA-FILEIO", "unable to open file " + fname +
			    " in mode \"" + fmode + "\".", ECA_ERROR::retry));
    }
  }
}

void ECA_FILE_IO_MMAP::close_file(void) { 
  ecasound_fiommap_close_fd(f1);
  close(f1);
}

void ECA_FILE_IO_MMAP::read_to_buffer(void* obuf, long int bytes) {
  if (is_file_ready() == false) {
    bytes_rep = 0;
    file_ended = true;
    return;
  }

  internal_buffer = ecasound_fiommap_active_buffer(f1);
  if (internal_buffer == MAP_FAILED) {
    bytes_rep = 0;
    fposition = flength;
    perror(0);
  }
  else {
    internal_bsize = ecasound_fiommap_active_buffersize(f1);
    assert(internal_bsize > 0);
    long int internal_bindex = fposition % fmaxbsize;
    assert(internal_bindex >= 0);
    assert(internal_bindex < internal_bsize);
    if (internal_bindex + bytes > internal_bsize) {
      bytes_rep = internal_bsize - internal_bindex;
      memcpy(obuf, internal_buffer + internal_bindex, bytes_rep);
      ecasound_fiommap_next_buffer(f1);
      internal_buffer = ecasound_fiommap_active_buffer(f1);
      if (internal_buffer == MAP_FAILED) {
	fposition = flength;
	perror(0);
      }
      else {
	internal_bsize = ecasound_fiommap_active_buffersize(f1);
	assert(internal_bsize >= 0);
	bytes_rep = bytes - bytes_rep;
	if (bytes_rep > internal_bsize) {
	  bytes_rep -= internal_bsize;
	  bytes -= bytes_rep;
	  bytes_rep = internal_bsize;
	}
	memcpy(obuf, internal_buffer, bytes_rep);
      }
    }
    else {
      if (internal_bindex + bytes > internal_bsize ||
	  internal_bindex + bytes == fmaxbsize) ecasound_fiommap_next_buffer(f1);
      memcpy(obuf, internal_buffer + internal_bindex, bytes);
    }
    set_file_position(fposition + bytes, false);
    bytes_rep = bytes;
  }
}

void ECA_FILE_IO_MMAP::write_from_buffer(void* obuf, long int
				   bytes) { }

long int ECA_FILE_IO_MMAP::file_bytes_processed(void) const { return(bytes_rep); }
bool ECA_FILE_IO_MMAP::is_file_ready(void) const { return(file_ready); }
bool ECA_FILE_IO_MMAP::is_file_ended(void) const { return(file_ended); }
bool ECA_FILE_IO_MMAP::is_file_error(void) const { return(!file_ready && !file_ended); }

void ECA_FILE_IO_MMAP::set_file_position(long int newpos, bool seek) { 
  fposition = newpos;
  if (fposition >= flength) {
    fposition = flength;
    file_ready = false;
    file_ended = true;
  }
  else {
    file_ready = true;
    file_ended = false;
    if (seek == true) ecasound_fiommap_reset(f1, fposition);
  }
}

void ECA_FILE_IO_MMAP::set_file_position_advance(long int fw) { 
  set_file_position(fposition + fw, false);
}

void ECA_FILE_IO_MMAP::set_file_position_end(void) { 
  fposition = get_file_length();
}

long int ECA_FILE_IO_MMAP::get_file_position(void) const { return(fposition); }

long int ECA_FILE_IO_MMAP::get_file_length(void) const {
  struct stat stattemp;
  fstat(f1, &stattemp);
  return((long int)stattemp.st_size);
}


