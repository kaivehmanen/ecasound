// ------------------------------------------------------------------------
// eca-chain.cpp: Class representing an abstract audio signal chain.
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cctype>
#include <string>
#include <vector>

#include <unistd.h>

#include <kvu_message_item.h>
#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "samplebuffer.h"
#include "generic-controller.h"
#include "eca-chainop.h"
#include "audioio.h"
#include "file-preset.h"
#include "global-preset.h"
#include "audiofx_ladspa.h"
#include "eca-object-factory.h"
#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-chain.h"
#include "eca-chainop.h"

#include "eca-error.h"
#include "eca-logger.h"

/* Debug controller source values */ 
// #define DEBUG_CONTROLLERS

#ifdef DEBUG_CONTROLLERS
#define DEBUG_CTRL_STATEMENT(x) x
#else
#define DEBUG_CTRL_STATEMENT(x) ((void)0)
#endif

CHAIN::CHAIN (void)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(chain) constuctor: CHAIN");
  muted_rep = false;
  sfx_rep = false;
  initialized_rep = false;
  input_id_rep = output_id_rep = -1;

  selected_chainop_repp = 0;
  selected_controller_repp = 0;
  selected_dynobj_repp = 0;

  selected_chainop_number_rep = 0;
  selected_controller_number_rep = 0;
  selected_chainop_parameter_rep = 0;
  selected_controller_parameter_rep = 0;
}

CHAIN::~CHAIN (void)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects,"CHAIN destructor!");

  for(std::vector<CHAIN_OPERATOR*>::iterator p = chainops_rep.begin(); p !=
	chainops_rep.end(); p++) {
    ECA_LOG_MSG(ECA_LOGGER::info, (*p)->status());
    delete *p;
  }

  for(std::vector<GENERIC_CONTROLLER*>::iterator p = gcontrollers_rep.begin(); p !=
	gcontrollers_rep.end(); p++) {
    delete *p;
  }
}

/**
 * Whether chain is in a valid state (= ready for processing)?
 */
bool CHAIN::is_valid(void) const
{
  if (input_id_rep == -1 ||
      output_id_rep == -1) {
    ECA_LOG_MSG(ECA_LOGGER::system_objects, "(eca-chain) Chain \"" + name() + "\" not valid.");
    return(false);
  }
  return(true);
}

/**
 * Connects input to chain
 */
void CHAIN::connect_input(int input) { input_id_rep = input; }

/**
 * Connects output to chain
 */
void CHAIN::connect_output(int output) { output_id_rep = output; }

/**
 * Disconnects input
 */
void CHAIN::disconnect_input(void) { input_id_rep = -1; initialized_rep = false; }

/**
 * Disconnects output
 */
void CHAIN::disconnect_output(void) { output_id_rep = -1; initialized_rep = false; }

/**
 * Disconnects the sample buffer
 */
void CHAIN::disconnect_buffer(void) { audioslot_repp = 0; initialized_rep = false; release(); }

/**
 * Adds the chain operator to the end of the chain
 *
 * require:
 *  chainop != 0
 *
 * ensure:
 *  selected_chain_operator() == number_of_chain_operators()
 *  is_processing()
 *  is_initialized() != true
 */
void CHAIN::add_chain_operator(CHAIN_OPERATOR* chainop)
{
  // --------
  DBC_REQUIRE(chainop != 0);
  // --------

  ECA_SAMPLERATE_AWARE* srateobj = dynamic_cast<ECA_SAMPLERATE_AWARE*>(chainop);
  if (srateobj != 0) {
    srateobj->set_samples_per_second(samples_per_second());
  }

  chainops_rep.push_back(chainop);
  selected_chainop_repp = chainop;
  selected_chainop_number_rep = chainops_rep.size();
  sfx_rep = true;
  initialized_rep = false;

  // --------
  DBC_ENSURE(selected_chain_operator() == number_of_chain_operators());
  DBC_ENSURE(is_processing() == true);
  DBC_ENSURE(is_initialized() != true);
  // --------
}

/**
 * Removes the selected chain operator
 *
 * require:
 *  selected_chain_operator() <= number_of_chain_operators();
 *  selected_chain_operator() > 0
 *
 * ensure:
 *  (chainsops.size() == 0 && is_processing()) ||
 *  (chainsops.size() != 0 && !is_processing()) &&
 *  is_initialized() != true
 */
