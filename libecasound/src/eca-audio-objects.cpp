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

#include <config.h>

#include <string>
#include <vector>
#include <map>

#include <kvutils/message_item.h>

#include "audioio.h"
#include "audioio-loop.h"
#include "audioio-null.h"
#include "eca-static-object-maps.h"
#include "eca-audio-object-map.h"

#include "eca-chain.h"
#include "samplebuffer.h"

#include "eca-debug.h"
#include "eca-audio-objects.h"

ECA_AUDIO_OBJECTS::ECA_AUDIO_OBJECTS(void) 
  : 
    double_buffering_rep (false),
    precise_sample_rates_rep (false),
    output_openmode_rep (AUDIO_IO::io_readwrite),
    buffersize_rep(0),
    last_audio_object(0),
    selected_chainids (0) { }

ECA_AUDIO_OBJECTS::~ECA_AUDIO_OBJECTS(void) {
  ecadebug->msg(ECA_DEBUG::system_objects,"ECA_AUDIO_OBJECTS destructor!");

  for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting chain \"" + (*q)->name() + "\".");
    delete *q;
  }
  
  for(vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
    if (dynamic_cast<LOOP_DEVICE*>(*q) == 0) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting audio device/file \"" + (*q)->label() + "\".");
      delete *q;
    }
  }
  //  inputs.resize(0);
  
  for(vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
    if (dynamic_cast<LOOP_DEVICE*>(*q) == 0) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting audio device/file \"" + (*q)->label() + "\".");
      delete *q;
    }
  }
  //  outputs.resize(0);

  for(map<int,LOOP_DEVICE*>::iterator q = loop_map.begin(); q != loop_map.end(); q++) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-audio-objects) Deleting loop device \"" + q->second->label() + "\".");
    delete q->second;
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

void ECA_AUDIO_OBJECTS::interpret_audioio_device (const string& argu) throw(ECA_ERROR*) { 
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------
 
  string tname = get_argument_number(1, argu);
  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-audio-objects) adding file \"" + tname + "\".");

  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 'i':
    {
      ecadebug->msg(ECA_DEBUG::system_objects, "Eca-audio-objects/Parsing input");
      last_audio_object = create_audio_object(argu);
      if (last_audio_object == 0) 
	last_audio_object = create_loop_input(argu);
      if (last_audio_object != 0) {
	if ((last_audio_object->supported_io_modes() &
	    AUDIO_IO::io_read) != AUDIO_IO::io_read) {
	  throw(new ECA_ERROR("ECA-AUDIO-OBJECTS", "I/O-mode 'io_read' not supported by " + last_audio_object->name()));
	}
	last_audio_object->io_mode(AUDIO_IO::io_read);
	last_audio_object->set_audio_format(default_audio_format_rep);
	add_input(last_audio_object);
      }
      else {
	throw(new ECA_ERROR("ECA-AUDIO-OBJECTS", "Format of input " +
			    tname + " not recognized."));
      }
      break;
    }

  case 'o':
    {
      ecadebug->msg(ECA_DEBUG::system_objects, "Eca-audio-objects/Parsing output");
      last_audio_object = create_audio_object(argu);
      if (last_audio_object == 0) last_audio_object = create_loop_output(argu);
      if (last_audio_object != 0) {
	int mode_tmp = output_openmode_rep;
	if (mode_tmp == AUDIO_IO::io_readwrite) {
	  if ((last_audio_object->supported_io_modes() &
	      AUDIO_IO::io_readwrite) != AUDIO_IO::io_readwrite) {
	    mode_tmp = AUDIO_IO::io_write;
	  }
	}
	if ((last_audio_object->supported_io_modes() &
	    mode_tmp != mode_tmp)) {
	    throw(new ECA_ERROR("ECA-AUDIO-OBJECTS", "I/O-mode 'io_write' not supported by " + last_audio_object->name()));
	}
      
	last_audio_object->io_mode(mode_tmp);
	last_audio_object->set_audio_format(default_audio_format_rep);
	add_output(last_audio_object);
      }
      else {
	throw(new ECA_ERROR("ECA-AUDIO-OBJECTS", "Format of output " +
			    tname + " not recognized."));
      }
      break;
    }

  case 'y':
    {
      if (last_audio_object == 0)
	ecadebug->msg("Error! No audio object defined.");

      last_audio_object->seek_position_in_seconds(atof(get_argument_number(1, argu).c_str()));
      if (last_audio_object->io_mode() == AUDIO_IO::io_read) {
	input_start_pos[input_start_pos.size() - 1] = last_audio_object->position_in_seconds_exact();
      }
      else {
	output_start_pos[output_start_pos.size() - 1] = last_audio_object->position_in_seconds_exact();
      }

      ecadebug->msg("(eca-audio-objects) Set starting position for audio object \""
		    + last_audio_object->label() 
		    + "\": "
		    + kvu_numtostr(last_audio_object->position_in_seconds_exact()) 
		    + " seconds.");
      break;
    }

  default: { }
  }
}

