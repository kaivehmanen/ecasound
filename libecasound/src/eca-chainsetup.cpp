// ------------------------------------------------------------------------
// eca-chainsetup.cpp: A class representing a group of chains and their
//                     configuration.
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
#include <cstring>
#include <algorithm>
#include <fstream>
#include <vector>

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "eca-resources.h"
#include "eca-session.h"

#include "generic-controller.h"
#include "eca-chainop.h"
#include "file-preset.h"
#include "global-preset.h"

#include "eca-static-object-maps.h"
#include "eca-ladspa-plugin-map.h"
#include "eca-chainop-map.h"
#include "eca-controller-map.h"

#include "eca-control-position.h"
#include "eca-audio-objects.h"

#include "eca-error.h"
#include "eca-debug.h"
#include "eca-chainsetup.h"

ECA_CHAINSETUP::ECA_CHAINSETUP(COMMAND_LINE& cline) 
  : ECA_CONTROL_POSITION(SAMPLE_SPECS::sample_rate_default) {

  setup_name = "command-line-setup";
  setup_filename = "";

  set_defaults();

  string temp;
  cline.begin();
  cline.next(); // skip the program name
  while(cline.end() == false) {
    temp = cline.current();
    if (temp != "") {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Adding \"" + temp + "\" to options.");
      options.push_back(temp);
    }
    cline.next();
  }

  interpret_options(options);

  // --------
  ENSURE(buffersize() != 0);
  // --------
}

ECA_CHAINSETUP::ECA_CHAINSETUP(const string& setup_file, bool fromfile) 
  : ECA_CONTROL_POSITION(SAMPLE_SPECS::sample_rate_default) {

  setup_name = "";

  set_defaults();

  if (fromfile) load_from_file(setup_file);
  if (setup_name == "") setup_name = setup_file;

  interpret_options(options);

  // --------
  ENSURE(buffersize() != 0);
  // --------
}


void ECA_CHAINSETUP::set_defaults(void) {
  register_default_objects();

  is_enabled_rep = false;
  mixmode_rep = ep_mm_auto;

  set_output_openmode(AUDIO_IO::io_readwrite);

  ECA_RESOURCES ecaresources;
  set_buffersize(atoi(ecaresources.resource("default-buffersize").c_str()));

  set_sample_rate(atol(ecaresources.resource("default-samplerate").c_str()));
  
  toggle_double_buffering(ecaresources.boolean_resource("default-to-double-buffering"));
  toggle_precise_sample_rates(ecaresources.boolean_resource("default-to-precise-sample-rates"));
}

ECA_CHAINSETUP::~ECA_CHAINSETUP(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects,"ECA_CHAINSETUP destructor!");
}

void ECA_CHAINSETUP::enable(void) {
  if (is_enabled_rep == false) {
    ecadebug->control_flow("Chainsetup/Enabling audio inputs");
    for(vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
      (*q)->buffersize(buffersize(), sample_rate());
      if ((*q)->is_open() == false) (*q)->open();
      audio_object_info(*q);
    }
    
    ecadebug->control_flow("Chainsetup/Enabling audio outputs");
    for(vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
      (*q)->buffersize(buffersize(), sample_rate());
      if ((*q)->is_open() == false) (*q)->open();
      audio_object_info(*q);      
    }

//      for(vector<CHAIN*>::const_iterator q = chains.begin(); q != 
//  	  chains.end(); q++) {     }
  }
  is_enabled_rep = true;

  // --------
  ENSURE(is_enabled() == true);
  // --------
}

void ECA_CHAINSETUP::disable(void) {
  update_option_strings();
  if (is_enabled_rep == true) {
    ecadebug->msg(ECA_DEBUG::system_objects, "Closing chainsetup \"" + name() + "\"");
    for(vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Closing audio device/file \"" + (*q)->label() + "\".");
      (*q)->close();
    }
    
    for(vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Closing audio device/file \"" + (*q)->label() + "\".");
      (*q)->close();
    }

    is_enabled_rep = false;
  }

  // --------
  ENSURE(is_enabled() == false);
  // --------
}

