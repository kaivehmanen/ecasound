// ------------------------------------------------------------------------
// resource-file.cpp: Class for representing resource files.
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

#include <map>
#include <string>
#include <cstdlib>
#include <fstream>

#include <kvutils.h>

#include "resource-file.h"
#include "eca-debug.h"

RESOURCE_FILE::~RESOURCE_FILE(void) {
  if (changed_rep ||
      (loaded_resource_values != -1 && 
       loaded_resource_values != rcmap.size())) save();
}

void RESOURCE_FILE::load(void) {
  ifstream fin (res_file.c_str());

  if (!fin) return;

  string line;
  string first, second;
  int count = 0;

  while(getline(fin,line)) {
    if (line.size() > 0 && line[0] == '#') {
      rc_comments[count] += line;
//        cerr << "(resource-file) Added commment \""
//        	+ line + "\" to index number " 
//       	+ kvu_numtostr(count) << ".\n";
    }
    else { 
      string::size_type n = line.find_first_of("=");
      if (n != string::npos) {
	use_equal_sign = true;
      }
      else {
	n = line.find_first_of(" ");
	use_equal_sign = false;
      }

      if (n == string::npos) continue;
      
      first = string(line, 0, n);
      second = string(line, n + 1, string::npos);

//        cerr << "(resource-file) Second: " << second << ".\n";

      first = remove_trailing_spaces(first); 
      second = remove_preceding_spaces(second);
      rcmap[first] = second;
      ++count;
//        cerr << "(resource-file) Added resource-pair \"" 
//  	+ first + "\" = \"" + second + "\".\n";
    }
  }
  loaded_resource_values = rcmap.size();
  changed_rep = false;
  fin.close();
}
 
void RESOURCE_FILE::save(void) {
  ofstream fout (res_file.c_str());
  if (!fout) return;

  map<string,string>::const_iterator p = rcmap.begin();
  map<int,string>::const_iterator q;
  int count = 0;
  while(p != rcmap.end()) {
    q = rc_comments.begin();
    while (q != rc_comments.end()) {
      if (q->first == count) {
	fout << q->second << "\n";
	break;
      }
      ++q;
    }
    if (use_equal_sign)
      fout << p->first << " = " << p->second << "\n";
    else
      fout << p->first << " " << p->second << "\n";
    ++p;
    ++count;
  }

  q = rc_comments.begin();
  while (q != rc_comments.end()) {
    if (q->first == count) {
      fout << q->second << "\n";
      break;
    }
    ++q;
  }

  fout.close();
}

const string& RESOURCE_FILE::resource(const string& tag) {
  return(rcmap[tag]); 
}

void RESOURCE_FILE::resource(const string& tag, const string& value) {
  changed_rep = true;
  rcmap[tag] = value;
}

