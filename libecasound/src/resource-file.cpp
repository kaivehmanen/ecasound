// ------------------------------------------------------------------------
// resource-file.cpp: Generic resource file class
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

#include <string>
#include <cstdlib>
#include <fstream>

#include <kvutils.h>

#include "resource-file.h"
#include "eca-debug.h"

RESOURCE_FILE::~RESOURCE_FILE(void) { }

vector<string> RESOURCE_FILE::keywords(void) const {
  vector<string> keys;

  ifstream fin (res_file.c_str());
  if (!fin) return(keys);

  string line, first;
  while(getline(fin,line)) {
    if (line.size() > 0 && line[0] == '#') continue;

    string::size_type n = line.find_first_of("=");
    if (n == string::npos) n = line.find_first_of(" ");
    if (n == string::npos) continue;

    first = string(line, 0, n);
    first = remove_surrounding_spaces(first);
    
    keys.push_back(first);
  }
  return(keys);
}

bool RESOURCE_FILE::boolean_resource(const string& tag) const {
  if (resource(tag) == "true") return(true);
  return(false);
}

bool RESOURCE_FILE::has(const string& tag) const {
  if (resource(tag) == "") return(false);
  return(true);
}

string RESOURCE_FILE::resource(const string& tag) const {
  ifstream fin (res_file.c_str());
  if (!fin) return("");

  string line;
  string first, second;

  while(getline(fin,line)) {
    if (line.size() > 0 && line[0] == '#') continue;

    string::size_type n = line.find_first_of("=");
    if (n == string::npos) n = line.find_first_of(" ");
    if (n == string::npos) continue;
      
    first = string(line, 0, n);
    second = string(line, n + 1, string::npos);

    first = remove_surrounding_spaces(first);
    second = remove_surrounding_spaces(second);
    string::iterator p = second.end();
    --p;
    while (*p == '\\') {
      second.erase(p);
      if (getline(fin, line)) {
	line = remove_surrounding_spaces(line);
	second += line;
	p = second.end();
	--p;
      }
    }

    //    ecadebug->msg(ECA_DEBUG::system_objects, "(resource-file) found key-value pair: " + 
    //		  first + "-" + second + ".");
    if (first == tag) return(second);
  }
  return("");
}

void RESOURCE_FILE::resource(const string& tag, const string& value) {
  vector<string> lines;
  bool found = false;

  ifstream fin (res_file.c_str());
  if (fin) {
    string line, first, second;
    while(getline(fin,line)) {
      if (line.size() > 0 && line[0] != '#') {
	string::size_type n = line.find_first_of("=");
	if (n == string::npos) n = line.find_first_of(" ");
	if (n != string::npos) {
	  first = remove_surrounding_spaces(string(line, 0, n));
	  second = string(line, n + 1, string::npos);

	  if (first == tag) {
	    if (found == false) lines.push_back(first + " = " + value);
	    found = true;
	  }
	  else {
	    lines.push_back(line);
	  }
	}
      }
    }
    fin.close();
  }

  ofstream fout (res_file.c_str(), ios::out | ios::trunc);
  if (!fout) return;

  vector<string>::const_iterator p = lines.begin();
  while(p != lines.end()) {
    fout << *p << "\n";
    ++p;
  }
  if (found == false) {
    fout << tag << " = " << value << "\n";
  }
}


