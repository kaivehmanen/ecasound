// ------------------------------------------------------------------------
// eca-fileio-stream.cpp: File-I/O and buffering routines using normal
//                        file streams.
// Copyright (C) 1999,2001 Kai Vehmanen (kai-vehmanen@wakkanet.fi)
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

#include <cstdio>
#include <cstring>
#include <climits> /* LONG_MAX */
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h> /* stat() */
#include <unistd.h> /* stat() */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* off_t */
#endif

#include <kvutils/dbc.h>

#include "eca-debug.h"
#include "eca-fileio.h"
#include "eca-fileio-stream.h"

void ECA_FILE_IO_STREAM::open_file(const std::string& fname, 
				   const std::string& fmode)
{ 
  fname_rep = fname;
  f1 = std::fopen(fname_rep.c_str(), fmode.c_str());
  if (!f1) {
    mode_rep = "";
  }
  else {
    mode_rep = fmode;
  }
  standard_mode = false;
  curpos_rep = 0;
}

void ECA_FILE_IO_STREAM::open_stdin(void) { 
  f1 = stdin;
  mode_rep = "rb";
  standard_mode = true;
  curpos_rep = 0;
}

void ECA_FILE_IO_STREAM::open_stdout(void) { 
  f1 = stdout;
  mode_rep = "wb";
  standard_mode = true;
  curpos_rep = 0;
}

void ECA_FILE_IO_STREAM::open_stderr(void) { 
  f1 = stderr;
  mode_rep = "wb";
  standard_mode = true;
  curpos_rep = 0;
}

void ECA_FILE_IO_STREAM::close_file(void) { 
  if (standard_mode != true) std::fclose(f1);
  mode_rep = "";
}

void ECA_FILE_IO_STREAM::read_to_buffer(void* obuf, off_t bytes) { 
  if (is_file_ready() == true) {
    last_rep = std::fread(obuf, 1, bytes, f1);
    curpos_rep += last_rep;
  }
  else {
    last_rep = 0;
  }
}

void ECA_FILE_IO_STREAM::write_from_buffer(void* obuf, off_t bytes) { 
  if (is_file_ready() == true) {
    last_rep = std::fwrite(obuf, 1, bytes, f1);
    curpos_rep += last_rep;
  }
  else {
    last_rep = 0;
  }
}

off_t ECA_FILE_IO_STREAM::file_bytes_processed(void) const { return(last_rep); }

bool ECA_FILE_IO_STREAM::is_file_ready(void) const { 
  if (mode_rep == "" ||
      std::feof(f1) ||
      std::ferror(f1)) return(false);
  return(true);
}

bool ECA_FILE_IO_STREAM::is_file_error(void) const { 
  if (std::ferror(f1)) return(true);
  return(false);
}

void ECA_FILE_IO_STREAM::set_file_position(off_t newpos) { 
  curpos_rep = newpos;
  if (standard_mode != true) {
/* fseeko doesn't seem to work with glibc 2.1.x */
#if _LARGEFILE_SOURCE
    DBC_CHECK(curpos_rep < 9223372036854775807LL); /* 2^63-1 */
    if (curpos_rep > LONG_MAX) {
      std::cerr << "(eca-fileio-stream) seeking from 0 to " << LONG_MAX << std::endl;
      int res = std::fseek(f1, LONG_MAX, SEEK_SET);
      if (res == 0) {
	std::cerr << "(eca-fileio-stream) fw-seeking from " << LONG_MAX << " to " << curpos_rep << std::endl; 
	int res2 = std::fseek(f1, static_cast<long int>(curpos_rep - LONG_MAX), SEEK_CUR);
	if (res2 != 0) {
	  ecadebug->msg(ECA_DEBUG::info, "(eca-fileio-stream) fseek() error2! (lfs).");
	}
      }
      else {
	ecadebug->msg(ECA_DEBUG::info, "(eca-fileio-stream) fseek() error1! (lfs).");
      }
    }
    else {
      std::cerr << "(eca-fileio-stream) seeking from 0 to " << curpos_rep << std::endl; 
      int res = std::fseek(f1, static_cast<long int>(curpos_rep), SEEK_SET);
      if (res == -1) {
	ecadebug->msg(ECA_DEBUG::info, "(eca-fileio-stream) fseek() error3! (lfs).");
      }
    }
#else
    DBC_CHECK(sizeof(long int) == sizeof(off_t));
    std::fseek(f1, curpos_rep, SEEK_SET);
#endif

  }
}

void ECA_FILE_IO_STREAM::set_file_position_advance(off_t fw) { 
  if (standard_mode != true) {
    set_file_position(curpos_rep + fw);
  }
}

void ECA_FILE_IO_STREAM::set_file_position_end(void) { 
  if (standard_mode == false) {
    std::fseek(f1, 0, SEEK_END);
  }
}

off_t ECA_FILE_IO_STREAM::get_file_position(void) const { 
  if (standard_mode == true) return(0);
  return(curpos_rep);
}

off_t ECA_FILE_IO_STREAM::get_file_length(void) const {
  if (standard_mode == true) return(0);
  
  struct stat temp;
  stat(fname_rep.c_str(), &temp);
  off_t lentemp = temp.st_size;

  return(lentemp); 
}
