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

RESOURCE_FILE::RESOURCE_FILE(const string& resource_file) :
  resfile_rep(resource_file) { 
  load();
}

RESOURCE_FILE::~RESOURCE_FILE(void) { 
// FIXME: dumps core for some reason
//    if (modified_rep == true) save();
}

void RESOURCE_FILE::load(void) { 
  lines_rep.resize(0);
  ifstream fin (resfile_rep.c_str());
  if (fin) {
    string line;
    string first, second;

    while(getline(fin,line)) {
      if (line.size() > 0 && line[0] == '#') {
	lines_rep.push_back(line);
	continue;
      }

      string::size_type n = line.find_first_of("=");
      if (n == string::npos) n = line.find_first_of(" ");
      if (n == string::npos) {
	continue;
      }
      
      first = string(line, 0, n);
      second = string(line, n + 1, string::npos);

      first = remove_surrounding_spaces(first);
      second = remove_surrounding_spaces(second);
      string::iterator p = second.end();
      --p;
      while (*p == '\\') {
	second.erase(p);
	lines_rep.push_back(line);
	if (getline(fin, line)) {
	  line = remove_surrounding_spaces(line);
	  second += line;
	  p = second.end();
	  --p;
	}
      }
//        cerr << "(resource-file) found key-value pair: " +
//  	first + " = \"" + second + "\"." << endl;
      resmap_rep[first] = second;
      lines_rep.push_back(line);
    }
  }
  fin.close();
  modified_rep = false;
}

void RESOURCE_FILE::save(void) { 
  ofstream fout (resfile_rep.c_str(), ios::out | ios::trunc);
  if (fout) {
    vector<string>::const_iterator p = lines_rep.begin();
    while(p != lines_rep.end()) {
      if (p->size() > 0) {
//  	cerr << "Writing line: " << *p << "." << endl;
	fout << *p << "\n";
      }
      ++p;
    }
  }
  fout.close();
  modified_rep = false;
}

vector<string> RESOURCE_FILE::keywords(void) const {
  vector<string> keys;
  map<string,string>::const_iterator p;
  p = resmap_rep.begin();
  while(p != resmap_rep.end()) {
//      cerr << "Adding keyword: " << p->first << "." << endl;
    keys.push_back(p->first);
    ++p;
  }
  return(keys);
}

bool RESOURCE_FILE::boolean_resource(const string& tag) const {
  if (resource(tag) == "true") return(true);
  return(false);
}

bool RESOURCE_FILE::has(const string& tag) const {
  if (resmap_rep.find(tag) == resmap_rep.end())
    return(false);
  return(true);
}

string RESOURCE_FILE::resource(const string& tag) const {
  if (has(tag) != true)
    return("");
//    cerr << "Returning resource: " << resmap_rep[tag] << "." << endl;
  return(resmap_rep[tag]);
}

void RESOURCE_FILE::resource(const string& tag, const string& value) {
  resmap_rep[tag] = value;
  
  bool found = false;
  vector<string>::iterator p;
  p = lines_rep.begin();
  while(p != lines_rep.end()) {
    string line = *p;
    if (line.size() > 0 && line[0] != '#') {
      string::size_type n = line.find_first_of("=");
      if (n == string::npos) n = line.find_first_of(" ");
      if (n != string::npos) {
	string first = remove_surrounding_spaces(string(line, 0, n));
	if (first == tag) {
	  *p = first + " = " + value;
	  found = true;
	}
      }
    }
    ++p;
  }

  if (found != true) {
    lines_rep.push_back(tag + " = " + value + "\n");
  }
  modified_rep = true;
}