void CHAIN::remove_chain_operator(void)
{
  // --------
  DBC_REQUIRE(selected_chain_operator() > 0);
  DBC_REQUIRE(selected_chain_operator() <= number_of_chain_operators());
  // --------

  int n = 0;
  for(std::vector<CHAIN_OPERATOR*>::iterator p = chainops_rep.begin(); p !=
	chainops_rep.end(); p++) {
    ++n;
    if (n == selected_chain_operator()) {
      for(std::vector<GENERIC_CONTROLLER*>::iterator q = gcontrollers_rep.begin(); q !=
	    gcontrollers_rep.end(); q++) {
	if ((*p) == (*q)->target_pointer()) {
	  delete *q;
	  gcontrollers_rep.erase(q);
	  select_controller(0);
	  break;
	}
      }
      delete *p;
      chainops_rep.erase(p);
      select_chain_operator(0);
      break;
    }
  }
  if (chainops_rep.size() == 0) {
    sfx_rep = false;
  }
  initialized_rep = false; 

  // --------
  DBC_ENSURE(chainops_rep.size() == 0 && !is_processing() ||
	     chainops_rep.size() != 0 && is_processing());
  DBC_ENSURE(is_initialized() != true);
  // --------
}

/**
 * Returns the name of selected chain operator.
 *
 * require:
  *  selected_chain_operator() != 0
 */
string CHAIN::chain_operator_name(void) const
{
  // --------
  DBC_REQUIRE(selected_chain_operator() > 0);
  // --------
  return(selected_chainop_repp->name());
}

/**
 * Returns the name of selected chain operator parameter.
 *
 * require:
  *  selected_chain_operator() != 0
  *  selected_chain_operator_parameter() != 0
 */
string CHAIN::chain_operator_parameter_name(void) const
{
  // --------
  DBC_REQUIRE(selected_chain_operator() > 0);
  DBC_REQUIRE(selected_chain_operator_parameter() > 0);
  // --------
  return(selected_chainop_repp->get_parameter_name(selected_chain_operator_parameter()));
}

/**
 * Returns the total number of parameters for the 
 * selected chain operator.
 *
 * require:
 *  selected_chain_operator() != 0
 */
int CHAIN::number_of_chain_operator_parameters(void) const
{
  // --------
  DBC_REQUIRE(selected_chain_operator() > 0);
  // --------
  return(selected_chainop_repp->number_of_params());
}

/**
 * Returns the name of selected controller.
 *
 * require:
 *  selected_controller() != 0
 */
string CHAIN::controller_name(void) const
{
  // --------
  DBC_REQUIRE(selected_controller() > 0);
  // --------
  return(selected_controller_repp->name());
}

/**
 * Sets the parameter value (selected chain operator) 
 *
 * @param index parameter number
 * @param value new value
 *
 * require:
 *  selected_chainop_number > 0 && selected_chainop_number <= number_of_chain_operators() &&
 *  selected_chain_operator_parameter() > 0
 */
void CHAIN::set_parameter(CHAIN_OPERATOR::parameter_t value)
{
  // --------
  DBC_REQUIRE(selected_chainop_number_rep > 0 && selected_chainop_number_rep <= number_of_chain_operators());
  DBC_REQUIRE(selected_chain_operator_parameter() > 0);
  // --------
  selected_chainop_repp->set_parameter(selected_chainop_parameter_rep, value);
}

/**
 * Gets the parameter value (selected chain operator) 
 *
 * @param index parameter number
 *
 * require:
 *  selected_chain_operator_parameter() > 0 &&
 *  selected_chain_operator() != 0
 */
CHAIN_OPERATOR::parameter_t CHAIN::get_parameter(void) const
{
  // --------
  DBC_REQUIRE(selected_chain_operator_parameter() > 0);
  DBC_REQUIRE(selected_chain_operator() != 0);
  // --------
  return(selected_chainop_repp->get_parameter(selected_chainop_parameter_rep));
}

/**
 * Adds a generic controller and assign it to selected dynamic object
 *
 * require:
 *  gcontroller != 0
 *  selected_dynobj != 0
 */
void CHAIN::add_controller(GENERIC_CONTROLLER* gcontroller)
{
  // --------
  DBC_REQUIRE(gcontroller != 0);
  DBC_REQUIRE(selected_dynobj_repp != 0);
  // --------

  gcontroller->set_samples_per_second(samples_per_second());
#ifndef ECA_DISABLE_EFFECTS
  gcontroller->assign_target(selected_dynobj_repp);
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(eca-chain) " + gcontroller->status());
#endif
  gcontrollers_rep.push_back(gcontroller);
  selected_controller_repp = gcontroller;
  selected_controller_number_rep = gcontrollers_rep.size();
}

