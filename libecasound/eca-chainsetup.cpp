// ------------------------------------------------------------------------
// eca-chainsetup.cpp: Class representing an ecasound chainsetup object.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <cstring>
#include <algorithm> /* find() */
#include <fstream>
#include <vector>
#include <iostream>

#include <kvutils/dbc.h>
#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "eca-resources.h"
#include "eca-session.h"

#include "generic-controller.h"
#include "eca-chainop.h"

#include "audioio.h"
#include "audioio-types.h"
#include "audioio-loop.h"
#include "audioio-null.h"

#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-static-object-maps.h"

#include "midiio.h"
#include "midi-client.h"

#include "eca-object-factory.h"
#include "eca-chainsetup-position.h"
#include "sample-specs.h"

#include "eca-error.h"
#include "eca-debug.h"
#include "eca-chainsetup.h"

/**
 * Construct from a command line object. 
 * 
 * If any invalid options are passed us argument, 
 * interpret_result() will be 'true', and 
 * interpret_result_verbose() contains more detailed 
 * error description.
 *
 * @post buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(const std::vector<std::string>& opts) 
  : ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_default),
    cparser_rep(this) {

  setup_name_rep = "command-line-setup";
  setup_filename_rep = "";

  set_defaults();

  std::vector<std::string> options (opts);
  cparser_rep.preprocess_options(options);
  interpret_options(options);
  add_default_output();

  // --------
  DBC_ENSURE(buffersize() != 0);
  // --------
}

/**
 * Constructs an empty chainsetup.
 *
 * @post buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(void) 
  : ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_default),
    cparser_rep(this) {

  setup_name_rep = "";
  set_defaults();

  // --------
  DBC_ENSURE(buffersize() != 0);
  // --------
}

/**
 * Construct from a chainsetup file.
 * 
 * If any invalid options are passed us argument, 
 * interpret_result() will be 'true', and 
 * interpret_result_verbose() contains more detailed 
 * error description.
 *
 * @post buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(const std::string& setup_file) 
  : ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_default),
    cparser_rep(this) {

  setup_name_rep = "";
  set_defaults();
  std::vector<std::string> options;
  load_from_file(setup_file, options);
  if (setup_name_rep == "") setup_name_rep = setup_file;
  cparser_rep.preprocess_options(options);
  interpret_options(options);
  add_default_output();

  // --------
  DBC_ENSURE(buffersize() != 0);
  // --------
}

/**
 * Destructor
 */
ECA_CHAINSETUP::~ECA_CHAINSETUP(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects,"ECA_CHAINSETUP destructor!");

  for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Deleting chain \"" + (*q)->name() + "\".");
    delete *q;
    *q = 0;
  }
  
  for(std::vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
    if (dynamic_cast<LOOP_DEVICE*>(*q) == 0) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Deleting audio device/file \"" + (*q)->label() + "\".");
      delete *q;
      *q = 0;
    }
  }
  
  for(std::vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
    if (dynamic_cast<LOOP_DEVICE*>(*q) == 0) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Deleting audio device/file \"" + (*q)->label() + "\".");
      delete *q;
      *q = 0;
    }
  }

  for(std::map<int,LOOP_DEVICE*>::iterator q = loop_map.begin(); q != loop_map.end(); q++) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Deleting loop device \"" + q->second->label() + "\".");
    delete q->second;
    q->second = 0;
  }
}

/**
 * Tests whether chainsetup is in a valid state.
 */
bool ECA_CHAINSETUP::is_valid(void) const {
  // FIXME: waiting for a better implementation...
  return(is_valid_for_connection());
}

