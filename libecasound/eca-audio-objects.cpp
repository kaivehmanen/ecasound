// ------------------------------------------------------------------------
// eca-audio-objects.cpp: A specialized container class for
//                        representing a group of inputs, outputs and
//                        chains. Not meant for general use.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <vector>
#include <map>

#include <kvutils/message_item.h>

#include "audioio.h"
#include "audioio-loop.h"
#include "audioio-null.h"
#include "midiio.h"
#include "eca-static-object-maps.h"

#include "eca-chain.h"
#include "samplebuffer.h"
#include "eca-resources.h"

#include "eca-debug.h"
#include "eca-audio-objects.h"

/**
 * Constructor
 */
ECA_AUDIO_OBJECTS::ECA_AUDIO_OBJECTS(void) 
  : 
    double_buffering_rep (false),
    precise_sample_rates_rep (false),
    ignore_xruns_rep(true),
    max_buffers_rep(true),
    output_openmode_rep (AUDIO_IO::io_readwrite),
    buffersize_rep(0),
    selected_chainids (0) { }

/**
 * Destructor
 */
ECA_AUDIO_OBJECTS::~ECA_AUDIO_OBJECTS(void) {
  ecadebug->msg(ECA_DEBUG::system_objects,"ECA_AUDIO_OBJECTS destructor!");

  for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting chain \"" + (*q)->name() + "\".");
    delete *q;
    *q = 0;
  }
  
  for(vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
    if (dynamic_cast<LOOP_DEVICE*>(*q) == 0) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting audio device/file \"" + (*q)->label() + "\".");
      delete *q;
      *q = 0;
    }
  }
  //  inputs.resize(0);
  
  for(vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
    if (dynamic_cast<LOOP_DEVICE*>(*q) == 0) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting audio device/file \"" + (*q)->label() + "\".");
      delete *q;
      *q = 0;
    }
  }
  //  outputs.resize(0);

  for(map<int,LOOP_DEVICE*>::iterator q = loop_map.begin(); q != loop_map.end(); q++) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting loop device \"" + q->second->label() + "\".");
    delete q->second;
    q->second = 0;
  }
}

bool ECA_AUDIO_OBJECTS::is_valid(void) const {
  if (inputs.size() == 0) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) No inputs in the current chainsetup.");
    return(false);
  }
  if (outputs.size() == 0) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) No outputs in the current chainsetup.");
    return(false);
  }
  if (chains.size() == 0) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) No chains in the current chainsetup.");
    return(false);
  }
  for(vector<CHAIN*>::const_iterator q = chains.begin(); q != chains.end();
      q++) {
    if ((*q)->is_valid() == false) return(false);
  }
  return(true);
}

/**
 * Create a new loop input object
 *
 * require:
 *  argu.empty() != true
 */
AUDIO_IO* ECA_AUDIO_OBJECTS::create_loop_input(const string& argu) {
  // --------
  REQUIRE(argu.empty() != true);
  // --------

  LOOP_DEVICE* p = 0;
  string tname = get_argument_number(1, argu);
  if (tname.find("loop") != string::npos) {
    int id = atoi(get_argument_number(2, argu).c_str());
    p = new LOOP_DEVICE(id);
    if (loop_map.find(id) == loop_map.end()) { 
      loop_map[id] = p;
    }
    else
      p = loop_map[id];

    p->register_input();
  }
  
  return(p);
}

/**
 * Create a new loop output object
 *
 * require:
 *  argu.empty() != true
 */
AUDIO_IO* ECA_AUDIO_OBJECTS::create_loop_output(const string& argu) {
  // --------
  REQUIRE(argu.empty() != true);
  // --------

  LOOP_DEVICE* p = 0;
  string tname = get_argument_number(1, argu);
  if (tname.find("loop") != string::npos) {
    int id = atoi(get_argument_number(2, argu).c_str());
    p = new LOOP_DEVICE(id);
    if (loop_map.find(id) == loop_map.end()) { 
      loop_map[id] = p;
    }
    else
      p = loop_map[id];

    p->register_output();
  }
  
  return(p);
}

