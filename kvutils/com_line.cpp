// ------------------------------------------------------------------------
// com_line.cpp: A wrapper class for parsing command line arguments.
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

#include <string>
#include <vector>

// #include "debug.h"
#include "com_line.h"

COMMAND_LINE::COMMAND_LINE(int argc, char *argv[]) {
  current = 0;

  for(int t = 0; t < argc; t++) {
    cparams.push_back(argv[t]);
  }
}

COMMAND_LINE::COMMAND_LINE(const vector<string>& params) {
  cparams = params;
}

string COMMAND_LINE::next_argument(void) {
  while (current < cparams.size()) {
    ++current;
    if (cparams[current - 1].size() > 0) {
      if (cparams[current - 1].at(0) == '-') return(cparams[current -
							   1]);
    }
  }
  return("");
}

void COMMAND_LINE::push_back(const string& argu) {
  cparams.push_back(argu);
}

string COMMAND_LINE::next_non_argument(void) {
  while (current < cparams.size()) {
    ++current;
    if (cparams[current - 1].size() > 0) {
      if (cparams[current - 1].at(0) != '-') return(cparams[current - 1]);
    }
  }
  return("");
}

string COMMAND_LINE::next(void) {
  if (ready() == false) return("");
  ++current;    
  return(cparams[current - 1]);
}

string COMMAND_LINE::previous(void) {
  --current;
  if (ready() == false) return("");
  if (current > 0) return(cparams[current - 1]);
  else return("");
}

bool COMMAND_LINE::has(char option) {
  vector<string>::size_type savepos = current;

  current = 0;
  while (current < cparams.size()) {
    ++current;
    if (cparams[current - 1].size() > 0) {
      if (cparams[current - 1].at(0) == '-' &&
	  cparams[current - 1].at(1) == option) {
	current = savepos;
	return(true);
      }
    }
  }
  current = savepos;
  return(false);
}

bool COMMAND_LINE::has(const string& option) {
  vector<string>::size_type savepos = current;

  current = 0;
  while (current < cparams.size()) {
    ++current;
    if (cparams[current - 1] == option) {
      current = savepos;
      return(true);
    }
  }
  current = savepos;
  return(false);
}