/**
 * Sets default values.
 *
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::set_defaults(void) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  double_buffering_rep = false;
  precise_sample_rates_rep = false;
  ignore_xruns_rep = true;
  max_buffers_rep = false;

  buffersize_rep = 0;
  proxy_clients_rep = 0;

  is_enabled_rep = false;
  mixmode_rep = ep_mm_auto;

  set_output_openmode(AUDIO_IO::io_readwrite);

  ECA_RESOURCES ecaresources;
  set_default_midi_device(ecaresources.resource("midi-device"));
  set_buffersize(atoi(ecaresources.resource("default-buffersize").c_str()));
  set_sample_rate(atol(ecaresources.resource("default-samplerate").c_str()));
  toggle_double_buffering(ecaresources.boolean_resource("default-to-double-buffering"));
  set_double_buffer_size(atol(ecaresources.resource("default-double-buffer-size").c_str()));
  toggle_precise_sample_rates(ecaresources.boolean_resource("default-to-precise-sample-rates"));
  toggle_max_buffers(ecaresources.boolean_resource("default-to-max-internal-buffering"));
}


/**
 * Checks whether chainsetup is valid for 
 * enabling/connecting.
 */
bool ECA_CHAINSETUP::is_valid_for_connection(void) const {
  if (inputs.size() == 0) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) No inputs in the current chainsetup.");
    return(false);
  }
  if (outputs.size() == 0) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) No outputs in the current chainsetup.");
    return(false);
  }
  if (chains.size() == 0) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) No chains in the current chainsetup.");
    return(false);
  }
  for(std::vector<CHAIN*>::const_iterator q = chains.begin(); q != chains.end();
      q++) {
    if ((*q)->is_valid() == false) return(false);
  }
  return(true);
}

/**
 * Adds a "default" chain to this chainsetup.
 *
 * @pre buffersize >= 0 && chains.size() == 0
 *
 * @post chains.back()->name() == "default" && 
 * @post active_chainids.back() == "default"
 */
void ECA_CHAINSETUP::add_default_chain(void) {
  // --------
  DBC_REQUIRE(buffersize() >= 0);
  DBC_REQUIRE(chains.size() == 0);
  // --------

  chains.push_back(new CHAIN());
  chains.back()->name("default");
  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-chainsetup) add_default_chain() ");
  selected_chainids.push_back("default");

  // --------
  DBC_ENSURE(chains.back()->name() == "default");
  DBC_ENSURE(selected_chainids.back() == "default");
  // --------  
}

/**
 * Adds new chains to this chainsetup.
 * 
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::add_new_chains(const std::vector<std::string>& newchains) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  for(std::vector<std::string>::const_iterator p = newchains.begin(); p != newchains.end(); p++) {
    bool exists = false;
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*p == (*q)->name()) exists = true;
    }
    if (exists == false) {
      chains.push_back(new CHAIN());
      chains.back()->name(*p);
      ecadebug->msg(ECA_DEBUG::system_objects,"add_new_chains() added chain " + *p);
    }
  }
}

/**
 * Removes all selected chains from this chainsetup.
 */
void ECA_CHAINSETUP::remove_chains(void) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  for(std::vector<std::string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	delete *q;
	chains.erase(q);
	break;
      }
    }
  }
  selected_chainids.resize(0);
}

/**
 * Clears all selected chains. Removes all chain operators
 * and controllers.
 *
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::clear_chains(void) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  for(std::vector<std::string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->clear();
      }
    }
  }
}

void ECA_CHAINSETUP::rename_chain(const std::string& name) {
  for(std::vector<std::string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->name(name);
	return;
      }
    }
  }
}

/**
 * Selects all chains present in this chainsetup.
 */
void ECA_CHAINSETUP::select_all_chains(void) {

  std::vector<CHAIN*>::const_iterator p = chains.begin();
  selected_chainids.resize(0);
  while(p != chains.end()) {
    selected_chainids.push_back((*p)->name());
    ++p;
  }
}

/**
 * Returns the index number of first selected chains. If no chains 
 * are selected, returns 'last_index + 1' (chains.size()).
 */
unsigned int ECA_CHAINSETUP::first_selected_chain(void) const {
  const std::vector<std::string>& schains = selected_chains();
  std::vector<std::string>::const_iterator o = schains.begin();
  unsigned int p = chains.size();
  while(o != schains.end()) {
    for(p = 0; p != chains.size(); p++) {
      if (chains[p]->name() == *o)
	return(p);
    }
    ++o;
  }
  return(p);
}