void ECA_CHAINSETUP::interpret_options(vector<string>& opts) {
  vector<string>::iterator p = opts.begin();
  while(p != opts.end()) {
    if (p->size() > 0 && (*p)[0] != '-') *p = "-i:" + *p;
    ++p;
  }

  p = opts.begin();
  while(p != opts.end()) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting general option \""
		  + *p + "\".");
    interpret_general_option(*p);
    interpret_processing_control(*p);
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting chain option \""
		  + *p + "\".");
    interpret_chains(*p);
    ++p;
  }

  if (chains.size() == 0) add_default_chain();
  string temp, another;

  p = opts.begin();
  while(p != opts.end()) {
    temp = *p;
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting setup, with args \""
		  + temp + "\", \"" + another + "\".");
    interpret_chains(temp);
    interpret_audio_format(temp);
    if (chains.size() == 0) add_default_chain();
    interpret_audioio_device(temp);
    interpret_effect_preset(temp);
    interpret_chain_operator(temp);
    interpret_controller(temp);
    ++p;
  }

  if (inputs.size() > 0 && outputs.size() == 0) {
    // No -o[:] options specified; let's use the default output
    select_all_chains();
    ECA_RESOURCES ecaresources;
    interpret_audioio_device("-o:" +  ecaresources.resource("default-output"));
  }
}

void ECA_CHAINSETUP::interpret_general_option (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 'b':
    {
      set_buffersize(atoi(get_argument_number(1, argu).c_str()));
      MESSAGE_ITEM mitemb;
      mitemb << "(eca-chainsetup) Setting buffersize to (samples) " << buffersize() << ".";
      ecadebug->msg(0, mitemb.to_string()); 
      break;
    }

  case 'm':      // mixmode
    {
      string temp = get_argument_number(1, argu);
      if (temp == "auto") {
	mixmode_rep = ep_mm_auto;
	ecadebug->msg("(eca-chainsetup) Mix-mode is selected automatically.");
      }
      else if (temp == "mthreaded") {
	ecadebug->msg("(eca-chainsetup) Multithreaded mixmode selected.");
	mixmode_rep = ep_mm_mthreaded;
      }
      else if (temp == "simple") {
	ecadebug->msg("(eca-chainsetup) Simple mixmode selected.");
	mixmode_rep = ep_mm_simple;
      }
      else if (temp == "normal") {
	ecadebug->msg("(eca-chainsetup) Normal mixmode selected.");
	mixmode_rep = ep_mm_normal;
      }
      break;
    }

  case 'n':
    {
      setup_name = get_argument_number(1, argu);
      ecadebug->msg("(eca-chainsetup) Setting chainsetup name to \""
		    + setup_name + "\".");
      break;
    }

  case 's':
    {
      if (argu.size() > 2 && argu[2] == 'r') {
	set_sample_rate(atol(get_argument_number(1,argu).c_str()));
	ecadebug->msg("(eca-chainsetup) Setting internal sample rate to: "
		      + get_argument_number(1,argu));
      }
      break;
    }

  case 'x':
    {
      ecadebug->msg("(eca-chainsetup) Truncating outputs.");
      set_output_openmode(AUDIO_IO::io_write);
      break;
    }

  case 'z':
    {
      if (get_argument_number(1, argu) == "db") {
	ecadebug->msg("(eca-chainsetup) Using double-buffering with all audio inputs that support it.");
	toggle_double_buffering(true);
      }
      else if (get_argument_number(1, argu) == "psr") {
	ecadebug->msg("(eca-chainsetup) Using precise-sample-rates with OSS audio devices.");
	toggle_precise_sample_rates(true);
      }
      break;
    }
  default: { }
  }
}

void ECA_CHAINSETUP::interpret_processing_control (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() < 2) return;

  switch(argu[1]) {
  case 't': 
    { 
      if (argu.size() < 3) return;
      switch(argu[2]) {
      case ':': 
	{
	  length_in_seconds(atof(get_argument_number(1, argu).c_str()));
	  ecadebug->msg("(eca-chainsetup) Set processing time to "
			+ kvu_numtostr(length_in_seconds()) + ".");
	  break;
	}
	
      case 'l': 
	{
	  toggle_looping(true);
	  ecadebug->msg("(eca-chainsetup) Looping enabled.");
	  break;
	}
      }
      break;
    }
    break;
  }
}

void ECA_CHAINSETUP::interpret_chains (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() < 2) return;  

  switch(argu[1]) {
  case 'a':
    {
      vector<string> schains = get_arguments(argu);
      if (find(schains.begin(), schains.end(), "all") != schains.end()) {
	select_all_chains();
	ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Selected all chains.");
      }
      else {
	select_chains(schains);
	add_new_chains(schains);
	MESSAGE_ITEM mtempa;
	mtempa << "(eca-chainsetup) Selected chain ids: ";
	for (vector<string>::const_iterator p = schains.begin(); p !=
	       schains.end(); p++) { mtempa << *p << " "; }
	ecadebug->msg(ECA_DEBUG::system_objects, mtempa.to_string());
      }
      break;
    }
  default: { }
  }
  return; // to avoid a internal compiler error
}