/**
 * Removes the selected controller
 *
 * require:
 *  selected_controller() <= number_of_controllers();
 *  selected_controller() > 0
 */
void CHAIN::remove_controller(void)
{
  // --------
  DBC_REQUIRE(selected_controller() > 0);
  DBC_REQUIRE(selected_controller() <= number_of_controllers());
  // --------

  int n = 0;
  for(std::vector<GENERIC_CONTROLLER*>::iterator q = gcontrollers_rep.begin(); 
      q != gcontrollers_rep.end(); 
      q++) {
    if ((n + 1) == selected_controller()) {
      delete *q;
      gcontrollers_rep.erase(q);
      select_controller(0);
      break;
    }
    ++n;
  }
}

/**
 * Clears chain (removes all chain operators and controllers)
 */
void CHAIN::clear(void)
{
  for(std::vector<CHAIN_OPERATOR*>::iterator p = chainops_rep.begin(); p != chainops_rep.end(); p++) {
    delete *p;
    *p = 0;
  }
  chainops_rep.resize(0);
  for(std::vector<GENERIC_CONTROLLER*>::iterator p = gcontrollers_rep.begin(); p !=
	gcontrollers_rep.end(); p++) {
    delete *p;
    *p = 0;
  }
  gcontrollers_rep.resize(0);

  initialized_rep = false;
  sfx_rep = false;
}

/**
 * Selects a chain operator. If no chain operators
 * are found with 'index', with index 'index'. 
 *
 * require:
 *  index > 0
 *
 * ensure:
 *  index == selected_chain_operator() || 
 *  selected_chain_operator() == 0
 */
void CHAIN::select_chain_operator(int index)
{
  selected_chainop_repp = 0;
  selected_chainop_number_rep = 0;
  for(int chainop_sizet = 0; chainop_sizet != static_cast<int>(chainops_rep.size()); chainop_sizet++) {
    if (chainop_sizet + 1 == index) {
      selected_chainop_repp = chainops_rep[chainop_sizet];
      selected_chainop_number_rep = index;
    }
  }
}

/**
 * Selects a chain operator parameter
 *
 * require:
 *  index > 0
 *  selected_chain_operator() != 0
 *  index <= selected_chain_operator()->number_of_params()
 *
 * ensure:
 *  index == selected_chain_operator_parameter()
 */
void CHAIN::select_chain_operator_parameter(int index)
{
  selected_chainop_parameter_rep = index;
}


/**
 * Selects a controller.
 *
 * require:
 *  index > 0
 *
 * ensure:
 *  index == selected_controller() ||
 *  selected_controller() == 0
 */
void CHAIN::select_controller(int index)
{
  selected_controller_repp = 0;
  selected_controller_number_rep = 0;
  for(int gcontroller_sizet = 0; gcontroller_sizet != static_cast<int>(gcontrollers_rep.size()); gcontroller_sizet++) {
    if (gcontroller_sizet + 1 == index) {
      selected_controller_repp = gcontrollers_rep[gcontroller_sizet];
      selected_controller_number_rep = index;
    }
  }
}

/**
 * Selects a controller parameter
 *
 * require:
 *  index > 0
 *  selected_controller() != 0
 *  index <= selected_controller()->number_of_params()
 *
 * ensure:
 *  index == selected_controller_parameter()
 */
void CHAIN::select_controller_parameter(int index)
{
  selected_controller_parameter_rep = index;
}

/**
 * Use current selected chain operator as 
 * target for parameters control.
 *
 * require:
 *   selected_chain_operator() != 0
 *
 * ensure:
 *   selected_target() == selected_chain_operator()
 */
void CHAIN::selected_chain_operator_as_target(void)
{
  // --------
  DBC_REQUIRE(selected_chainop_repp != 0);
  // --------
  selected_dynobj_repp = selected_chainop_repp;
  // --------
  DBC_ENSURE(selected_dynobj_repp == selected_chainop_repp);
  // --------
}

/**
 * Use current selected controller as 
 * target for parameter control.
 *
 * require:
 *   selected_controller() != 0
 *
 * ensure:
 *   selected_target() == selected_controller()
 */
void CHAIN::selected_controller_as_target(void)
{
  // --------
  DBC_REQUIRE(selected_controller_repp != 0);
  // --------
  selected_dynobj_repp = selected_controller_repp;
  // --------
  DBC_ENSURE(selected_dynobj_repp == selected_controller_repp);
  // --------
}