void ECA_CHAINSETUP::toggle_chain_muting(void) {
  for(std::vector<std::string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	if ((*q)->is_muted()) 
	  (*q)->toggle_muting(false);
	else 
	  (*q)->toggle_muting(true);
      }
    }
  }
}

void ECA_CHAINSETUP::toggle_chain_bypass(void) {
  for(std::vector<std::string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	if ((*q)->is_processing()) 
	  (*q)->toggle_processing(false);
	else 
	  (*q)->toggle_processing(true);
      }
    }
  }
}

std::vector<std::string> ECA_CHAINSETUP::chain_names(void) const {
  std::vector<std::string> result;
  std::vector<CHAIN*>::const_iterator p = chains.begin();
  while(p != chains.end()) {
    result.push_back((*p)->name());
    ++p;
  }
  return(result);
}

std::vector<std::string> ECA_CHAINSETUP::audio_input_names(void) const {
  std::vector<std::string> result;
  std::vector<AUDIO_IO*>::const_iterator p = inputs.begin();
  while(p != inputs.end()) {
    result.push_back((*p)->label());
    ++p;
  }
  return(result);
}

std::vector<std::string> ECA_CHAINSETUP::audio_output_names(void) const {
  std::vector<std::string> result;
  std::vector<AUDIO_IO*>::const_iterator p = outputs.begin();
  while(p != outputs.end()) {
    result.push_back((*p)->label());
    ++p;
  }
  return(result);
}



std::vector<std::string>
ECA_CHAINSETUP::get_attached_chains_to_input(AUDIO_IO* aiod) const{ 
  std::vector<std::string> res;
  
  std::vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->input_id_repp) {
      res.push_back((*q)->name());
    }
    ++q;
  }
  
  return(res); 
}

std::vector<std::string>
ECA_CHAINSETUP::get_attached_chains_to_output(AUDIO_IO* aiod) const { 
  std::vector<std::string> res;
  
  std::vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->output_id_repp) {
      res.push_back((*q)->name());
    }
    ++q;
  }

  return(res); 
}

int ECA_CHAINSETUP::number_of_attached_chains_to_input(AUDIO_IO* aiod) const {
  int count = 0;
  
  std::vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->input_id_repp) {
      ++count;
    }
    ++q;
  }

  return(count); 
}

int ECA_CHAINSETUP::number_of_attached_chains_to_output(AUDIO_IO* aiod) const {
  int count = 0;
  
  std::vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->output_id_repp) {
      ++count;
    }
    ++q;
  }

  return(count); 
}

/**
 * Slave output is a non-realtime output which is not 
 * connected to any realtime inputs.
 */
bool ECA_CHAINSETUP::is_slave_output(AUDIO_IO* aiod) const {
  AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>(aiod);
  if (p != 0) return(false);
  std::vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if ((*q)->output_id_repp == aiod) {
      p = dynamic_cast<AUDIO_IO_DEVICE*>((*q)->input_id_repp);
      if (p != 0) {
	ecadebug->msg(ECA_DEBUG::system_objects,"(eca-chainsetup) slave output detected: " + (*q)->output_id_repp->label());
	return(true);
      }
    }
    ++q;
  }
  return(false);
}

std::vector<std::string> ECA_CHAINSETUP::get_attached_chains_to_iodev(const
							     std::string&
							     filename) const
  {
  std::vector<AUDIO_IO*>::size_type p;

  p = 0;
  while (p < inputs.size()) {
    if (inputs[p]->label() == filename)
      return(get_attached_chains_to_input(inputs[p]));
    ++p;
  }

  p = 0;
  while (p < outputs.size()) {
    if (outputs[p]->label() == filename)
      return(get_attached_chains_to_output(outputs[p]));
    ++p;
  }
  return(std::vector<std::string> (0));
}

/**
 * Returns true if the connected chainsetup contains at least
 * one realtime audio input or output.
 */
