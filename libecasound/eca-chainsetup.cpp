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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

#include "audioio.h"
#include "eca-static-object-maps.h"
#include "eca-ladspa-plugin-map.h"
#include "eca-chainop-map.h"
#include "eca-controller-map.h"

#include "eca-object-factory.h"
#include "eca-chainsetup-position.h"
#include "eca-audio-objects.h"
#include "sample-specs.h"

#include "eca-error.h"
#include "eca-debug.h"
#include "eca-chainsetup.h"

/**
 * Construct from a command line object.
 *
 * ensure:
 *   buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(const vector<string>& opts) 
  : ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_default),
    options(opts) {

  setup_name_rep = "command-line-setup";
  setup_filename_rep = "";

  set_defaults();

//    string temp;
//    cline.begin();
//    cline.next(); // skip the program name
//    while(cline.end() == false) {
//      temp = cline.current();
//      if (temp != "") {
//        ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Adding \"" + temp + "\" to options.");
//        options.push_back(temp);
//      }
//      cline.next();
//    }

  preprocess_options(options);
  interpret_options(options);
  add_default_output();

  // --------
  ENSURE(buffersize() != 0);
  // --------
}

/**
 * Constructs an empty chainsetup.
 *
 * ensure:
 *   buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(void) 
  : ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_default) {

  setup_name_rep = "";
  set_defaults();

  // --------
  ENSURE(buffersize() != 0);
  // --------
}

/**
 * Construct from a chainsetup file. ff 'fromfile' is
 *
 * ensure:
 *   buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(const string& setup_file) 
  : ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_default) {

  setup_name_rep = "";
  set_defaults();
  load_from_file(setup_file);
  if (setup_name_rep == "") setup_name_rep = setup_file;
  preprocess_options(options);
  interpret_options(options);
  add_default_output();

  // --------
  ENSURE(buffersize() != 0);
  // --------
}


/**
 * Sets default values.
 */
void ECA_CHAINSETUP::set_defaults(void) {
  register_default_objects();

  is_enabled_rep = false;
  mixmode_rep = ep_mm_auto;
  last_audio_object = 0;

  set_output_openmode(AUDIO_IO::io_readwrite);

  ECA_RESOURCES ecaresources;
  set_buffersize(atoi(ecaresources.resource("default-buffersize").c_str()));
  set_sample_rate(atol(ecaresources.resource("default-samplerate").c_str()));
  toggle_double_buffering(ecaresources.boolean_resource("default-to-double-buffering"));
  set_double_buffer_size(atol(ecaresources.resource("default-double-buffer-size").c_str()));
  toggle_precise_sample_rates(ecaresources.boolean_resource("default-to-precise-sample-rates"));
}

/**
 * Destructor
 */
ECA_CHAINSETUP::~ECA_CHAINSETUP(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects,"ECA_CHAINSETUP destructor!");
}

/**
 * Enable chainsetup. Opens all devices and reinitializes all 
 * chain operators if necessary.
 * 
 * ensure:
 *   is_enabled() == true
 */
void ECA_CHAINSETUP::enable(void) throw(ECA_ERROR&) {
  try {
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
    }
    is_enabled_rep = true;
  }
  catch(AUDIO_IO::SETUP_ERROR& e) {
    throw(ECA_ERROR("ECA-CHAINSETUP", 
		    string("Enabling chainsetup: ")
		    + e.message()));
  }

  // --------
  ENSURE(is_enabled() == true);
  // --------
}

/**
 * Disable chainsetup. Closes all devices. 
 * 
 * ensure:
 *   is_enabled() == false
 */
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

/**
 * Preprocesses a set of options.
 * 
 * ensure:
 *  all options valid for interpret_option()
 */
void ECA_CHAINSETUP::preprocess_options(vector<string>& opts) {
  vector<string>::iterator p = opts.begin();
  while(p != opts.end()) {
    if (p->size() > 0 && (*p)[0] != '-') *p = "-i:" + *p;
    ++p;
  }
}

/**
 * Interprets one option. This is the most generic variant of
 * the interpretation routines; both global and object specific
 * options are handled.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 * 
 * ensure:
 *  (option succesfully interpreted && interpretation_result() ==  true) ||
 *  (unknown or invalid option && interpretation_result() == false)
 */