AUDIO_IO* ECA_AUDIO_OBJECTS::create_audio_object(const string& argu) {
  assert(argu.empty() != true);
 
  register_default_objects();
  string tname = get_argument_number(1, argu);

  AUDIO_IO* main_file = 0;
  main_file = ECA_AUDIO_OBJECT_MAP::object(tname);

  if (main_file != 0) {
    main_file = main_file->new_expr();
    main_file->map_parameters();
    ecadebug->msg(ECA_DEBUG::system_objects, "(e-a-o) number of params: " + main_file->name() + "=" + 
		  kvu_numtostr(main_file->number_of_params()));
    for(int n = 0; n < main_file->number_of_params(); n++) {
      main_file->set_parameter(n + 1, get_argument_number(n + 1, argu));
    }
  }
  return(main_file);
}

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

vector<string>
ECA_AUDIO_OBJECTS::get_attached_chains_to_input(AUDIO_IO* aiod) const{ 
  vector<string> res;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->input_id) {
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
    if (aiod == (*q)->output_id) {
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
    if (aiod == (*q)->input_id) {
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
    if (aiod == (*q)->output_id) {
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


void ECA_AUDIO_OBJECTS::add_input(AUDIO_IO* aio) {
  // --------
  REQUIRE(aio != 0);
  REQUIRE(chains.size() > 0);
  // --------

  inputs.push_back(aio);
  input_start_pos.push_back(0);
  attach_input_to_selected_chains(aio->label());

  // --------
  ENSURE(inputs.size() > 0);
  // --------
}

void ECA_AUDIO_OBJECTS::add_output(AUDIO_IO* aiod) {
  // --------
  REQUIRE(aiod != 0);
  REQUIRE(chains.size() > 0);
  // --------

  outputs.push_back(aiod);
  output_start_pos.push_back(0);
  attach_output_to_selected_chains(aiod->label());

  // --------
  ENSURE(outputs.size() > 0);
  // --------
}

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
	if ((*q)->input_id == *ci) (*q)->disconnect_input();
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
	if ((*q)->output_id == *ci) (*q)->disconnect_output();
	++q;
      }
      delete *ci;
      (*ci) = new NULLFILE("null");
    }
    ++ci;
  }
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

void ECA_AUDIO_OBJECTS::attach_input_to_selected_chains(const string& filename) {
  string temp;
  vector<AUDIO_IO*>::size_type c = 0;

  while (c < inputs.size()) {
    if (inputs[c]->label() == filename) {
      for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	if ((*q)->input_id == inputs[c]) {
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

void ECA_AUDIO_OBJECTS::attach_output_to_selected_chains(const string& filename) {
  string temp;
  vector<AUDIO_IO*>::size_type c = 0;

  while (c < outputs.size()) {
    if (outputs[c]->label() == filename) {
      for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	if ((*q)->output_id == outputs[c]) {
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