bool ECA_CHAINSETUP::has_realtime_objects(void) const {
  for(size_t n = 0; n < inputs.size(); n++) {
    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>(inputs[n]);
    if (p != 0) return(true);
  }

  for(size_t n = 0; n < outputs.size(); n++) {
    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>(outputs[n]);
    if (p != 0) return(true);
  }

  return(false);
}

/** 
 * Helper function used by add_input() and add_output().
 */
AUDIO_IO* ECA_CHAINSETUP::add_audio_object_helper(AUDIO_IO* aio) {
  AUDIO_IO* retobj = aio;
  if (double_buffering() == true) {
    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>(aio);
    if (p == 0) {
      /* not a realtime device */
      retobj = new AUDIO_IO_BUFFERED_PROXY(&pserver_rep, aio);
      ++proxy_clients_rep;
    }
  }
  return(retobj);
}

/** 
 * Helper function used by add_output() and add_output().
 */
AUDIO_IO* ECA_CHAINSETUP::remove_audio_object_helper(AUDIO_IO* aio) {
  AUDIO_IO_BUFFERED_PROXY* p = dynamic_cast<AUDIO_IO_BUFFERED_PROXY*>(aio);
  if (p != 0) {
    /* a proxied object */
    --proxy_clients_rep;
  }
  return(aio);
}

/**
 * Adds a new input object and attaches it to selected chains.
 * 
 * If double-buffering is enabled (double_buffering() == true),
 * and the object in question is not a realtime object, it
 * is wrapped in a AUDIO_IO_BUFFERED_PROXY object before 
 * inserted to the chainsetup. Otherwise object is added
 * as is. 
 * 
 * Ownership of the insert object is transfered to 
 * ECA_CHAINSETUP.
 *
 * @pre aiod != 0
 * @pre chains.size() > 0 
 * @pre is_enabled() != true
 * @post inputs.size() == old(inputs.size() + 1
 */
void ECA_CHAINSETUP::add_input(AUDIO_IO* aio) {
  // --------
  DBC_REQUIRE(aio != 0);
  DBC_REQUIRE(chains.size() > 0);
  DBC_REQUIRE(is_enabled() != true);
  DBC_DECLARE(size_t old_inputs_size = inputs.size());
  // --------

  aio = add_audio_object_helper(aio);
  inputs.push_back(aio);
  input_start_pos.push_back(0);
  attach_input_to_selected_chains(aio);

  // --------
  DBC_ENSURE(inputs.size() == old_inputs_size + 1);
  // --------
}

/**
 * Add a new output object and attach it to selected chains.
 * 
 * If double-buffering is enabled (double_buffering() == true),
 * and the object in question is not a realtime object, it
 * is wrapped in a AUDIO_IO_BUFFERED_PROXY object before 
 * inserted to the chainsetup. Otherwise object is added
 * as is. 
 * 
 * Ownership of the insert object is transfered to 
 * ECA_CHAINSETUP.
 *
 * @pre aiod != 0
 * @pre chains.size() > 0
 * @pre is_enabled() != true
 * @post outputs.size() > 0
 */
void ECA_CHAINSETUP::add_output(AUDIO_IO* aio) {
  // --------
  DBC_REQUIRE(aio != 0);
  DBC_REQUIRE(is_enabled() != true);
  DBC_REQUIRE(chains.size() > 0);
  DBC_DECLARE(size_t old_outputs_size = outputs.size());
  // --------

  aio = add_audio_object_helper(aio);
  outputs.push_back(aio);
  output_start_pos.push_back(0);
  attach_output_to_selected_chains(aio);

  // --------
  DBC_ENSURE(outputs.size() == old_outputs_size + 1);
  // --------
}

