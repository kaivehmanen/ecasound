// ------------------------------------------------------------------------
// eca-chain.cpp: Class representing an abstract audio signal chain.
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

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "samplebuffer.h"
#include "generic-controller.h"
#include "eca-chainop.h"
#include "audioio.h"
#include "file-preset.h"
#include "global-preset.h"
#include "audiofx_ladspa.h"
#include "eca-static-object-maps.h"
#include "eca-chain.h"
#include "eca-chainop.h"

#include "eca-error.h"
#include "eca-debug.h"

CHAIN::CHAIN (void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(chain) constuctor: CHAIN");
  muted_rep = false;
  sfx_rep = false;
  initialized_rep = false;
  input_id_repp = output_id_repp = 0;

  selected_chainop_repp = 0;
  selected_controller_repp = 0;
  selected_dynobj_repp = 0;

  selected_chainop_number_rep = 0;
  selected_controller_number_rep = 0;
}

CHAIN::~CHAIN (void) { 
  ecadebug->msg(ECA_DEBUG::system_objects,"CHAIN destructor!");

  for(vector<CHAIN_OPERATOR*>::iterator p = chainops_rep.begin(); p !=
	chainops_rep.end(); p++) {
    ecadebug->msg((*p)->status());
    delete *p;
  }

  for(vector<GENERIC_CONTROLLER*>::iterator p = gcontrollers_rep.begin(); p !=
	gcontrollers_rep.end(); p++) {
    delete *p;
  }
}

bool CHAIN::is_valid(void) const {
  if (input_id_repp == 0 ||
      output_id_repp == 0) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chain) Chain \"" + name() + "\" not valid.");
    return(false);
  }
  return(true);
}

void CHAIN::connect_input(AUDIO_IO* input) { input_id_repp = input; }
void CHAIN::connect_output(AUDIO_IO* output) { output_id_repp = output; }

void CHAIN::add_chain_operator(CHAIN_OPERATOR* chainop) {
  // --------
  // require:
  assert(chainop != 0);
  // --------

  chainops_rep.push_back(chainop);
  selected_chainop_repp = chainop;
  selected_chainop_number_rep = chainops_rep.size();
  sfx_rep = true;

  if (initialized_rep == true) 
    init(audioslot_repp, in_channels_rep, out_channels_rep);

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
  for(vector<CHAIN_OPERATOR*>::iterator p = chainops_rep.begin(); p !=
	chainops_rep.end(); p++) {
    ++n;
    if (n == selected_chain_operator()) {
      for(vector<GENERIC_CONTROLLER*>::iterator q = gcontrollers_rep.begin(); q !=
	    gcontrollers_rep.end(); q++) {
	if ((*p) == (*q)->target_pointer()) {
	  delete *q;
	  gcontrollers_rep.erase(q);
	  break;
	}
      }
      delete *p;
      chainops_rep.erase(p);
      break;
    }
  }
  if (chainops_rep.size() == 0) {
    sfx_rep = false;
  }
  if (initialized_rep == true) 
    init(audioslot_repp, in_channels_rep, out_channels_rep);

  // --------
  // ensure:
  assert(chainops_rep.size() == 0 && !is_processing() ||
	 chainops_rep.size() != 0 && is_processing());
  // --------
}

void CHAIN::remove_controller(void) {
  // --------
  // require:
  assert(selected_controller() > 0);
  assert(selected_controller() <= number_of_controllers());
  // --------

  int n = 0;
  for(vector<GENERIC_CONTROLLER*>::iterator q = gcontrollers_rep.begin(); q !=
	gcontrollers_rep.end(); q++) {
    if (n == selected_controller()) {
      delete *q;
      gcontrollers_rep.erase(q);
      break;
    }
  }
}

void CHAIN::set_parameter(int par_index, CHAIN_OPERATOR::parameter_type value) {
  // --------
  // require:
  assert(selected_chainop_number_rep > 0 && selected_chainop_number_rep <= number_of_chain_operators());
  assert(par_index > 0);
  // --------
  selected_chainop_repp->set_parameter(par_index, value);
}

CHAIN_OPERATOR::parameter_type CHAIN::get_parameter(int index) const {
  // --------
  // require:
  assert(index > 0 && selected_chain_operator() != 0);
  // --------
  return(selected_chainop_repp->get_parameter(index));
}

void CHAIN::add_controller(GENERIC_CONTROLLER* gcontroller) {
  // --------
  // require:
  assert(gcontroller != 0);
  assert(selected_dynobj_repp != 0);
  // --------
  gcontroller->assign_target(selected_dynobj_repp);
  gcontrollers_rep.push_back(gcontroller);
  ecadebug->msg("(eca-chain) " + gcontroller->status());
  selected_controller_repp = gcontroller;
  selected_controller_number_rep = gcontrollers_rep.size();
}

void CHAIN::clear(void) {
  for(vector<CHAIN_OPERATOR*>::iterator p = chainops_rep.begin(); p != chainops_rep.end(); p++) {
    delete *p;
  }
  chainops_rep.resize(0);
  for(vector<GENERIC_CONTROLLER*>::iterator p = gcontrollers_rep.begin(); p !=
	gcontrollers_rep.end(); p++) {
    delete *p;
  }
  gcontrollers_rep.resize(0);
}

void CHAIN::select_chain_operator(int index) {
  for(int chainop_sizet = 0; chainop_sizet != static_cast<int>(chainops_rep.size()); chainop_sizet++) {
    if (chainop_sizet + 1 == index) {
      selected_chainop_repp = chainops_rep[chainop_sizet];
      selected_chainop_number_rep = index;
    }
  }
}

