// ------------------------------------------------------------------------
// eca-chain.cpp: Class representing abstract audio signal chain.
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
#include <vector>
#include <unistd.h>

#include <kvutils.h>

#include "eca-error.h"
#include "eca-debug.h"
#include "samplebuffer.h"
#include "eca-chainop-map.h"
#include "eca-controller-map.h"
#include "eca-chain.h"

CHAIN::CHAIN (int bsize, int channels) 
  : audioslot(bsize, channels) {
  ecadebug->msg(1, "(chain) constuctor: CHAIN( " 
		+ kvu_numtostr(bsize) + ", " + 
		  kvu_numtostr(channels) + ")");
  muted = false;
  sfx = false;
  initialized_rep = false;
  input_id = output_id = 0;

  selected_chainop = 0;
  selected_controller_rep = 0;
  selected_dynobj = 0;

  selected_chainop_number = 0;
  selected_controller_number = 0;
}

CHAIN::~CHAIN (void) { 
  ecadebug->msg(1,"CHAIN destructor!");

  for(vector<CHAIN_OPERATOR*>::iterator p = chainops.begin(); p !=
	chainops.end(); p++) {
    ecadebug->msg((*p)->status());
    delete *p;
  }

  for(vector<GENERIC_CONTROLLER*>::iterator p = gcontrollers.begin(); p !=
	gcontrollers.end(); p++) {
    delete *p;
  }
}

bool CHAIN::is_valid(void) const {
  if (input_id == 0 ||
      output_id == 0) {
    ecadebug->msg(1, "(eca-chain) Chain \"" + name() + "\" not valid.");
    return(false);
  }
  return(true);
}

void CHAIN::connect_input(AUDIO_IO* input) { input_id = input; }
void CHAIN::connect_output(AUDIO_IO* output) { output_id = output; }

void CHAIN::add_chain_operator(CHAIN_OPERATOR* chainop) {
  // --------
  // require:
  assert(chainop != 0);
  // --------

  chainops.push_back(chainop);
  selected_chainop = chainop;
  selected_chainop_number = chainops.size();
  sfx = true;

  // --------
  // ensure:
  assert(selected_chain_operator() == number_of_chain_operators());
  assert(is_processing() == true);
  // --------
}

void CHAIN::remove_chain_operator(void) {
  // --------
  // require:
  assert(selected_chain_operator() > 0);
  assert(selected_chain_operator() <= number_of_chain_operators());
  // --------

  int n = 0;
  for(vector<CHAIN_OPERATOR*>::iterator p = chainops.begin(); p !=
	chainops.end(); p++) {
    ++n;
    if (n == selected_chain_operator()) {
      for(vector<GENERIC_CONTROLLER*>::iterator q = gcontrollers.begin(); q !=
	    gcontrollers.end(); q++) {
	if ((*p) == (*q)->target_pointer()) {
	  delete *q;
	  gcontrollers.erase(q);
	  break;
	}
      }
      delete *p;
      chainops.erase(p);
      break;
    }
  }
  if (chainops.size() == 0) {
    sfx = false;
  }

  // --------
  // ensure:
  assert(chainops.size() == 0 && !is_processing() ||
	 chainops.size() != 0 && is_processing());
  // --------
}

void CHAIN::set_parameter(int par_index, DYNAMIC_PARAMETERS::parameter_type value) {
  // --------
  // require:
  assert(selected_chainop_number > 0 && selected_chainop_number <= number_of_chain_operators());
  assert(par_index > 0);
  // --------
  selected_chainop->set_parameter(par_index, value);
}

DYNAMIC_PARAMETERS::parameter_type CHAIN::get_parameter(int index) const {
  // --------
  // require:
  assert(index > 0 && selected_chain_operator() != 0);
  // --------
  return(selected_chainop->get_parameter(index));
}

void CHAIN::add_controller(GENERIC_CONTROLLER* gcontroller) {
  // --------
  // require:
  assert(gcontroller != 0);
  assert(selected_dynobj != 0);
  // --------
  gcontroller->assign_target(selected_dynobj);
  gcontrollers.push_back(gcontroller);
  ecadebug->msg("(eca-chain) " + gcontroller->status());
  selected_controller_rep = gcontroller;
  selected_controller_number = gcontrollers.size();
}

void CHAIN::clear(void) {
  for(vector<CHAIN_OPERATOR*>::iterator p = chainops.begin(); p != chainops.end(); p++) {
    delete *p;
  }
  chainops.resize(0);
  for(vector<GENERIC_CONTROLLER*>::iterator p = gcontrollers.begin(); p !=
	gcontrollers.end(); p++) {
    delete *p;
  }
  gcontrollers.resize(0);
}