/**
 * Removes the labeled audio input from this chainsetup.
 *
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::remove_audio_input(const std::string& label) { 
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  std::vector<AUDIO_IO*>::iterator ci = inputs.begin();
  while (ci != inputs.end()) {
    if ((*ci)->label() == label) {

      *ci = remove_audio_object_helper(*ci);

      std::vector<CHAIN*>::iterator q = chains.begin();
      while(q != chains.end()) {
	if ((*q)->input_id_repp == *ci) (*q)->disconnect_input();
	++q;
      }

      delete *ci;
      (*ci) = new NULLFILE("null");
      //      ecadebug->msg("(eca-chainsetup) Removing input " + label + ".");
    }
    ++ci;
  }
}

/**
 * Removes the labeled audio output from this chainsetup.
 *
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::remove_audio_output(const std::string& label) { 
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  std::vector<AUDIO_IO*>::iterator ci = outputs.begin();
  while (ci != outputs.end()) {
    if ((*ci)->label() == label) {

      *ci = remove_audio_object_helper(*ci);

      std::vector<CHAIN*>::iterator q = chains.begin();
      while(q != chains.end()) {
	if ((*q)->output_id_repp == *ci) (*q)->disconnect_output();
	++q;
      }

      delete *ci;
      (*ci) = new NULLFILE("null");
    }
    ++ci;
  }
}

/**
 * Print format and id information
 *
 * @pre aio != 0
 */
void ECA_CHAINSETUP::audio_object_info(const AUDIO_IO* aio) const {
  // --------
  DBC_REQUIRE(aio != 0);
  // --------

  string temp = "(eca-chainsetup) Audio object \"" + aio->label();
  temp += "\", mode \"";
  if (aio->io_mode() == AUDIO_IO::io_read) temp += "read";
  if (aio->io_mode() == AUDIO_IO::io_write) temp += "write";
  if (aio->io_mode() == AUDIO_IO::io_readwrite) temp += "read/write";
  temp += "\".\n";
  temp += aio->format_info();

  ecadebug->msg(temp);
}


/**
 * Adds a new MIDI-device object.
 *
 * @pre mididev != 0
 * @pre is_enabled() != true
 * @post midi_devices.size() > 0
 */
void ECA_CHAINSETUP::add_midi_device(MIDI_IO* mididev) {
  // --------
  DBC_REQUIRE(mididev != 0);
  DBC_REQUIRE(is_enabled() != true);
  // --------

  midi_devices.push_back(mididev);
  midi_server_rep.register_client(mididev);

  // --------
  DBC_ENSURE(midi_devices.size() > 0);
  // --------
}

/**
 * Remove an MIDI-device by the name 'mdev_name'.
 *
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::remove_midi_device(const std::string& mdev_name)
{
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  for(std::vector<MIDI_IO*>::iterator q = midi_devices.begin(); q != midi_devices.end(); q++) {
    if (mdev_name == (*q)->label()) {
      delete *q;
      midi_devices.erase(q);
      break;
    }
  }
}


const CHAIN* ECA_CHAINSETUP::get_chain_with_name(const std::string& name) const {
  std::vector<CHAIN*>::const_iterator p = chains.begin();
  while(p != chains.end()) {
    if ((*p)->name() == name) return(*p);
    ++p;
  }
  return(0);
}

void ECA_CHAINSETUP::attach_input_to_selected_chains(const AUDIO_IO* obj) {
    // --------
  DBC_REQUIRE(obj != 0);
  // --------
  string temp;
  std::vector<AUDIO_IO*>::size_type c = 0;

  while (c < inputs.size()) {
    if (inputs[c] == obj) {
      for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	if ((*q)->input_id_repp == inputs[c]) {
	  (*q)->disconnect_input();
	}
      }
      temp += "(eca-chainsetup) Assigning file to chains:";
      for(std::vector<std::string>::const_iterator p = selected_chainids.begin(); p!= selected_chainids.end(); p++) {
	for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	  if (*p == (*q)->name()) {
	    (*q)->connect_input(inputs[c]);
	    temp += " " + *p;
	  }
	}
      }
    }
    ++c;
  }
  ecadebug->msg(ECA_DEBUG::system_objects, temp);
}

void ECA_CHAINSETUP::attach_output_to_selected_chains(const AUDIO_IO* obj) {
    // --------
  DBC_REQUIRE(obj != 0);
  // --------

  string temp;
  std::vector<AUDIO_IO*>::size_type c = 0;
  while (c < outputs.size()) {
    if (outputs[c] == obj) {
      for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	if ((*q)->output_id_repp == outputs[c]) {
	  (*q)->disconnect_output();
	}
      }
      temp += "(eca-chainsetup) Assigning file to chains:";
      for(std::vector<std::string>::const_iterator p = selected_chainids.begin(); p!= selected_chainids.end(); p++) {
	for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	  if (*p == (*q)->name()) {
	    (*q)->connect_output(outputs[c]);
	    temp += " " + *p;
	  }
	}
      }
    }
    ++c;
  }
  ecadebug->msg(ECA_DEBUG::system_objects, temp);
}

/**
 * Enable chainsetup. Opens all devices and reinitializes all 
 * chain operators if necessary.
 *
 * This action is performed before connecting the chainsetup
 * to a engine object (for instance ECA_PROCESSOR). 
 * 
 * @post is_enabled() == true
 */
