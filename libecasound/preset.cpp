// ------------------------------------------------------------------------
// preset.cpp: Class for representing effect presets
// Copyright (C) 2000,2001 Kai Vehmanen (kaiv@wakkanet.fi)
// Copyright (C) 2001 Arto Hamara (artham@utu.fi)
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
#include <kvutils/dbc.h>

#include "eca-chain.h"
#include "eca-chainop.h"
#include "generic-controller.h"
#include "samplebuffer.h"
#include "eca-debug.h"
#include "eca-error.h"
#include "preset.h"

PRESET::PRESET(void) {
  parsed_rep = false;
}

PRESET::PRESET(const std::string& formatted_string) {
  parse(formatted_string);
}

PRESET* PRESET::clone(void) {
  std::vector<parameter_type> param_values;
  for(int n = 0; n < number_of_params(); n++) {
    param_values.push_back(get_parameter(n + 1));
  }
  PRESET* preset = new PRESET(parse_string_rep);
  for(int n = 0; n < preset->number_of_params(); n++) {
    preset->set_parameter(n + 1, param_values[n]);
  }
  return(preset);
}

PRESET::~PRESET(void) {
  std::vector<SAMPLE_BUFFER*>::iterator p = buffers.begin();
  while(p != buffers.end()) {
    if (p != buffers.begin()) delete *p; // first buffer points to an
                                         // outside buffer -> not
                                         // deleted here
    ++p;
  }

  std::vector<CHAIN*>::iterator q = chains.begin();
  while(q != chains.end()) {
    delete *q;
    ++q;
  }
}

void PRESET::parse(const std::string& formatted_string) {
  // --------
  DBC_REQUIRE(formatted_string.empty() == false);
  // --------

  parse_string_rep = formatted_string;
  chains.clear();
  chains.push_back(new CHAIN());

  CHAIN_OPERATOR *cop;
  GENERIC_CONTROLLER* gctrl;

  int param_index = 0;
  std::vector<std::string> tokens = string_to_words(formatted_string);
  std::vector<std::string>::const_iterator p = tokens.begin();
  while(p != tokens.end()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "Parsing: " + *p + ".");
    if (*p == "|") {
      add_chain();
      ++p;
      continue;
    }

    // Parse for parameters.
    std::vector<int> arg_numbers;
//      cerr << "*p = " << *p << endl;
//      cerr << "get_number_of_arguments(*p) = " << get_number_of_arguments(*p) << endl; 
    std::vector<std::string> ps_parts(get_number_of_arguments(*p));
    for(int i = 1; i <= get_number_of_arguments(*p); i++) {
        ecadebug->msg(ECA_DEBUG::system_objects, "  COP-argument "+get_argument_number(i, *p)+".");
        if(get_argument_number(i, *p)[0] == '%') {
            param_names.push_back(get_argument_number(i, *p).substr(1));
            arg_numbers.push_back(i);
            ps_parts[i-1] = "0.0";
            param_index++;
        } else {
            ps_parts[i-1] = get_argument_number(i, *p);
        }
    }
    string ps = "-" + get_argument_prefix(*p) + ":" + vector_to_string(ps_parts, ",");

    DYNAMIC_OBJECT<SAMPLE_SPECS::sample_type>* object = 0;

    ECA_CHAINSETUP csetup;
    cop = 0;
    cop = csetup.create_chain_operator(ps);
    if (cop == 0) cop = csetup.create_ladspa_plugin(ps);
    if (cop == 0) cop = csetup.create_vst_plugin(ps);
    if (cop != 0) {
      chains.back()->add_chain_operator(cop);
      chains.back()->selected_chain_operator_as_target();
      object = cop;
    }
    else {
      if (get_argument_prefix(ps) == "kx") 
	chains.back()->selected_controller_as_target();
      else {
	gctrl = 0;
	gctrl = csetup.create_controller(ps);
	if (gctrl != 0) {
	  chains.back()->add_controller(gctrl);
	}
        object = gctrl;
      }
    }

    for(int i = 0; i < static_cast<int>(arg_numbers.size()); i++) {
        param_objects.push_back(object);
	param_arg_indices.push_back(arg_numbers[i]);
    }

    ++p;
  }

  parsed_rep = true;

  // --------
  DBC_ENSURE(is_parsed() == true);
  // --------
}

void PRESET::add_chain(void) {
  chains.push_back(new CHAIN());
  buffers.push_back(new SAMPLE_BUFFER());
}


std::string PRESET::parameter_names(void) const {
  return vector_to_string(param_names, ",");
}

void PRESET::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  if (param > 0 && param <= static_cast<int>(param_objects.size())) {
    assert(param_objects.size() == param_arg_indices.size());
    param_objects[param-1]->set_parameter(param_arg_indices[param-1], value);
  }
}

CHAIN_OPERATOR::parameter_type PRESET::get_parameter(int param) const { 
  if (param > 0 && param <= static_cast<int>(param_objects.size())) {
    assert(param_objects.size() == param_arg_indices.size());
    return param_objects[param-1]->get_parameter(param_arg_indices[param-1]);
  }
  return(0.0f);
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
  std::vector<SAMPLE_BUFFER*>::iterator p = buffers.begin();
  while(p != buffers.end()) {
    (*p)->copy(*first_buffer);
    ++p;
  }

  std::vector<CHAIN*>::iterator q = chains.begin();
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