/**
 * Adds a "default" chain to the setup.
 *
 * require:
 *   buffersize >= 0 && chains.size() == 0
 *
 * ensure:
 *   chains.back()->name() == "default" && 
 *   active_chainids.back() == "default"
 */
void ECA_AUDIO_OBJECTS::add_default_chain(void) {
  // --------
  REQUIRE(buffersize() >= 0);
  REQUIRE(chains.size() == 0);
  // --------

  chains.push_back(new CHAIN());
  chains.back()->name("default");
  ecadebug->msg(ECA_DEBUG::system_objects,"add_default_chain() ");
  selected_chainids.push_back("default");

  // --------
  ENSURE(chains.back()->name() == "default");
  ENSURE(selected_chainids.back() == "default");
  // --------  
}

void ECA_AUDIO_OBJECTS::add_new_chains(const vector<string>& newchains) {
  for(vector<string>::const_iterator p = newchains.begin(); p != newchains.end(); p++) {
    bool exists = false;
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*p == (*q)->name()) exists = true;
    }
    if (exists == false) {
      chains.push_back(new CHAIN());
      chains.back()->name(*p);
      ecadebug->msg(ECA_DEBUG::system_objects,"add_new_chains() added chain " + *p);
    }
  }
}

void ECA_AUDIO_OBJECTS::remove_chains(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	delete *q;
	chains.erase(q);
	break;
      }
    }
  }
  selected_chainids.resize(0);
}

void ECA_AUDIO_OBJECTS::clear_chains(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->clear();
      }
    }
  }
}

void ECA_AUDIO_OBJECTS::rename_chain(const string& name) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->name(name);
	return;
      }
    }
  }
}

void ECA_AUDIO_OBJECTS::select_all_chains(void) {
  vector<CHAIN*>::const_iterator p = chains.begin();
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
unsigned int ECA_AUDIO_OBJECTS::first_selected_chain(void) const {
  const vector<string>& schains = selected_chains();
  vector<string>::const_iterator o = schains.begin();
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

void ECA_AUDIO_OBJECTS::toggle_chain_muting(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	if ((*q)->is_muted()) 
	  (*q)->toggle_muting(false);
	else 
	  (*q)->toggle_muting(true);
      }
    }
  }
}

void ECA_AUDIO_OBJECTS::toggle_chain_bypass(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	if ((*q)->is_processing()) 
	  (*q)->toggle_processing(false);
	else 
	  (*q)->toggle_processing(true);
      }
    }
  }
}

vector<string> ECA_AUDIO_OBJECTS::chain_names(void) const {
  vector<string> result;
  vector<CHAIN*>::const_iterator p = chains.begin();
  while(p != chains.end()) {
    result.push_back((*p)->name());
    ++p;
  }
  return(result);
}

vector<string>
ECA_AUDIO_OBJECTS::get_attached_chains_to_input(AUDIO_IO* aiod) const{ 
  vector<string> res;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->input_id_repp) {
      res.push_back((*q)->name());
    }
    ++q;
  }
  
  return(res); 
}

vector<string>
ECA_AUDIO_OBJECTS::get_attached_chains_to_output(AUDIO_IO* aiod) const { 
  vector<string> res;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->output_id_repp) {
      res.push_back((*q)->name());
    }
    ++q;
  }

  return(res); 
}

int ECA_AUDIO_OBJECTS::number_of_attached_chains_to_input(AUDIO_IO* aiod) const {
  int count = 0;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->input_id_repp) {
      ++count;
    }
    ++q;
  }

  return(count); 
}

int ECA_AUDIO_OBJECTS::number_of_attached_chains_to_output(AUDIO_IO* aiod) const {
  int count = 0;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->output_id_repp) {
      ++count;
    }
    ++q;
  }

  return(count); 
}

vector<string> ECA_AUDIO_OBJECTS::get_attached_chains_to_iodev(const
							     string&
							     filename) const
  {
  vector<AUDIO_IO*>::size_type p;

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
  return(vector<string> (0));
}