void CHAIN::select_chain_operator(int index) {
  for(int chainop_sizet = 0; chainop_sizet != static_cast<int>(chainops.size()); chainop_sizet++) {
    if (chainop_sizet + 1 == index) {
      selected_chainop = chainops[chainop_sizet];
      selected_chainop_number = index;
    }
  }
}

void CHAIN::select_controller(int index) {
  for(int gcontroller_sizet = 0; gcontroller_sizet != static_cast<int>(gcontrollers.size()); gcontroller_sizet++) {
    if (gcontroller_sizet + 1 == index) {
      selected_controller_rep = gcontrollers[gcontroller_sizet];
      selected_controller_number = index;
    }
  }
}

void CHAIN::selected_chain_operator_as_target(void) {
  // --------
  // require:
  assert(selected_chainop != 0);
  // --------
  selected_dynobj = selected_chainop;
  // --------
  // ensure:
  assert(selected_dynobj == selected_chainop);
  // --------
}

void CHAIN::selected_controller_as_target(void) {
  // --------
  // require:
  assert(selected_controller_rep != 0);
  // --------
  selected_dynobj = selected_controller_rep;
  // --------
  // ensure:
  assert(selected_dynobj == selected_controller_rep);
  // --------
}

void CHAIN::init(void) {
  // --------
  // require:
  assert(input_id != 0);
  assert(output_id != 0);
  // --------

  in_channels_rep = input_id->channels();
  out_channels_rep = output_id->channels();

  int init_channels = in_channels_rep;
  audioslot.number_of_channels(init_channels);
  for(int p = 0; p != static_cast<int>(chainops.size()); p++) {
    chainops[p]->init(&audioslot);
    init_channels = chainops[p]->output_channels(init_channels);
    audioslot.number_of_channels(init_channels);
  }

  refresh_parameters();
  initialized_rep = true;

  // --------
  // ensure:
  assert(is_initialized() == true);
  // --------
}

void CHAIN::process(void) {
  // --------
  // require:
  assert(is_initialized() == true);
  // --------

  controller_update();
  if (muted == false) {
    if (sfx == true) {
      for(int p = 0; p != static_cast<int>(chainops.size()); p++) {
	audioslot.number_of_channels(chainops[p]->output_channels(audioslot.number_of_channels()));
	chainops[p]->process();
      }
      //    audioslot.limit_values();
    }
  }
  else {
    audioslot.make_silent();
  }
}

void CHAIN::controller_update(void) {
  for(int gcontroller_sizet = 0; gcontroller_sizet < static_cast<int>(gcontrollers.size()); gcontroller_sizet++) {
    gcontrollers[gcontroller_sizet]->process();
  }
}

void CHAIN::refresh_parameters(void) {
  for(int chainop_sizet = 0; chainop_sizet != static_cast<int>(chainops.size()); chainop_sizet++) {
    for(int n = 0; n < chainops[chainop_sizet]->number_of_params(); n++) {
      chainops[chainop_sizet]->set_parameter(n + 1, 
					       chainops[chainop_sizet]->get_parameter(n + 1));
    }
  }
}

string CHAIN::to_string(void) const {
  MESSAGE_ITEM t; 

  int q = 0;
  while (q < static_cast<int>(chainops.size())) {
    t << chain_operator_to_string(chainops[q]) << " ";
    ++q;
  }

  return(t.to_string());
}

string CHAIN::chain_operator_to_string(CHAIN_OPERATOR* chainop) const {
  MESSAGE_ITEM t;
  t << "-" << ECA_CHAIN_OPERATOR_MAP::object_prefix_map[chainop->name()];
  if (chainop->number_of_params() > 0) t << ":";
  for(int n = 0; n < chainop->number_of_params(); n++) {
    t << chainop->get_parameter(n + 1);
    if (n + 1 < chainop->number_of_params()) t << ",";
  }

  vector<GENERIC_CONTROLLER*>::size_type p = 0;
  while (p < gcontrollers.size()) {
    if (chainop == gcontrollers[p]->target_pointer()) {
      t << " " << controller_to_string(gcontrollers[p]);
    }
    ++p;
  } 

  return(t.to_string());
}

string CHAIN::controller_to_string(GENERIC_CONTROLLER* gctrl) const {
  MESSAGE_ITEM t; 
  t << "-" << ECA_CONTROLLER_MAP::object_prefix_map[gctrl->name()];
  t << ":";
  for(int n = 0; n < gctrl->number_of_params(); n++) {
    t << gctrl->get_parameter(n + 1);
    if (n + 1 < gctrl->number_of_params()) t << ",";
  }

  return(t.to_string());
}