void ECA_CHAINSETUP::enable(void) throw(ECA_ERROR&) {
  try {
    if (is_enabled_rep == false) {
      ecadebug->control_flow("Chainsetup/Enabling audio inputs");
      for(std::vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
	(*q)->buffersize(buffersize(), sample_rate());
	AUDIO_IO_DEVICE* dev = dynamic_cast<AUDIO_IO_DEVICE*>(*q);
	if (dev != 0) {
	  dev->toggle_max_buffers(max_buffers());
	  dev->toggle_ignore_xruns(ignore_xruns());
	}
	if ((*q)->is_open() == false) (*q)->open();
	audio_object_info(*q);
      }
      
      ecadebug->control_flow("Chainsetup/Enabling audio outputs");
      for(std::vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
	(*q)->buffersize(buffersize(), sample_rate());
	AUDIO_IO_DEVICE* dev = dynamic_cast<AUDIO_IO_DEVICE*>(*q);
	if (dev != 0) {
	  dev->toggle_max_buffers(max_buffers());
	  dev->toggle_ignore_xruns(ignore_xruns());
	}
	if ((*q)->is_open() == false) (*q)->open();
	audio_object_info(*q);
      }

    if (midi_server_rep.is_enabled() != true &&
	midi_devices.size() > 0) midi_server_rep.enable();
      for(std::vector<MIDI_IO*>::iterator q = midi_devices.begin(); q != midi_devices.end(); q++) {
	(*q)->toggle_nonblocking_mode(true);
	if ((*q)->is_open() != true) {
	  (*q)->open();
	  if ((*q)->is_open() != true) {
	    throw(ECA_ERROR("ECA-CHAINSETUP", 
			    std::string("Unable to open MIDI-device: ") +
			    (*q)->label() +
			    "."));
	  }
	}
      }
    }
    is_enabled_rep = true;
  }
  catch(AUDIO_IO::SETUP_ERROR& e) {
    throw(ECA_ERROR("ECA-CHAINSETUP", 
		    std::string("Enabling chainsetup: ")
		    + e.message()));
  }
  catch(...) { throw; }

  // --------
  DBC_ENSURE(is_enabled() == true);
  // --------
}



/**
 * Disable chainsetup. Closes all devices. 
 * 
  * This action is performed before disconnecting the 
 * chainsetup from a engine object (for instance 
 * ECA_PROCESSOR). 
 * 
 * @post is_enabled() != true
 */
void ECA_CHAINSETUP::disable(void) {

  if (is_enabled_rep == true) {
    ecadebug->msg(ECA_DEBUG::system_objects, "Closing chainsetup \"" + name() + "\"");
    for(std::vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Closing audio device/file \"" + (*q)->label() + "\".");
      (*q)->close();
    }
    
    for(std::vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Closing audio device/file \"" + (*q)->label() + "\".");
      (*q)->close();
    }

    if (midi_server_rep.is_enabled() == true) midi_server_rep.disable();
    for(std::vector<MIDI_IO*>::iterator q = midi_devices.begin(); q != midi_devices.end(); q++) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Closing midi device \"" + (*q)->label() + "\".");
      if ((*q)->is_open() == true) (*q)->close();
    }

    is_enabled_rep = false;
  }

  // --------
  DBC_ENSURE(is_enabled() != true);
  // --------
}