/**
 * Add a new input object and attach it to selected chains.
 *
 * require:
 *   aiod != 0 && chains.size() > 0
 *
 * ensure:
 *   inputs.size() > 0
 */
void ECA_AUDIO_OBJECTS::add_input(AUDIO_IO* aio) {
  // --------
  REQUIRE(aio != 0);
  REQUIRE(chains.size() > 0);
  // --------

  inputs.push_back(aio);
  input_start_pos.push_back(0);
  attach_input_to_selected_chains(aio);

  // --------
  ENSURE(inputs.size() > 0);
  // --------
}

/**
 * Add a new output object and attach it to selected chains.
 *
 * require:
 *   aiod != 0 && chains.size() > 0
 *
 * ensure:
 *   outputs.size() > 0
 */
void ECA_AUDIO_OBJECTS::add_output(AUDIO_IO* aiod) {
  // --------
  REQUIRE(aiod != 0);
  REQUIRE(chains.size() > 0);
  // --------

  outputs.push_back(aiod);
  output_start_pos.push_back(0);
  attach_output_to_selected_chains(aiod);

  // --------
  ENSURE(outputs.size() > 0);
  // --------
}

/**
 * Print format and id information
 *
 * require:
 *   aio != 0
 */
void ECA_AUDIO_OBJECTS::audio_object_info(const AUDIO_IO* aio) const {
  // --------
  REQUIRE(aio != 0);
  // --------

  string temp = "(eca-audio-objects) Audio object \"" + aio->label();
  temp += "\", mode \"";
  if (aio->io_mode() == AUDIO_IO::io_read) temp += "read";
  if (aio->io_mode() == AUDIO_IO::io_write) temp += "write";
  if (aio->io_mode() == AUDIO_IO::io_readwrite) temp += "read/write";
  temp += "\".\n";
  temp += aio->format_info();

  ecadebug->msg(temp);
}