void ECA_CHAINSETUP::interpret_audio_format (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() < 2) return; 

  switch(argu[1]) {
  case 'f':
    {
      ECA_AUDIO_FORMAT active_sinfo;
      active_sinfo.set_sample_format(get_argument_number(1, argu));
      active_sinfo.set_channels(atoi(get_argument_number(2, argu).c_str()));
      active_sinfo.set_samples_per_second(atol(get_argument_number(3, argu).c_str()));
      set_default_audio_format(active_sinfo);
      
      MESSAGE_ITEM ftemp;
      ftemp << "(eca-chainsetup) Set active format to (bits/channels/srate): ";
      ftemp << active_sinfo.format_string() << "/" << (int)active_sinfo.channels() << "/" << active_sinfo.samples_per_second();
      ecadebug->msg(ftemp.to_string());
      break;
    }
  default: { }
  }
  return; // to avoid a internal compiler error
}

void ECA_CHAINSETUP::interpret_effect_preset (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() < 2) return;

  switch(argu[1]) {
  case 'p':
    {
      if (argu.size() < 3) return;  
      switch(argu[2]) {
      case 'f':
	{
	  add_chain_operator(dynamic_cast<CHAIN_OPERATOR*>(new FILE_PRESET(get_argument_number(1,argu))));
	  break;
	}

      case 'n': 
	{
	  string name = get_argument_number(1,argu);
	  const map<string,string>& preset_map = eca_preset_map.registered_objects();
	  map<string,string>::const_iterator p = preset_map.begin();
	  while (p != preset_map.end()) {
	    if (p->first == name) {
	      add_chain_operator(dynamic_cast<CHAIN_OPERATOR*>(new GLOBAL_PRESET(name)));
	    }
	    ++p;
	  }
	  break;
	}
	
      default: { }
      }
      break;
    }
  default: { }
  }
}

void ECA_CHAINSETUP::interpret_chain_operator (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  CHAIN_OPERATOR* t = create_chain_operator(argu);
  if (t == 0) t = create_ladspa_plugin(argu);
  if (t != 0) add_chain_operator(t);
  else 
    interpret_effect_preset(argu);
}

CHAIN_OPERATOR* ECA_CHAINSETUP::create_ladspa_plugin (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = 0;
  string prefix = get_argument_prefix(argu);
  if (prefix == "el") {
    prefix = get_argument_number(1, argu);
    cop = ECA_LADSPA_PLUGIN_MAP::object(prefix);
    if (cop != 0) {
      cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());
      cop->map_parameters();

      ecadebug->control_flow("Chainsetup/Adding LADSPA-plugin \"" +
			     cop->name() + "\"");
      otemp << "Setting parameters: ";
      for(int n = 0; n < cop->number_of_params(); n++) {
	cop->set_parameter(n + 1, atof(get_argument_number(n + 2, argu).c_str()));
	otemp << cop->get_parameter_name(n + 1) << " = ";
	otemp << cop->get_parameter(n + 1);
	if (n + 1 < cop->number_of_params()) otemp << ", ";
      }
      ecadebug->msg(otemp.to_string());
    }
    return(cop);
  }
  return(0);
}

CHAIN_OPERATOR* ECA_CHAINSETUP::create_chain_operator (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  string prefix = get_argument_prefix(argu);

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = ECA_CHAIN_OPERATOR_MAP::object(prefix);
  if (cop != 0) {
    cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());
    cop->map_parameters();

    ecadebug->control_flow("Chainsetup/Adding chain operator \"" +
			   cop->name() + "\"");
    //    otemp << "(eca-chainsetup) Adding effect " << cop->name();
    otemp << "Setting parameters: ";
    for(int n = 0; n < cop->number_of_params(); n++) {
      cop->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << cop->get_parameter_name(n + 1) << " = ";
      otemp << cop->get_parameter(n +1);
      if (n + 1 < cop->number_of_params()) otemp << ", ";
    }
    ecadebug->msg(otemp.to_string());
    return(cop);
  }
  return(0);
}

void ECA_CHAINSETUP::interpret_controller (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  GENERIC_CONTROLLER* t = create_controller(argu);
  if (t != 0) add_controller(t);
}

