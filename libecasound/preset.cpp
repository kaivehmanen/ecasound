// ------------------------------------------------------------------------
// preset.cpp: Class for representing effect presets
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <vector>
#include <string>

#include <kvutils.h>

#include "preset.h"

#include "eca-chain.h"
#include "eca-chainop.h"
#include "generic-controller.h"
#include "samplebuffer.h"

#include "eca-debug.h"
#include "eca-error.h"

PRESET::PRESET(void) 
  : csetup ("untitled", false) {
  parsed_rep = false;
}

PRESET::PRESET(const string& formatted_string) 
  : csetup ("untitled", false),
    parse_string_rep(formatted_string) {
  parse(formatted_string);
}

PRESET::~PRESET(void) {
  vector<SAMPLE_BUFFER*>::iterator p = buffers.begin();
  while(p != buffers.end()) {
    if (p != buffers.begin()) delete *p; // first buffer points to an
                                         // outside buffer -> not
                                         // deleted here
    ++p;
  }

  vector<CHAIN*>::iterator q = chains.begin();
  while(q != chains.end()) {
    delete *q;
    ++q;
  }
}

void PRESET::parse(const string& formatted_string) {
  // --------
  REQUIRE(formatted_string.empty() == false);
  // --------

  chains.clear();
  chains.push_back(new CHAIN());

  CHAIN_OPERATOR *cop;
  GENERIC_CONTROLLER* gctrl;

  vector<string> tokens = string_to_words(formatted_string);
  vector<string>::const_iterator p = tokens.begin();
  while(p != tokens.end()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "Parsing: " + *p + ".");
    if (*p == "|") {
      add_chain();
      ++p;
      continue;
    }
    cop = 0;
    cop = csetup.create_chain_operator(*p);
    if (cop != 0) {
      chains.back()->add_chain_operator(cop);
      chains.back()->selected_chain_operator_as_target();
    }
    else {
      if (get_argument_prefix(*p) == "kx") 
	chains.back()->selected_controller_as_target();
      else {
	gctrl = 0;
	gctrl = csetup.create_controller(*p);
	if (gctrl != 0) {
	  chains.back()->add_controller(gctrl);
	}
      }
    }
    ++p;
  }

  parsed_rep = true;

  // --------
  ENSURE(is_parsed() == true);
  // --------
}

void PRESET::add_chain(void) {
  chains.push_back(new CHAIN());
  buffers.push_back(new SAMPLE_BUFFER());
}

void PRESET::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  switch (param) {
  case 1: 

    break;
  }
}

CHAIN_OPERATOR::parameter_type PRESET::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(0.0);
  }
  return(0.0);
}

void PRESET::init(SAMPLE_BUFFER *insample) {  
  first_buffer = insample;
  chains[0]->init(first_buffer, first_buffer->number_of_channels(), first_buffer->number_of_channels());
  for(int q = 1; q < static_cast<int>(chains.size()); q++) {
    assert(q - 1 < static_cast<int>(buffers.size()));
    buffers[q - 1]->length_in_samples(first_buffer->length_in_samples());
    buffers[q - 1]->number_of_channels(first_buffer->number_of_channels());
    buffers[q - 1]->sample_rate(first_buffer->sample_rate());
    chains[q]->init(buffers[q - 1], first_buffer->number_of_channels(), first_buffer->number_of_channels());
  }
}

void PRESET::process(void) {
  vector<SAMPLE_BUFFER*>::iterator p = buffers.begin();
  while(p != buffers.end()) {
    (*p)->copy(*first_buffer);
    ++p;
  }

  vector<CHAIN*>::iterator q = chains.begin();
  while(q != chains.end()) {
    (*q)->process();
    ++q;
  }

  if (chains.size() > 1) {
    first_buffer->divide_by(chains.size());
    p = buffers.begin();
    while(p != buffers.end()) {
      first_buffer->add_with_weight(**p, static_cast<int>(chains.size()));
      ++p;
    }
  }
}