void ECA_AUDIO_OBJECTS::remove_audio_input(const string& label) { 
  vector<AUDIO_IO*>::iterator ci = inputs.begin();
  while (ci != inputs.end()) {
    if ((*ci)->label() == label) {
      vector<CHAIN*>::iterator q = chains.begin();
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

void ECA_AUDIO_OBJECTS::remove_audio_output(const string& label) { 
  vector<AUDIO_IO*>::iterator ci = outputs.begin();
  while (ci != outputs.end()) {
    if ((*ci)->label() == label) {
      vector<CHAIN*>::iterator q = chains.begin();
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
 * Add a new MIDI-device object.
 *
 * require:
 *   mididev != 0
 *
 * ensure:
 *   midi_devices.size() > 0
 */
void ECA_AUDIO_OBJECTS::add_midi_device(MIDI_IO* mididev) {
  // --------
  REQUIRE(mididev != 0);
  // --------

  midi_devices.push_back(mididev);
  midi_server_rep.register_client(mididev);

  // --------
  ENSURE(midi_devices.size() > 0);
  // --------
}

/**
 * Remove an MIDI-device by the name 'mdev_name'.
 */
void ECA_AUDIO_OBJECTS::remove_midi_device(const string& mdev_name) {
  for(vector<MIDI_IO*>::iterator q = midi_devices.begin(); q != midi_devices.end(); q++) {
    if (mdev_name == (*q)->label()) {
      delete *q;
      midi_devices.erase(q);
      break;
    }
  }
}

string ECA_AUDIO_OBJECTS::midi_to_string(void) const { 
  MESSAGE_ITEM t; 
  t.setprecision(3);

  // FIXME: don't use name() fro saving, but use a set of 
  //        set_/get_param calls like with audioio objs
  vector<MIDI_IO*>::size_type p = 0;
  while (p < midi_devices.size()) {
    t << "-Md:" << midi_devices[p]->name() << " ";
    ++p;
    if (p < chains.size()) t << "\n";
  }

  return(t.to_string());
}

string ECA_AUDIO_OBJECTS::inputs_to_string(void) const { 
  MESSAGE_ITEM t; 
  t.setprecision(3);
  int p = 0;
  while (p < static_cast<int>(inputs.size())) {
    t << "-a:";
    vector<string> c = get_attached_chains_to_input(inputs[p]);
    vector<string>::const_iterator cp = c.begin();
    while (cp != c.end()) {
      t << *cp;
      ++cp;
      if (cp != c.end()) t << ",";
    }
    t << " ";
    t << audioio_to_string(inputs[p], "i");

    if (input_start_pos[p] != 0) {
      t << " -y:" << input_start_pos[p];
    }

    t << "\n";
    ++p;
  }

  return(t.to_string());
}

string ECA_AUDIO_OBJECTS::outputs_to_string(void) const { 
  MESSAGE_ITEM t; 
  t.setprecision(3);
  vector<AUDIO_IO*>::size_type p = 0;
  while (p < outputs.size()) {
    t << "-a:";
    vector<string> c = get_attached_chains_to_output(outputs[p]);
    vector<string>::const_iterator cp = c.begin();
    while (cp != c.end()) {
      t << *cp;
      ++cp;
      if (cp != c.end()) t << ",";
    }
    t << " ";
    t << audioio_to_string(outputs[p], "o");

    if (output_start_pos[p] != 0) {
      t << " -y:" << output_start_pos[p];
    }

    t << "\n";
    ++p;

//    if (startpos_map.find(id) != startpos_map.end()) {
//      
//    }
  }

  return(t.to_string());
}

string ECA_AUDIO_OBJECTS::chains_to_string(void) const { 
  MESSAGE_ITEM t;

  vector<CHAIN*>::size_type p = 0;
  while (p < chains.size()) {
    t << "-a:" << chains[p]->name() << " ";
    t << chains[p]->to_string();
    ++p;
    if (p < chains.size()) t << "\n";
  }

  return(t.to_string());
}

string ECA_AUDIO_OBJECTS::audioio_to_string(const AUDIO_IO* aiod, const string& direction) const {
  MESSAGE_ITEM t;

  t << "-f:" << aiod->format_string() << "," <<
    aiod->channels() << ","  << aiod->samples_per_second();
  t << " -" << direction << ":";
  for(int n = 0; n < aiod->number_of_params(); n++) {
    t << aiod->get_parameter(n + 1);
    if (n + 1 < aiod->number_of_params()) t << ",";
  }

  return(t.to_string());
}

const CHAIN* ECA_AUDIO_OBJECTS::get_chain_with_name(const string& name) const {
  vector<CHAIN*>::const_iterator p = chains.begin();
  while(p != chains.end()) {
    if ((*p)->name() == name) return(*p);
    ++p;
  }
  return(0);
}

void ECA_AUDIO_OBJECTS::attach_input_to_selected_chains(const AUDIO_IO* obj) {
    // --------
  REQUIRE(obj != 0);
  // --------
  string temp;
  vector<AUDIO_IO*>::size_type c = 0;

  while (c < inputs.size()) {
    if (inputs[c] == obj) {
      for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	if ((*q)->input_id_repp == inputs[c]) {
	  (*q)->disconnect_input();
	}
      }
      temp += "(eca-audio-objects) Assigning file to chains:";
      for(vector<string>::const_iterator p = selected_chainids.begin(); p!= selected_chainids.end(); p++) {
	for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
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

void ECA_AUDIO_OBJECTS::attach_output_to_selected_chains(const AUDIO_IO* obj) {
    // --------
  REQUIRE(obj != 0);
  // --------

  string temp;
  vector<AUDIO_IO*>::size_type c = 0;
  while (c < outputs.size()) {
    if (outputs[c] == obj) {
      for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	if ((*q)->output_id_repp == outputs[c]) {
	  (*q)->disconnect_output();
	}
      }
      temp += "(eca-chainsetup) Assigning file to chains:";
      for(vector<string>::const_iterator p = selected_chainids.begin(); p!= selected_chainids.end(); p++) {
	for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
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