void ECA_CHAINSETUP::interpret_option (const string& arg) throw(ECA_ERROR&) {
  interpret_global_option(arg);
  if (istatus_rep == false) interpret_object_option(arg);
}

/**
 * Interprets one option. All non-global options are ignored. Global
 * options can be interpreted multiple times and in any order.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 * 
 * ensure:
 *  (option succesfully interpreted && interpretation_result() ==  true) ||
 *  (unknown or invalid option && interpretation_result() == false)
 */
void ECA_CHAINSETUP::interpret_global_option (const string& arg) {
  istatus_rep = false;
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting global option \"" + arg + "\".");
  if (istatus_rep == false) interpret_general_option(arg);
  if (istatus_rep == false) interpret_processing_control(arg);
  if (istatus_rep == false) interpret_chains(arg);
}

/**
 * Interprets one option. All options not directly related to 
 * ecasound objects are ignored.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 * 
 * ensure:
 *  (option succesfully interpreted && interpretation_result() ==  true) ||
 *  (unknown or invalid option && interpretation_result() == false)
 */
void ECA_CHAINSETUP::interpret_object_option (const string& arg) throw(ECA_ERROR&) {
  istatus_rep = false;
  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting object option \"" + arg + "\".");
  interpret_chains(arg);
  if (istatus_rep == false) interpret_audio_format(arg);
  if (istatus_rep == false) interpret_audioio_device(arg);
  if (istatus_rep == false) interpret_chain_operator(arg);
  if (istatus_rep == false) interpret_controller(arg);
}

/**
 * Interpret a vector of options.
 */
void ECA_CHAINSETUP::interpret_options(vector<string>& opts) throw(ECA_ERROR&) {
  vector<string>::iterator p = opts.begin();
  p = opts.begin();
  while(p != opts.end()) {
    interpret_global_option(*p);
    ++p;
  }

  if (chains.size() == 0) add_default_chain();

  p = opts.begin();
  while(p != opts.end()) {
    interpret_object_option(*p);
    if (interpretation_status() == false) {
      int dlevel = ecadebug->get_debug_level(); /* hack to avoid printing the same info
						   multiple times */
      ecadebug->disable();
      interpret_global_option(*p);
      ecadebug->set_debug_level(dlevel);
      if (interpretation_status() == false) {
	throw(ECA_ERROR("ECA-CHAINSETUP", "Invalid argument, unable to parse: '" + *p + "'"));
      }
    }
    ++p;
  }
}

/**
 * Handle general options. 
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 *  istatus_rep == false
 */
void ECA_CHAINSETUP::interpret_general_option (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------

  bool match = true;
  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 'b':
    {
      set_buffersize(atoi(get_argument_number(1, argu).c_str()));
      MESSAGE_ITEM mitemb;
      mitemb << "(eca-chainsetup) Setting buffersize to (samples) " << buffersize() << ".";
      ecadebug->msg(mitemb.to_string()); 
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
      setup_name_rep = get_argument_number(1, argu);
      ecadebug->msg("(eca-chainsetup) Setting chainsetup name to \""
		    + setup_name_rep + "\".");
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
	long int bufs = atol(get_argument_number(2, argu).c_str());
	if (bufs != 0) 
	  set_double_buffer_size(bufs);
	ecadebug->msg("(eca-chainsetup) Using double-buffer of " + 
		      kvu_numtostr(double_buffer_size()) + " sample frames.");
	toggle_double_buffering(true);
      }
      else if (get_argument_number(1, argu) == "psr") {
	ecadebug->msg("(eca-chainsetup) Using precise-sample-rates with OSS audio devices.");
	toggle_precise_sample_rates(true);
      }
      break;
    }
  default: { match = false; }
  }
  if (match == true) istatus_rep = true;
}

/**
 * Handle processing control
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 *  istatus_rep == false
 */