/**
 * Prepares chain for processing. All further processing
 * will be done using the buffer pointer by 'sbuf'.
 * If all parameters are zero, previously specified 
 * parameters are used (state re-initialization).
 *
 * require:
 *  input_id != 0 || in_channels != 0
 *  output_id != 0 || out_channels != 0
 *  audioslot_repp != 0 || sbuf != 0
 *
 * ensure:
 *  is_initialized() == true
 */
void CHAIN::init(SAMPLE_BUFFER* sbuf, int in_channels, int out_channels)
{
  // --------
  DBC_REQUIRE(in_channels != 0 || in_channels_rep != 0);
  DBC_REQUIRE(out_channels != 0 || out_channels_rep != 0);
  DBC_REQUIRE(sbuf != 0 || audioslot_repp != 0);
  // --------

  DBC_CHECK(samples_per_second() > 0);

  if (sbuf != 0) audioslot_repp = sbuf;
  if (in_channels != 0) in_channels_rep = in_channels;
  if (out_channels != 0) out_channels_rep = out_channels;

  int init_channels = in_channels_rep;
  audioslot_repp->number_of_channels(init_channels);
  for(size_t p = 0; p != chainops_rep.size(); p++) {
    chainops_rep[p]->init(audioslot_repp);
    init_channels = chainops_rep[p]->output_channels(init_channels);
    audioslot_repp->number_of_channels(init_channels);
  }

  for(size_t p = 0; p != gcontrollers_rep.size(); p++) {
    gcontrollers_rep[p]->init();
  }

  refresh_parameters();
  initialized_rep = true;

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(chain) Initialized chain " +
		name() + 
		" with " +
		kvu_numtostr(chainops_rep.size()) +
		" chainops and " +
		kvu_numtostr(gcontrollers_rep.size()) +
		" gcontrollers. Sbuf points to " +
		kvu_numtostr(reinterpret_cast<long int>(audioslot_repp)) + ".");
  
  // --------
  DBC_ENSURE(is_initialized() == true);
  // --------
}

/** 
 * Releases all buffers assigned to chain operators.
 */
void CHAIN::release(void)
{
  for(size_t p = 0; p != chainops_rep.size(); p++) {
    chainops_rep[p]->release();
  }
  initialized_rep = false;

  // ---------
  DBC_ENSURE(is_initialized() != true);
  // ---------
}

/**
 * Processes chain data with all chain operators.
 *
 * require:
 *  is_initialized() == true
 */
