// ------------------------------------------------------------------------
// kvutils.cpp: Misc helper routines
// Copyright (C) 1999-2001 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <sys/time.h> /* gettimeofday() */
#include <unistd.h>
#include <ctype.h> /* isspace(), toupper() */

#include <cstdlib> /* atoi() */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "kvutils.h"

vector<string> string_to_words(const string& s) {
  vector<string> vec;
  string stmp = "";

  for(string::const_iterator p = s.begin(); p != s.end(); p++) {
    if (isspace(*p) == false)
      stmp += *p;
    else {
      if (stmp == "") continue;
      vec.push_back(stmp);
      //      cout << "EDebug - added word: " << stmp << ".\n";
      stmp = "";
    }
  }
  if (stmp.size() > 0)
    vec.push_back(stmp);

  return(vec);
}

vector<string> string_to_vector(const string& str, const
				string::value_type separator) {
  vector<string> vec;
  string stmp = "";

  for(string::const_iterator p = str.begin(); p != str.end(); p++) {
    if (*p != separator)
      stmp += *p;
    else {
      if (stmp == "") continue;
      vec.push_back(stmp);
      stmp = "";
    }
  }
  if (stmp.size() > 0)
    vec.push_back(stmp);

  return(vec);
}

vector<int> string_to_int_vector(const string& str, 
				 const string::value_type separator) {
  vector<int> vec;
  string stmp = "";

  for(string::const_iterator p = str.begin(); p != str.end(); p++) {
    if (*p != separator)
      stmp += *p;
    else {
      if (stmp == "") continue;
      vec.push_back(atoi(stmp.c_str()));
      stmp = "";
    }
  }
  if (stmp.size() > 0)
    vec.push_back(atoi(stmp.c_str()));

  return(vec);
}

string vector_to_string(const vector<string>& str, 
			const string& separator) {

  string stmp;

  vector<string>::const_iterator p = str.begin();
  while(p != str.end()) {
    stmp += *p;
    ++p;
    if (p != str.end()) 
      stmp += separator;
  }

  return(stmp);
}

string string_search_and_replace(const string& str, 
				 const string::value_type from,
				 const string::value_type to) {
  string stmp (str);
  for(vector<string>::size_type p = 0; p < str.size(); p++) {
    if (str[p] == from) stmp[p] = to;
    else stmp[p] = str[p];
  }

  return(stmp);
}


bool string_icmp(const string& first, const string& second) {

  string a = first;
  string b = second;

  a = remove_trailing_spaces(a);
  a = remove_preceding_spaces(a);
  a = convert_to_uppercase(a);

  b = remove_trailing_spaces(b);
  b = remove_preceding_spaces(b);
  b = convert_to_uppercase(b);

  return(a == b);
}

string remove_trailing_spaces(const string& a) { 
  string r = "";
  string::const_reverse_iterator p;
  for(p = a.rbegin(); p != a.rend(); p++) {
    if (*p != ' ') break;
  }
  for(; p != a.rend(); p++) {
    r = *p + r;
  }
  return(r);
}

string remove_preceding_spaces(const string& a) { 
  string r = "";
  string::const_iterator p;
  for(p = a.begin(); p != a.end(); p++) {
    if (*p != ' ') break;
  }
  for(; p != a.end(); p++) {
    r += *p;
  }
  return(r);
}

string remove_surrounding_spaces(const string& a) { 
  string::const_iterator p,q;
  for(p = a.begin(); p != a.end() && *p == ' '; p++);
  for(q = (a.end() - 1); q != a.begin() && *q == ' '; q--);
  return(string(p,q + 1));
}

string convert_to_uppercase(const string& a) { 
  string r = a;
  for(string::iterator p = r.begin(); p != r.end(); p++)
    *p = toupper(*p);
  return(r);
}

string convert_to_lowercase(const string& a) { 
  string r = a;
  for(string::iterator p = r.begin(); p != r.end(); p++)
    *p = tolower(*p);
  return(r);
}

void to_uppercase(string& a) { 
  string::iterator p = a.begin();
  while(p != a.end()) {
    *p = toupper(*p);
    ++p;
  }
}

void to_lowercase(string& a) { 
  string::iterator p = a.begin();
  while(p != a.end()) {
    *p = tolower(*p);
    ++p;
  }
}

string get_argument_number(int number, const string& argu) {
    int curnro = 1;
    string::const_iterator e;
    string::const_iterator b = find(argu.begin(), argu.end(), ':');

    string target;

    if (b == argu.end()) {
      if (argu.size() > 0) b = argu.begin();
      else {
	return("");
      }
    }
    else 
      ++b;

    do {
        e = find(b, argu.end(), ',');

        if (number == curnro) {
            target = string(b, e);
            break;
        }
        curnro++;

        b = e;
        ++b;
    } while( b < argu.end());

    return(target);
}

int get_number_of_arguments(const string& argu) {
  int result = 0;

  string::const_iterator b,e; 
  b = find(argu.begin(), argu.end(), ':');

  if (b == argu.end()) {
    if (argu.size() > 0) b = argu.begin();
    else return(0);
  }
  else 
    ++b;

  for(; b != argu.end(); b++) {
    e = find(b, argu.end(), ',');
    if (b != e) {
      ++result;
    }
    if (e == argu.end()) break;
    b = e;
  }
  return(result);
}

vector<string> get_arguments(const string& argu) {
  vector<string> rvalue;

  string::const_iterator b = find(argu.begin(), argu.end(), ':');
  string::const_iterator e;

  if (b == argu.end()) {
    if (argu.size() > 0) b = argu.begin();
    else return(rvalue);
  }
  else 
    ++b;


  for(; b != argu.end(); b++) {
    e = find(b, argu.end(), ',');
    string target = string(b, e);
    if (target.size() > 0) {
      rvalue.push_back(target);
    }
    if (e == argu.end()) break;
    b = e;
  }
  return(rvalue);
}

string get_argument_prefix(const string& argu) {
  // --------
  // require:
  assert(argu.find('-') != string::npos);
  // --------

  string::const_iterator b = find(argu.begin(), argu.end(), '-');
  string::const_iterator e = find(argu.begin(), argu.end(), ':');

  if (b != argu.end()) {
    ++b;
    if (b !=  argu.end()) {
      return(string(b,e));
    }
  }

  return("");

  // --------
  // ensure:
  assert(argu.size() >= 0);
  // --------
}

void print_time_stamp(void) {
  // --
  // not thread-safe!
  // --
  static bool first = true;
  static struct timeval last;
  struct timeval current;

  if (first) {
    ::gettimeofday(&last, 0);
    first = false;
  }

  ::gettimeofday(&current, 0);

  cerr << "(timestamp) " << current.tv_sec << "sec, " <<
    current.tv_usec << "msec.";
  
  long delta = current.tv_usec;
  delta -= last.tv_usec;
  delta += (current.tv_sec - last.tv_sec) * 1000000;
  cerr << " Delta " << delta << "msec." << endl;

  last.tv_sec = current.tv_sec;
  last.tv_usec = current.tv_usec;
}