void ECA_CHAINSETUP::interpret_processing_control (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------

  bool match = true;
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
			+ kvu_numtostr(length_in_seconds_exact()) + ".");
	  break;
	}
	
      case 'l': 
	{
	  toggle_looping(true);
	  if (length_set() != true)
	    ecadebug->msg("(eca-chainsetup) Looping enabled, although loop length is not set (-t:time). Looping won't work before it is set properly.");
	  else
	    ecadebug->msg("(eca-chainsetup) Looping enabled.");
	  break;
	}
      }
      break;
    }
  default: { match = false; }
  }
  if (match == true) istatus_rep = true;
}

/**
 * Handle chain options.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 *  istatus_rep == false
 */
void ECA_CHAINSETUP::interpret_chains (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------

  bool match = true;
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
  default: { match = false; }
  }
  if (match == true) istatus_rep = true;
}


/**
 * Handle chainsetup options.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 *  istatus_rep == false
 */
void ECA_CHAINSETUP::interpret_audio_format (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------

  bool match = true;
  if (argu.size() < 2) return; 
  switch(argu[1]) {
  case 'f':
    {
      ECA_AUDIO_FORMAT active_sinfo;
      active_sinfo.set_sample_format(get_argument_number(1, argu));
      active_sinfo.set_channels(atoi(get_argument_number(2, argu).c_str()));
      active_sinfo.set_samples_per_second(atol(get_argument_number(3, argu).c_str()));
      if (get_argument_number(4, argu) == "n")
	active_sinfo.toggle_interleaved_channels(false);
      else
	active_sinfo.toggle_interleaved_channels(true);
      set_default_audio_format(active_sinfo);
      
      MESSAGE_ITEM ftemp;
      ftemp << "(eca-chainsetup) Set active format to (bits/channels/srate): ";
      ftemp << active_sinfo.format_string() << "/" << (int)active_sinfo.channels() << "/" << active_sinfo.samples_per_second();
      ecadebug->msg(ECA_DEBUG::user_objects, ftemp.to_string());
      break;
    }
  default: { match = false; }
  }
  if (match == true) istatus_rep = true;
}

/**
 * Handle effect preset options.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 *  istatus_rep == false
 */
void ECA_CHAINSETUP::interpret_effect_preset (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------

  bool match = true;
  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 'p':
    {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting preset \"" + argu + "\".");
      CHAIN_OPERATOR* cop = 0;

      if (argu.size() < 3) return;  
      switch(argu[2]) {
      case 'f':
	{
//  	  add_chain_operator(dynamic_cast<CHAIN_OPERATOR*>(new FILE_PRESET(get_argument_number(1,argu))));
          cop = dynamic_cast<CHAIN_OPERATOR*>(new FILE_PRESET(get_argument_number(1,argu)));
	  break;
	}

      case 'n': 
	{
	  string name = get_argument_number(1,argu);
  	  const map<string,string>& preset_map = eca_preset_map.registered_objects(); 
	  map<string,string>::const_iterator p = preset_map.begin();
	  while (p != preset_map.end()) {
	    if (p->first == name) {
//	      add_chain_operator(dynamic_cast<CHAIN_OPERATOR*>(new GLOBAL_PRESET(name)));
	      cop = dynamic_cast<CHAIN_OPERATOR*>(new GLOBAL_PRESET(name));
	    }
	    ++p;
	  }
	  break;
	}
	
      default: { }
      }
      if (cop != 0) {
          MESSAGE_ITEM otemp;
          for(int n = 0; n < cop->number_of_params(); n++) {
              cop->set_parameter(n + 1, atof(get_argument_number(n + 2, argu).c_str()));
              otemp << cop->get_parameter_name(n + 1) << " = ";
              otemp << cop->get_parameter(n +1);
              if (n + 1 < cop->number_of_params()) otemp << ", ";
          }
          ecadebug->msg(otemp.to_string());          
          add_chain_operator(cop);
      }
      break;
    }
  default: { match = false; }
  }
  if (match == true) istatus_rep = true;
}

/**
 * Handle audio-IO-devices and files.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 */