void CHAIN::select_controller(int index) {
  for(int gcontroller_sizet = 0; gcontroller_sizet != static_cast<int>(gcontrollers_rep.size()); gcontroller_sizet++) {
    if (gcontroller_sizet + 1 == index) {
      selected_controller_repp = gcontrollers_rep[gcontroller_sizet];
      selected_controller_number_rep = index;
    }
  }
}

void CHAIN::selected_chain_operator_as_target(void) {
  // --------
  // require:
  assert(selected_chainop_repp != 0);
  // --------
  selected_dynobj_repp = selected_chainop_repp;
  // --------
  // ensure:
  assert(selected_dynobj_repp == selected_chainop_repp);
  // --------
}

void CHAIN::selected_controller_as_target(void) {
  // --------
  // require:
  assert(selected_controller_repp != 0);
  // --------
  selected_dynobj_repp = selected_controller_repp;
  // --------
  // ensure:
  assert(selected_dynobj_repp == selected_controller_repp);
  // --------
}

void CHAIN::init(SAMPLE_BUFFER* sbuf, int in_channels, int out_channels) {
  // --------
  // require:
  assert(input_id_repp != 0 || in_channels != 0);
  assert(output_id_repp != 0 || out_channels != 0);
  // --------

  audioslot_repp = sbuf;

  in_channels_rep = in_channels;
  out_channels_rep = out_channels;
  if (in_channels == 0) in_channels_rep = input_id_repp->channels();
  if (out_channels == 0) out_channels_rep = output_id_repp->channels();

  int init_channels = in_channels_rep;
  audioslot_repp->number_of_channels(init_channels);
  for(int p = 0; p != static_cast<int>(chainops_rep.size()); p++) {
    chainops_rep[p]->init(audioslot_repp);
    init_channels = chainops_rep[p]->output_channels(init_channels);
    audioslot_repp->number_of_channels(init_channels);
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
  if (muted_rep == false) {
    if (sfx_rep == true) {
      for(int p = 0; p != static_cast<int>(chainops_rep.size()); p++) {
	audioslot_repp->number_of_channels(chainops_rep[p]->output_channels(audioslot_repp->number_of_channels()));
	chainops_rep[p]->process();
      }
      //    audioslot_repp.limit_values();
    }
  }
  else {
    audioslot_repp->make_silent();
  }
}

void CHAIN::controller_update(void) {
  for(int gcontroller_sizet = 0; gcontroller_sizet < static_cast<int>(gcontrollers_rep.size()); gcontroller_sizet++) {
    gcontrollers_rep[gcontroller_sizet]->process();
  }
}

void CHAIN::refresh_parameters(void) {
  for(int chainop_sizet = 0; chainop_sizet != static_cast<int>(chainops_rep.size()); chainop_sizet++) {
    for(int n = 0; n < chainops_rep[chainop_sizet]->number_of_params(); n++) {
      chainops_rep[chainop_sizet]->set_parameter(n + 1, 
					       chainops_rep[chainop_sizet]->get_parameter(n + 1));
    }
  }
}

string CHAIN::to_string(void) const {
  MESSAGE_ITEM t; 

  FILE_PRESET* fpreset;
  GLOBAL_PRESET* gpreset;

  int q = 0;
  while (q < static_cast<int>(chainops_rep.size())) {
    fpreset = 0;
    fpreset = dynamic_cast<FILE_PRESET*>(chainops_rep[q]);
    if (fpreset != 0) {
      t << "-pf:" << fpreset->filename() << " ";
    }
    else {
      gpreset = 0;
      gpreset = dynamic_cast<GLOBAL_PRESET*>(chainops_rep[q]);
      if (gpreset != 0) {
	t << "-pn:" << gpreset->name() << " ";
      }
      else {
        t << chain_operator_to_string(chainops_rep[q]) << " ";
      }
    }
    ++q;
  }

  return(t.to_string());
}

string CHAIN::chain_operator_to_string(CHAIN_OPERATOR* chainop) const {
  MESSAGE_ITEM t;
  
  // >--
  // special handling for LADPSA-plugins
  EFFECT_LADSPA* ladspa = dynamic_cast<EFFECT_LADSPA*>(chainop);
  if (ladspa != 0) {
    t << "-eli:" << ladspa->unique_number();
    if (chainop->number_of_params() > 0) t << ",";
  }
  else {
    t << "-" << eca_chain_operator_map.object_identifier(chainop);
    if (chainop->number_of_params() > 0) t << ":";
  }
  // --<

  for(int n = 0; n < chainop->number_of_params(); n++) {
    t << chainop->get_parameter(n + 1);
    if (n + 1 < chainop->number_of_params()) t << ",";
  }

  vector<GENERIC_CONTROLLER*>::size_type p = 0;
  while (p < gcontrollers_rep.size()) {
    if (chainop == gcontrollers_rep[p]->target_pointer()) {
      t << " " << controller_to_string(gcontrollers_rep[p]);
    }
    ++p;
  } 

  return(t.to_string());
}

string CHAIN::controller_to_string(GENERIC_CONTROLLER* gctrl) const {
  MESSAGE_ITEM t; 
  t << "-" << eca_controller_map.object_identifier(gctrl);
  t << ":";
  for(int n = 0; n < gctrl->number_of_params(); n++) {
    t << gctrl->get_parameter(n + 1);
    if (n + 1 < gctrl->number_of_params()) t << ",";
  }

  return(t.to_string());
}
