// ------------------------------------------------------------------------
// eca-fileio-stream.cpp: File-I/O and buffering routines using normal
//                        file streams.
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
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#include "eca-fileio.h"
#include "eca-fileio-stream.h"

void ECA_FILE_IO_STREAM::open_file(const string& fname, 
				   const string& fmode)
{ 
  f1 = ::fopen(fname.c_str(), fmode.c_str());
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

void ECA_FILE_IO_STREAM::close_file(void) { 
  if (standard_mode == false) ::fclose(f1);
  mode_rep = "";
}

void ECA_FILE_IO_STREAM::read_to_buffer(void* obuf, long int bytes) { 
  bytes_rep = ::fread(obuf, 1, bytes, f1);
}

void ECA_FILE_IO_STREAM::write_from_buffer(void* obuf, long int bytes) { 
  bytes_rep = ::fwrite(obuf, 1, bytes, f1);
}

long int ECA_FILE_IO_STREAM::file_bytes_processed(void) const { return(bytes_rep); }

bool ECA_FILE_IO_STREAM::is_file_ready(void) const { 
  if (mode_rep == "" ||
      feof(f1) ||
      ferror(f1)) return(false);
  return(true);
}

bool ECA_FILE_IO_STREAM::is_file_error(void) const { 
  if (ferror(f1)) return(true);
  return(false);
}

void ECA_FILE_IO_STREAM::set_file_position(long int newpos) { 
  if (f1 != stdin && f1 != stdout)
    fseek(f1, newpos, SEEK_SET);
}

void ECA_FILE_IO_STREAM::set_file_position_advance(long int fw) { 
  if (standard_mode == false)
    ::fseek(f1, fw, SEEK_CUR);
}

void ECA_FILE_IO_STREAM::set_file_position_end(void) { 
  if (standard_mode == false) {
    ::fseek(f1, 0, SEEK_END);
  }
}

long int ECA_FILE_IO_STREAM::get_file_position(void) const { 
  if (standard_mode == false) return(::ftell(f1));
  return(0);
}

long int ECA_FILE_IO_STREAM::get_file_length(void) const {
  if (standard_mode == true) return(0);
  long int savetemp = ::ftell(f1);
  ::fseek(f1, 0, SEEK_END);
  long int lentemp = ftell(f1);
  ::fseek(f1, savetemp, SEEK_SET);
  return(lentemp); 
}