void CHAIN::process(void)
{
  // --------
  DBC_REQUIRE(is_initialized() == true);
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

/**
 * Calculates/fetches new values for all controllers.
 */
void CHAIN::controller_update(void)
{
  for(size_t n = 0; n < gcontrollers_rep.size(); n++) {
    DEBUG_CTRL_STATEMENT(GENERIC_CONTROLLER* ptr = gcontrollers_rep[n]);

    gcontrollers_rep[n]->value();
    gcontrollers_rep[n]->seek_position_in_samples_advance(audioslot_repp->length_in_samples());

    DEBUG_CTRL_STATEMENT(std::cerr << "trace: " << ptr->name());
    DEBUG_CTRL_STATEMENT(std::cerr << "; value " << ptr->source_pointer()->value() << "." << std::endl);
  }
}

/**
 * Re-initializes all effect parameters.
 */
void CHAIN::refresh_parameters(void)
{
  for(int chainop_sizet = 0; chainop_sizet != static_cast<int>(chainops_rep.size()); chainop_sizet++) {
    for(int n = 0; n < chainops_rep[chainop_sizet]->number_of_params(); n++) {
      chainops_rep[chainop_sizet]->set_parameter(n + 1, 
					       chainops_rep[chainop_sizet]->get_parameter(n + 1));
    }
  }
}

/**
 * Converts chain to a formatted string.
 */
string CHAIN::to_string(void) const
{
  MESSAGE_ITEM t; 

  FILE_PRESET* fpreset;
  GLOBAL_PRESET* gpreset;

  int q = 0;
  while (q < static_cast<int>(chainops_rep.size())) {
#ifndef ECA_DISABLE_EFFECTS
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
#endif
    ++q;
  }

  return(t.to_string());
}

string CHAIN::chain_operator_to_string(CHAIN_OPERATOR* chainop) const
{
  MESSAGE_ITEM t;
  
  // >--
  // special handling for LADPSA-plugins
#ifndef ECA_DISABLE_EFFECTS
  EFFECT_LADSPA* ladspa = dynamic_cast<EFFECT_LADSPA*>(chainop);
  if (ladspa != 0) {
    t << "-eli:" << ladspa->unique_number();
    if (chainop->number_of_params() > 0) t << ",";
  }
  else {
    ECA_OBJECT_MAP& copmap = ECA_OBJECT_FACTORY::chain_operator_map();
    ECA_PRESET_MAP& presetmap = ECA_OBJECT_FACTORY::preset_map();
    
    string idstring = copmap.object_identifier(chainop);
    if (idstring.size() == 0) {
      idstring = presetmap.object_identifier(chainop);
    }
    if (idstring.size() == 0) {
      ECA_LOG_MSG(ECA_LOGGER::errors,
		  "(eca-chain) Unable to save chain operator \"" +
		  chainop->name() + "\".");
      return(t.to_string());
    }
     
    t << "-" << idstring;
    if (chainop->number_of_params() > 0) t << ":";
  }
#endif
  // --<

  for(int n = 0; n < chainop->number_of_params(); n++) {
    t << chainop->get_parameter(n + 1);
    if (n + 1 < chainop->number_of_params()) t << ",";
  }

  std::vector<GENERIC_CONTROLLER*>::size_type p = 0;
  while (p < gcontrollers_rep.size()) {
    if (chainop == gcontrollers_rep[p]->target_pointer()) {
      t << " " << controller_to_string(gcontrollers_rep[p]);
    }
    ++p;
  } 

  return(t.to_string());
}

string CHAIN::controller_to_string(GENERIC_CONTROLLER* gctrl) const
{
  MESSAGE_ITEM t;
  ECA_OBJECT_MAP& ctrlmap = ECA_OBJECT_FACTORY::controller_map();
  string idstring = ctrlmap.object_identifier(gctrl);

  if (idstring.size() == 0) {
    ECA_LOG_MSG(ECA_LOGGER::errors, 
		"(eca-chain) Unable to save controller \"" +
		gctrl->name() + "\".");
    return(t.to_string());
  }

  t << "-" << idstring;
  t << ":";
  for(int n = 0; n < gctrl->number_of_params(); n++) {
    t << gctrl->get_parameter(n + 1);
    if (n + 1 < gctrl->number_of_params()) t << ",";
  }

  std::vector<GENERIC_CONTROLLER*>::size_type p = 0;
  while (p < gcontrollers_rep.size()) {
    if (gctrl == gcontrollers_rep[p]->target_pointer()) {
      t << " -kx " << controller_to_string(gcontrollers_rep[p]);
    }
    ++p;
  } 

  return(t.to_string());
}

/**
 * Reimplemented from ECA_SAMPLERATE_AWARE
 */
void CHAIN::set_samples_per_second(SAMPLE_SPECS::sample_rate_t v)
{
  for(size_t p = 0; p != chainops_rep.size(); p++) {
    CHAIN_OPERATOR* temp = chainops_rep[p];
    ECA_SAMPLERATE_AWARE* srateobj = dynamic_cast<ECA_SAMPLERATE_AWARE*>(temp);
    if (srateobj != 0) {
      ECA_LOG_MSG(ECA_LOGGER::user_objects,
		    "(eca-chain) sample rate change, chain '" +
		    name() + "' object '" +
		    temp->name() + "' rate " +
		    kvu_numtostr(v) + ".");
      srateobj->set_samples_per_second(v);
    }
  }

  for(size_t p = 0; p != gcontrollers_rep.size(); p++) {
    ECA_LOG_MSG(ECA_LOGGER::user_objects,
		"(eca-chain) sample rate change, chain '" +
		name() + "' object '" +
		gcontrollers_rep[p]->name() + "' rate " +
		kvu_numtostr(v) + ".");
    gcontrollers_rep[p]->set_samples_per_second(v);
  }
      
  ECA_SAMPLERATE_AWARE::set_samples_per_second(v);
}

/**
 * Reimplemented from ECA_AUDIO_POSITION.
 */
void CHAIN::seek_position(void)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects,
		"(eca-chain) seek position, to pos " +
		kvu_numtostr(position_in_seconds()) + ".");

  for(size_t p = 0; p != gcontrollers_rep.size(); p++) {
    gcontrollers_rep[p]->seek_position_in_samples(position_in_samples());
  }
}