void ECA_CHAINSETUP::interpret_audioio_device (const string& argu) throw(ECA_ERROR&) { 
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------
 
  string tname = get_argument_number(1, argu);
  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-audio-objects) adding file \"" + tname + "\".");

  bool match = true;
  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 'i':
    {
      ecadebug->msg(ECA_DEBUG::system_objects, "Eca-audio-objects/Parsing input");
      last_audio_object = ECA_OBJECT_FACTORY::create_audio_object(argu);
      if (last_audio_object == 0) 
	last_audio_object = create_loop_input(argu);
      if (last_audio_object != 0) {
	if ((last_audio_object->supported_io_modes() &
	    AUDIO_IO::io_read) != AUDIO_IO::io_read) {
	  throw(ECA_ERROR("ECA-CHAINSETUP", "I/O-mode 'io_read' not supported by " + last_audio_object->name()));
	}
	last_audio_object->io_mode(AUDIO_IO::io_read);
	last_audio_object->set_audio_format(default_audio_format_rep);
	add_input(last_audio_object);
      }
      else {
	throw(ECA_ERROR("ECA-CHAINSETUP", "Format of input " +
			    tname + " not recognized."));
      }
      break;
    }

  case 'o':
    {
      ecadebug->msg(ECA_DEBUG::system_objects, "Eca-audio-objects/Parsing output");
      last_audio_object = ECA_OBJECT_FACTORY::create_audio_object(argu);
      if (last_audio_object == 0) last_audio_object = create_loop_output(argu);
      if (last_audio_object != 0) {
	int mode_tmp = output_openmode();
	if (mode_tmp == AUDIO_IO::io_readwrite) {
	  if ((last_audio_object->supported_io_modes() &
	      AUDIO_IO::io_readwrite) != AUDIO_IO::io_readwrite) {
	    mode_tmp = AUDIO_IO::io_write;
	  }
	}
	if ((last_audio_object->supported_io_modes() &
	    mode_tmp != mode_tmp)) {
	    throw(ECA_ERROR("ECA-CHAINSETUP", "I/O-mode 'io_write' not supported by " + last_audio_object->name()));
	}
      
	last_audio_object->io_mode(mode_tmp);
	last_audio_object->set_audio_format(default_audio_format_rep);
	add_output(last_audio_object);
      }
      else {
	throw(ECA_ERROR("ECA-CHAINSETUP", "Format of output " +
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

  default: { match = false; }
  }
  if (match == true) istatus_rep = true;

  return;
}

/**
 * Handle chain operator options (chain operators, presets 
 * and plugins)
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 *  istatus_rep == false
 */
void ECA_CHAINSETUP::interpret_chain_operator (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------

  CHAIN_OPERATOR* t = create_chain_operator(argu);
  if (t == 0) t = create_ladspa_plugin(argu);
  if (t == 0) t = create_vst_plugin(argu);
  if (t != 0) {
    add_chain_operator(t);
    istatus_rep = true;
  }
  else 
    interpret_effect_preset(argu);
}

/**
 * Create a new LADSPA-plugin
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 */
CHAIN_OPERATOR* ECA_CHAINSETUP::create_ladspa_plugin (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = 0;
  string prefix = get_argument_prefix(argu);
  if (prefix == "el" || prefix == "eli") {
    string unique = get_argument_number(1, argu);
    if (prefix == "el") 
      cop = ECA_LADSPA_PLUGIN_MAP::object(unique);
    else 
      cop = ECA_LADSPA_PLUGIN_MAP::object(atol(unique.c_str()));

    if (cop != 0) {
      cop = dynamic_cast<CHAIN_OPERATOR*>(cop->new_expr());

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

/**
 * Create a new VST1.0/2.0 plugin
 */
CHAIN_OPERATOR* ECA_CHAINSETUP::create_vst_plugin (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  // --------

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = 0;
  string prefix = get_argument_prefix(argu);

#ifdef FEELING_EXPERIMENTAL
  cop = dynamic_cast<CHAIN_OPERATOR*>(eca_vst_plugin_map.object(prefix));
#endif
  if (cop != 0) {
    
    ecadebug->control_flow("Chainsetup/Adding VST-plugin \"" + cop->name() + "\"");
    otemp << "Setting parameters: ";
    for(int n = 0; n < cop->number_of_params(); n++) {
      cop->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << cop->get_parameter_name(n + 1) << " = ";
      otemp << cop->get_parameter(n + 1);
      if (n + 1 < cop->number_of_params()) otemp << ", ";
    }
    ecadebug->msg(otemp.to_string());
  }
  return(cop);
}

/**
 * Create a new chain operator
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 */
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

/**
 * Handle controller sources and general controllers.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 *  istatus_rep == false
 */
void ECA_CHAINSETUP::interpret_controller (const string& argu) {
  // --------
  REQUIRE(argu.size() > 0);
  REQUIRE(argu[0] == '-');
  REQUIRE(istatus_rep == false);
  // --------

  GENERIC_CONTROLLER* t = create_controller(argu);
  if (t != 0) {
    add_controller(t);
    istatus_rep = true;
  }
  if (get_argument_prefix(argu) == "kx") istatus_rep = true;
}

/**
 * Handle controller sources and general controllers.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 */
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

/**
 * Select controllers as targets for parameter control
 */
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

/**
 * Add general controller to selected chainop.
 *
 * require:
 *   csrc != 0
 *   selected_chains().size() == 1
 */
void ECA_CHAINSETUP::add_controller(GENERIC_CONTROLLER* csrc) {
  // --------
  REQUIRE(csrc != 0);
  // --------

  vector<string> schains = selected_chains();
  for(vector<string>::const_iterator a = schains.begin(); a != schains.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
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
 * require:
 *   cotmp != 0
 *   selected_chains().size() == 1
 */
void ECA_CHAINSETUP::add_chain_operator(CHAIN_OPERATOR* cotmp) {
  // --------
  REQUIRE(cotmp != 0);
  // --------

  vector<string> schains = selected_chains();
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
}

/**
 * If chainsetup has inputs, but no outputs, a default output is
 * added.
 */
void ECA_CHAINSETUP::add_default_output(void) {
  if (inputs.size() > 0 && outputs.size() == 0) {
    // No -o[:] options specified; let's use the default output
    select_all_chains();
    ECA_RESOURCES ecaresources;
    istatus_rep = false;
    interpret_audioio_device(string("-o:" + ecaresources.resource("default-output")));
  }
}

void ECA_CHAINSETUP::load_from_file(const string& filename) throw(ECA_ERROR&) { 
  ifstream fin (filename.c_str());
  if (!fin) throw(ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup read file: \"" + filename + "\".", ECA_ERROR::retry));

  string temp;
  while(getline(fin,temp)) {
    if (temp.size() > 0 && temp[0] == '#') {
      continue;
    }
    vector<string> words = string_to_words(temp);
    for(unsigned int n = 0; n < words.size(); n++) {
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainseup) Adding \"" + words[n] + "\" to options (loaded from \"" + filename + "\".");
      options.push_back(words[n]);
    }
  }
  fin.close();

  setup_filename_rep = filename;
  options = COMMAND_LINE::combine(options);
}

void ECA_CHAINSETUP::save(void) throw(ECA_ERROR&) { 
  if (setup_filename_rep.empty() == true)
    setup_filename_rep = setup_name_rep + ".ecs";
  save_to_file(setup_filename_rep);
}

void ECA_CHAINSETUP::save_to_file(const string& filename) throw(ECA_ERROR&) {
  update_option_strings();

  ofstream fout (filename.c_str());
  if (!fout) {
    cerr << "Going to throw an exception...\n";
    throw(ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup save file: \"" +
  			filename + "\".", ECA_ERROR::retry));
  }

  fout << options_general << "\n";
  fout << inputs_to_string() << "\n";
  fout << outputs_to_string() << "\n";
  fout << chains_to_string() << "\n";
  fout.close();

  setup_filename_rep = filename;
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

  t << " -n:" << setup_name_rep;

  switch(mixmode()) {
  case ep_mm_simple: {
    t << " -m:simple";
    break;
  }
  case ep_mm_normal: {
     t << " -m:normal";
    break;
  }
  default: { }
  }

  if (output_openmode() == AUDIO_IO::io_write) t << " -x";
  if (double_buffering()) t << " -z:db," << double_buffer_size();
  if (precise_sample_rates()) t << " -z:psr";

  t.setprecision(3);
  if (length_set()) {
    t << " -t:" << length_in_seconds();
    if (looping_enabled()) t << " -tl";
  }

  return(t.to_string());
}