/**
 * Changes the chainsetup position relatively to the current position. 
 */
void ECA_CHAINSETUP::change_position_exact(double seconds) { 
  ECA_CHAINSETUP_POSITION::change_position_exact(seconds); // change the global cs position

  std::vector<AUDIO_IO*>::iterator q = inputs.begin();
  while(q != inputs.end()) {
    (*q)->seek_position_in_seconds(seconds + (*q)->position_in_seconds_exact());
    ++q;
  }

  q = outputs.begin();
  while(q != outputs.end()) {
    (*q)->seek_position_in_seconds(seconds + (*q)->position_in_seconds_exact());
    ++q;
  }
}

/**
 * Sets the chainsetup position.
 */
void ECA_CHAINSETUP::set_position_exact(double seconds) {
  ECA_CHAINSETUP_POSITION::set_position_exact(seconds); // set the global cs position

  std::vector<AUDIO_IO*>::iterator q = inputs.begin();
  while(q != inputs.end()) {
    (*q)->seek_position_in_seconds(seconds);
    ++q;
  }

  q = outputs.begin();
  while(q != outputs.end()) {
    (*q)->seek_position_in_seconds(seconds);
    ++q;
  }
}


/**
 * Interprets one option. This is the most generic variant of
 * the interpretation routines; both global and object specific
 * options are handled.
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre is_enabled() != true
 * 
 * @post (option succesfully interpreted && interpret_result() ==  true) ||
 *       (unknown or invalid option && interpret_result() != true)
 */
void ECA_CHAINSETUP::interpret_option (const std::string& arg) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  cparser_rep.interpret_option(arg);
}

/**
 * Interprets one option. All non-global options are ignored. Global
 * options can be interpreted multiple times and in any order.
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre is_enabled() != true
 * @post (option succesfully interpreted && interpretation_result() ==  true) ||
 *       (unknown or invalid option && interpretation_result() == false)
 */
void ECA_CHAINSETUP::interpret_global_option (const std::string& arg) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  cparser_rep.interpret_global_option(arg);
}

/**
 * Interprets one option. All options not directly related to 
 * ecasound objects are ignored.
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre is_enabled() != true
 * 
 * @post (option succesfully interpreted && interpretation_result() ==  true) ||
 *       (unknown or invalid option && interpretation_result() == false)
 */
void ECA_CHAINSETUP::interpret_object_option (const std::string& arg) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  cparser_rep.interpret_object_option(arg);
}

/**
 * Interpret a vector of options.
 *
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::interpret_options(vector<string>& opts) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  cparser_rep.interpret_options(opts);
}


/**
 * Select controllers as targets for parameter control
 */
void ECA_CHAINSETUP::set_target_to_controller(void) {
  std::vector<std::string> schains = selected_chains();
  for(std::vector<std::string>::const_iterator a = schains.begin(); a != schains.end(); a++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->selected_controller_as_target();
	return;
      }
    }
  }
}

/**
 * Add general controller to selected chainop.
 *
 * @pre  csrc != 0
 * @pre is_enabled() != true
 * @pre selected_chains().size() == 1
 */
void ECA_CHAINSETUP::add_controller(GENERIC_CONTROLLER* csrc) {
  // --------
  DBC_REQUIRE(csrc != 0);
  DBC_REQUIRE(is_enabled() != true);
  // --------

  AUDIO_STAMP_CLIENT* p = dynamic_cast<AUDIO_STAMP_CLIENT*>(csrc->source_pointer());
  if (p != 0) {
//      cerr << "Found a stamp client!" << endl;
    p->register_server(&stamp_server_rep);
  }

  csrc->init((double)buffersize() / sample_rate());

  std::vector<std::string> schains = selected_chains();
  for(std::vector<std::string>::const_iterator a = schains.begin(); a != schains.end(); a++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	if ((*q)->selected_target() == 0) return;
	(*q)->add_controller(csrc);
	return;
      }
    }
  }
}