GENERIC_CONTROLLER* ECA_CHAINSETUP::create_controller (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() > 0 && argu[0] != '-') return(0);
  string prefix = get_argument_prefix(argu);
  if (prefix == "kx") {
    set_target_to_controller();
    ecadebug->msg(ECA_DEBUG::system_objects, "Selected controllers as parameter control targets.");
    return(0);
  }

  MESSAGE_ITEM otemp;

  GENERIC_CONTROLLER* gcontroller = ECA_CONTROLLER_MAP::object(prefix);
  if (gcontroller != 0) {
    gcontroller = gcontroller->clone();
    gcontroller->map_parameters();

    ecadebug->control_flow("Chainsetup/Adding controller source \"" +  gcontroller->name() + "\"");

    gcontroller->init((double)buffersize() / sample_rate());

    otemp << "Setting parameters: ";
    for(int n = 0; n < gcontroller->number_of_params(); n++) {
      gcontroller->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << gcontroller->get_parameter_name(n + 1) << " = ";
      otemp << gcontroller->get_parameter(n +1);
      if (n + 1 < gcontroller->number_of_params()) otemp << ", ";
    }
    ecadebug->msg(otemp.to_string());
    return(gcontroller);
  }
  return(0);
}

void ECA_CHAINSETUP::set_target_to_controller(void) {
  vector<string> schains = selected_chains();
  for(vector<string>::const_iterator a = schains.begin(); a != schains.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->selected_controller_as_target();
	return;
      }
    }
  }
}

void ECA_CHAINSETUP::add_controller(GENERIC_CONTROLLER* csrc) {
  // --------
  REQUIRE(csrc != 0);
  // --------

  vector<string> schains = selected_chains();
  for(vector<string>::const_iterator a = schains.begin(); a != schains.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->add_controller(csrc);
	return;
      }
    }
  }
}

void ECA_CHAINSETUP::add_chain_operator(CHAIN_OPERATOR* cotmp) {
  // --------
  REQUIRE(cotmp != 0);
  // --------

  vector<string> schains = selected_chains();
  assert(schains.size() == 1);
  for(vector<string>::const_iterator p = schains.begin(); p != schains.end(); p++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*p == (*q)->name()) {
	ecadebug->msg(ECA_DEBUG::system_objects, "Adding chainop to chain " + (*q)->name() + ".");
	(*q)->add_chain_operator(cotmp);
	(*q)->selected_chain_operator_as_target();
	return;
	//	(*q)->add_chain_operator(cotmp->clone());
      }
    }
  }

  // --------
  ENSURE(chains.size() > 0);
  // --------
}

void ECA_CHAINSETUP::load_from_file(const string& filename) throw(ECA_ERROR*) { 
  ifstream fin (filename.c_str());
  if (!fin) throw(new ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup read file: \"" + filename + "\".", ECA_ERROR::retry));

  string temp, another;
  while(fin >> temp) {
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainseup) Adding \"" + temp + "\" to options (loaded from \"" + filename + "\".");
    options.push_back(temp);
  }
  fin.close();

  setup_filename = filename;
  options = COMMAND_LINE::combine(options);
}

void ECA_CHAINSETUP::save(void) throw(ECA_ERROR*) { 
  if (setup_filename.empty() == true)
    setup_filename = setup_name + ".ecs";
  save_to_file(setup_filename);
}

void ECA_CHAINSETUP::save_to_file(const string& filename) throw(ECA_ERROR*) {
  update_option_strings();

  ofstream fout (filename.c_str());
  if (!fout) {
    cerr << "Going to throw an exception...\n";
    throw(new ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup save file: \"" +
  			filename + "\".", ECA_ERROR::retry));
  }

  fout << options_general << "\n";
  fout << inputs_to_string() << "\n";
  fout << outputs_to_string() << "\n";
  fout << chains_to_string() << "\n";
  fout.close();

  setup_filename = filename;
}

void ECA_CHAINSETUP::update_option_strings(void) {
  options_general = general_options_to_string();
  
  while(options.size() > 0) options.pop_back();

  string temp = options_general + 
                inputs_to_string() + 
                outputs_to_string() +
                chains_to_string();

  options = string_to_words(temp);
}

string ECA_CHAINSETUP::general_options_to_string(void) const {
  MESSAGE_ITEM t;

  t << "-b:" << buffersize();
  t << " -sr:" << sample_rate();

  t << " -n:" << setup_name;

  switch(mixmode()) {
  case ep_mm_simple: {
    t << " -m:simple";
    break;
  }
  case ep_mm_normal: {
     t << " -m:normal";
    break;
  }
  case ep_mm_mthreaded: {
    t << " -m:mthreaded";
    break;
  }
  default: { }
  }

  if (output_openmode() == AUDIO_IO::io_write) t << " -x";
  if (double_buffering()) t << " -z:db";
  if (precise_sample_rates()) t << " -z:psr";

  t.setprecision(3);
  if (length_set()) {
    t << " -t:" << length_in_seconds();
    if (looping_enabled()) t << " -tl";
  }

  return(t.to_string());
}
