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

#include <cstdio>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> /* off_t */
#endif

#include "eca-fileio.h"
#include "eca-fileio-stream.h"

void ECA_FILE_IO_STREAM::open_file(const std::string& fname, 
				   const std::string& fmode)
{ 
  f1 = std::fopen(fname.c_str(), fmode.c_str());
  if (!f1) {
    mode_rep = "";
  }
  else {
    mode_rep = fmode;
  }
  standard_mode = false;
}

void ECA_FILE_IO_STREAM::open_stdin(void) { 
  f1 = stdin;
  mode_rep = "rb";
  standard_mode = true;
}

void ECA_FILE_IO_STREAM::open_stdout(void) { 
  f1 = stdout;
  mode_rep = "wb";
  standard_mode = true;
}

void ECA_FILE_IO_STREAM::open_stderr(void) { 
  f1 = stderr;
  mode_rep = "wb";
  standard_mode = true;
}

void ECA_FILE_IO_STREAM::close_file(void) { 
  if (standard_mode != true) std::fclose(f1);
  mode_rep = "";
}

void ECA_FILE_IO_STREAM::read_to_buffer(void* obuf, off_t bytes) { 
  if (is_file_ready() == true) {
    bytes_rep = std::fread(obuf, 1, bytes, f1);
  }
  else {
    bytes_rep = 0;
  }
}

void ECA_FILE_IO_STREAM::write_from_buffer(void* obuf, off_t bytes) { 
  if (is_file_ready() == true) {
    bytes_rep = std::fwrite(obuf, 1, bytes, f1);
  }
  else {
    bytes_rep = 0;
  }
}

off_t ECA_FILE_IO_STREAM::file_bytes_processed(void) const { return(bytes_rep); }

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
  if (standard_mode != true) {
    fseeko(f1, newpos, SEEK_SET);
  }
}

void ECA_FILE_IO_STREAM::set_file_position_advance(off_t fw) { 
  if (standard_mode != true) {
    fseeko(f1, fw, SEEK_CUR);
  }
}

void ECA_FILE_IO_STREAM::set_file_position_end(void) { 
  if (standard_mode == false) {
    fseeko(f1, 0, SEEK_END);
  }
}

off_t ECA_FILE_IO_STREAM::get_file_position(void) const { 
  if (standard_mode == true) return(0);
  return(ftello(f1));
}

off_t ECA_FILE_IO_STREAM::get_file_length(void) const {
  if (standard_mode == true) return(0);
  
  /* save old position */
  fpos_t savetemp;
  std::fgetpos(f1, &savetemp);

  /* seek to end and fetch file length */
  std::fseek(f1, 0, SEEK_END);
  off_t lentemp = ftello(f1);

  /* restore position */
  std::fsetpos(f1, &savetemp);

  return(lentemp); 
}