/**
 * Add chain operator to selected chain.
 *
 * @pre cotmp != 0
 * @pre is_enabled() != true
 * @pre selected_chains().size() == 1
 */
void ECA_CHAINSETUP::add_chain_operator(CHAIN_OPERATOR* cotmp) {
  // --------
  DBC_REQUIRE(cotmp != 0);
  DBC_REQUIRE(is_enabled() != true);
  // --------
  
  AUDIO_STAMP* p = dynamic_cast<AUDIO_STAMP*>(cotmp);
  if (p != 0) {
    stamp_server_rep.register_stamp(p);
  }
  std::vector<std::string> schains = selected_chains();
  for(std::vector<std::string>::const_iterator p = schains.begin(); p != schains.end(); p++) {
    for(std::vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*p == (*q)->name()) {
	ecadebug->msg(ECA_DEBUG::system_objects, "Adding chainop to chain " + (*q)->name() + ".");
	(*q)->add_chain_operator(cotmp);
	(*q)->selected_chain_operator_as_target();
	return;
	//	(*q)->add_chain_operator(cotmp->clone());
      }
    }
  }
}

/**
 * If chainsetup has inputs, but no outputs, a default output is
 * added.
 * 
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::add_default_output(void) {
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  if (inputs.size() > 0 && outputs.size() == 0) {
    // No -o[:] options specified; let's use the default output
    select_all_chains();
    ECA_RESOURCES ecaresources;
    interpret_object_option(string("-o:" + ecaresources.resource("default-output")));
  }
}

/**
 * Loads chainsetup options from file.
 *
 * @pre is_enabled() != true
 */
void ECA_CHAINSETUP::load_from_file(const std::string& filename,
				    std::vector<std::string>& opts) const throw(ECA_ERROR&) { 
  // --------
  DBC_REQUIRE(is_enabled() != true);
  // --------

  std::ifstream fin (filename.c_str());
  if (!fin) throw(ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup read file: \"" + filename + "\".", ECA_ERROR::retry));

  std::vector<std::string> options;
  std::string temp;
  while(getline(fin,temp)) {
    if (temp.size() > 0 && temp[0] == '#') {
      continue;
    }
    std::vector<std::string> words = string_to_words(temp);
    for(unsigned int n = 0; n < words.size(); n++) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Adding \"" + words[n] + "\" to options (loaded from \"" + filename + "\".");
      options.push_back(words[n]);
    }
  }
  fin.close();

  opts = COMMAND_LINE::combine(options);
}

void ECA_CHAINSETUP::save(void) throw(ECA_ERROR&) { 
  if (setup_filename_rep.empty() == true)
    setup_filename_rep = setup_name_rep + ".ecs";
  save_to_file(setup_filename_rep);
}

void ECA_CHAINSETUP::save_to_file(const std::string& filename) throw(ECA_ERROR&) {
  std::ofstream fout (filename.c_str());
  if (!fout) {
    std::cerr << "Going to throw an exception...\n";
    throw(ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup save file: \"" +
  			filename + "\".", ECA_ERROR::retry));
  }
  else {
    fout << "# ecasound chainsetup file" << endl;
    fout << endl;

    fout << "# general " << endl;
    fout << cparser_rep.general_options_to_string() << endl;
    fout << endl;

    string tmpstr = cparser_rep.midi_to_string();
    if (tmpstr.size() > 0) {
      fout << "# MIDI " << endl;
      fout << tmpstr << endl;
      fout << endl;      
    }

    fout << "# audio inputs " << endl;
    fout << cparser_rep.inputs_to_string() << endl;
    fout << endl;

    fout << "# audio outputs " << endl;
    fout << cparser_rep.outputs_to_string() << endl;
    fout << endl;

    tmpstr = cparser_rep.chains_to_string();
    if (tmpstr.size() > 0) {
      fout << "# chain operators and controllers " << endl;
      fout << tmpstr << endl;
      fout << endl;      
    }

    fout.close();
    set_filename(filename);
  }
}
