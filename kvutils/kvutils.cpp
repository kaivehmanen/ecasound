// ------------------------------------------------------------------------
// kvutils.cpp: Misc helper routines
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <algorithm>
#include <cstdlib> /* atoi() */
#include <iostream>
#include <string>
#include <vector>

#include <sys/time.h> /* gettimeofday() */
#include <unistd.h>
#include <ctype.h> /* isspace(), toupper() */

#include "kvutils.h"

std::vector<std::string> string_to_words(const std::string& s)
{
  return(string_to_tokens(s));
}

std::vector<std::string> string_to_tokens(const std::string& s)
{
  std::vector<std::string> vec;
  std::string stmp = "";

  for(std::string::const_iterator p = s.begin(); p != s.end(); p++) {
    if (isspace(*p) == 0)
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

std::vector<std::string> string_to_tokens_quoted(const std::string& s)
{
  std::vector<std::string> vec;
  std::string stmp;
  bool quoteflag = false;

  for(std::string::const_iterator p = s.begin(); p != s.end(); p++) {
    if (*p == '\"') {
      quoteflag = !quoteflag;
    }
    else if (*p == '\\') {
      p++;
      stmp += *p;
    }
    else if (isspace(*p) == 0 || quoteflag == true) {
      stmp += *p;
    }
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

std::vector<std::string> string_to_vector(const std::string& str, 
					  const std::string::value_type separator)
{
  std::vector<std::string> vec;
  std::string stmp = "";

  for(std::string::const_iterator p = str.begin(); p != str.end(); p++) {
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

std::vector<int> string_to_int_vector(const std::string& str, 
				 const std::string::value_type separator) {
  std::vector<int> vec;
  std::string stmp = "";

  for(std::string::const_iterator p = str.begin(); p != str.end(); p++) {
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

std::string vector_to_string(const std::vector<std::string>& str, 
			const std::string& separator) {

  std::string stmp;

  std::vector<std::string>::const_iterator p = str.begin();
  while(p != str.end()) {
    stmp += *p;
    ++p;
    if (p != str.end()) 
      stmp += separator;
  }

  return(stmp);
}

std::string string_search_and_replace(const std::string& str, 
				 const std::string::value_type from,
				 const std::string::value_type to) {
  std::string stmp (str);
  for(std::vector<std::string>::size_type p = 0; p < str.size(); p++) {
    if (str[p] == from) stmp[p] = to;
    else stmp[p] = str[p];
  }

  return(stmp);
}


bool string_icmp(const std::string& first, const std::string& second) {

  std::string a = first;
  std::string b = second;

  a = remove_trailing_spaces(a);
  a = remove_preceding_spaces(a);
  a = convert_to_uppercase(a);

  b = remove_trailing_spaces(b);
  b = remove_preceding_spaces(b);
  b = convert_to_uppercase(b);

  return(a == b);
}

std::string remove_trailing_spaces(const std::string& a) { 
  std::string r = "";
  std::string::const_reverse_iterator p;
  for(p = a.rbegin(); p != a.rend(); p++) {
    if (*p != ' ') break;
  }
  for(; p != a.rend(); p++) {
    r = *p + r;
  }
  return(r);
}

std::string remove_preceding_spaces(const std::string& a) { 
  std::string r = "";
  std::string::const_iterator p;
  for(p = a.begin(); p != a.end(); p++) {
    if (*p != ' ') break;
  }
  for(; p != a.end(); p++) {
    r += *p;
  }
  return(r);
}

std::string remove_surrounding_spaces(const std::string& a) { 
  std::string::const_iterator p,q;
  for(p = a.begin(); p != a.end() && *p == ' '; p++);
  for(q = (a.end() - 1); q != a.begin() && *q == ' '; q--);
  return(std::string(p,q + 1));
}

std::string convert_to_uppercase(const std::string& a) { 
  std::string r = a;
  for(std::string::iterator p = r.begin(); p != r.end(); p++)
    *p = toupper(*p);
  return(r);
}

std::string convert_to_lowercase(const std::string& a) { 
  std::string r = a;
  for(std::string::iterator p = r.begin(); p != r.end(); p++)
    *p = tolower(*p);
  return(r);
}

void to_uppercase(std::string& a) { 
  std::string::iterator p = a.begin();
  while(p != a.end()) {
    *p = toupper(*p);
    ++p;
  }
}

void to_lowercase(std::string& a) { 
  std::string::iterator p = a.begin();
  while(p != a.end()) {
    *p = tolower(*p);
    ++p;
  }
}

std::string get_argument_number(int number, const std::string& argu) {
    int curnro = 1;
    std::string::const_iterator e;
    std::string::const_iterator b = std::find(argu.begin(), argu.end(), ':');

    std::string target;

    if (b == argu.end()) {
      if (argu.size() > 0) b = argu.begin();
      else {
	return("");
      }
    }
    else 
      ++b;

    do {
        e = std::find(b, argu.end(), ',');

        if (number == curnro) {
            target = std::string(b, e);
            break;
        }
        curnro++;

        b = e;
        ++b;
    } while( b < argu.end());

    return(target);
}

int get_number_of_arguments(const std::string& argu) {
  int result = 0;

  std::string::const_iterator b,e; 
  b = std::find(argu.begin(), argu.end(), ':');

  if (b == argu.end()) {
    if (argu.size() > 0) b = argu.begin();
    else return(0);
  }
  else 
    ++b;

  for(; b != argu.end(); b++) {
    e = std::find(b, argu.end(), ',');
    if (b != e) {
      ++result;
    }
    if (e == argu.end()) break;
    b = e;
  }
  return(result);
}

std::vector<std::string> get_arguments(const std::string& argu) {
  std::vector<std::string> rvalue;

  std::string::const_iterator b = std::find(argu.begin(), argu.end(), ':');
  std::string::const_iterator e;

  if (b == argu.end()) {
    if (argu.size() > 0) b = argu.begin();
    else return(rvalue);
  }
  else 
    ++b;


  for(; b != argu.end(); b++) {
    e = std::find(b, argu.end(), ',');
    std::string target = std::string(b, e);
    if (target.size() > 0) {
      rvalue.push_back(target);
    }
    if (e == argu.end()) break;
    b = e;
  }
  return(rvalue);
}

std::string get_argument_prefix(const std::string& argu) {
  // --------
  // require:
  assert(argu.find('-') != std::string::npos);
  // --------

  std::string::const_iterator b = std::find(argu.begin(), argu.end(), '-');
  std::string::const_iterator e = std::find(argu.begin(), argu.end(), ':');

  if (b != argu.end()) {
    ++b;
    if (b !=  argu.end()) {
      return(std::string(b,e));
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

  std::cerr << "(timestamp) " << current.tv_sec << "sec, " <<
    current.tv_usec << "msec.";
  
  long delta = current.tv_usec;
  delta -= last.tv_usec;
  delta += (current.tv_sec - last.tv_sec) * 1000000;
  std::cerr << " Delta " << delta << "msec." << std::endl;

  last.tv_sec = current.tv_sec;
  last.tv_usec = current.tv_usec;
}
