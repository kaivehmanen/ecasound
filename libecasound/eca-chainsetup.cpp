// ------------------------------------------------------------------------
// eca-chainsetup.cpp: A class representing a group of chains and their
//                     configuration.
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
#include <algorithm>
#include <fstream>
#include <vector>
#include <iostream>

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "eca-resources.h"
#include "eca-session.h"

#include "generic-controller.h"
#include "eca-chainop.h"
#include "file-preset.h"
#include "global-preset.h"

#include "audioio.h"
#include "audioio-types.h"
#include "audiofx_ladspa.h"

#include "eca-object-map.h"
#include "eca-preset-map.h"
#include "eca-static-object-maps.h"

#include "midiio.h"
#include "midi-client.h"

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
 * If any invalid options are passed us argument, 
 * interpret_result() will be 'true', and 
 * interpret_result_verbose() contains more detailed 
 * error description.
 *
 * ensure:
 *   buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(const std::vector<std::string>& opts) 
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
  DBC_ENSURE(buffersize() != 0);
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
  DBC_ENSURE(buffersize() != 0);
  // --------
}

/**
 * Construct from a chainsetup file. ff 'fromfile' is
 * 
 * If any invalid options are passed us argument, 
 * interpret_result() will be 'true', and 
 * interpret_result_verbose() contains more detailed 
 * error description.
 *
 * ensure:
 *   buffersize != 0
 */
ECA_CHAINSETUP::ECA_CHAINSETUP(const std::string& setup_file) 
  : ECA_CHAINSETUP_POSITION(SAMPLE_SPECS::sample_rate_default) {

  setup_name_rep = "";
  set_defaults();
  load_from_file(setup_file);
  if (setup_name_rep == "") setup_name_rep = setup_file;
  preprocess_options(options);
  interpret_options(options);
  add_default_output();

  // --------
  DBC_ENSURE(buffersize() != 0);
  // --------
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
 */
void ECA_CHAINSETUP::set_defaults(void) {
  is_enabled_rep = false;
  mixmode_rep = ep_mm_auto;
  last_audio_object = 0;

  set_output_openmode(AUDIO_IO::io_readwrite);

  ECA_RESOURCES ecaresources;
  set_default_midi_device(ecaresources.resource("midi-device"));
  set_buffersize(atoi(ecaresources.resource("default-buffersize").c_str()));
  set_sample_rate(atol(ecaresources.resource("default-samplerate").c_str()));
  toggle_double_buffering(ecaresources.boolean_resource("default-to-double-buffering"));
  set_double_buffer_size(atol(ecaresources.resource("default-double-buffer-size").c_str()));
  toggle_precise_sample_rates(ecaresources.boolean_resource("default-to-precise-sample-rates"));
  toggle_max_buffers(ecaresources.boolean_resource("default-to-internal-buffering"));
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
 * ensure:
 *   is_enabled() == false
 */
void ECA_CHAINSETUP::disable(void) {
  update_option_strings();
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
  DBC_ENSURE(is_enabled() == false);
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
 * Preprocesses a set of options.
 * 
 * ensure:
 *  all options valid for interpret_option()
 */
void ECA_CHAINSETUP::preprocess_options(std::vector<std::string>& opts) {
  std::vector<std::string>::iterator p = opts.begin();
  while(p != opts.end()) {

    /* handle options not starting with an '-' sign */

    if (p->size() > 0 && (*p)[0] != '-') {
      /* hack1: options ending with .ecs as "-s:file.ecs" */
      string::size_type pos = p->find(".ecs");
      if (pos + 4 == p->size())
	*p = "-s:" + *p;
      
      /* hack2: rest as "-i:file.ecs" */
      else
	*p = "-i:" + *p;
    }
    ++p;
  }
}

/**
 * Resets the interpretation logic.
 *
 * ensure:
 *  interpret_status() != true
 */
void ECA_CHAINSETUP::interpret_entry(void) {
  istatus_rep = false;
  interpret_set_result(true, "");

  DBC_ENSURE(interpret_match_found() != true);
}

/**
 * Exits the interpretation logic.
 *
 * ensure:
 *  interpret_result() == true && interpret_result_verbose() == "" ||
 *  interpret_result() == false && interpret_result_verbose() != ""
 */
void ECA_CHAINSETUP::interpret_exit(const std::string& arg) {
  if (istatus_rep != true) {
    /* option 'arg' was not found */
    interpret_set_result(false, string("Interpreting option \"") +
			 arg + 
			 "\" failed.");
  }
  else {
    /* option 'arg' was found, but incorrect */
    if (interpret_result() != true) {
      if (interpret_result_verbose() == "") {
	interpret_set_result(false, string("Interpreting option \"") +
			     arg + 
			     "\" failed.");
      }
      /* else -> otherwise error code is already set */
    }
  }

  DBC_ENSURE(interpret_result() == true && interpret_result_verbose() == "" ||
	     interpret_result() == false && interpret_result_verbose() != "");
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
 *  (option succesfully interpreted && interpret_result() ==  true) ||
 *  (unknown or invalid option && interpret_result() != true)
 */
void ECA_CHAINSETUP::interpret_option (const std::string& arg) {
  interpret_entry();

  istatus_rep = false;
  interpret_global_option(arg);
  if (istatus_rep != true) interpret_object_option(arg);

  interpret_exit(arg);
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
void ECA_CHAINSETUP::interpret_global_option (const std::string& arg) {
  interpret_entry();

  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting global option \"" + arg + "\".");
  if (istatus_rep == false) interpret_general_option(arg);
  if (istatus_rep == false) interpret_processing_control(arg);
  if (istatus_rep == false) interpret_chains(arg);

  interpret_exit(arg);
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
void ECA_CHAINSETUP::interpret_object_option (const std::string& arg) {
  interpret_entry();

  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Interpreting object option \"" + arg + "\".");
  interpret_chains(arg);
  if (istatus_rep == false) interpret_audio_format(arg);
  if (istatus_rep == false) interpret_audioio_device(arg);
  if (istatus_rep == false) interpret_midi_device(arg);
  if (istatus_rep == false) interpret_chain_operator(arg);
  if (istatus_rep == false) interpret_controller(arg);

  interpret_exit(arg);
}

/**
 * Interpret a vector of options.
 */
void ECA_CHAINSETUP::interpret_options(vector<string>& opts) {
  int optcount = static_cast<int>(opts.size());
  int global_matches = 0;
  int other_matches = 0;

  interpret_set_result(true, ""); /* if opts.size() == 0 */

  /*
   * phase1: parse global options only */

  vector<string>::iterator p = opts.begin();
  p = opts.begin();
  while(p != opts.end()) {
    interpret_global_option(*p);
    if (interpret_match_found() == true) global_matches++;
    ++p;
  }

  if (chains.size() == 0) add_default_chain();

  /* 
   * phase2: parse all options, including processing
   *         the global options again */

  p = opts.begin();
  while(p != opts.end()) {
    interpret_object_option(*p);
    if (interpret_match_found() == true) {
      other_matches++;
      if (interpret_result() != true) {
	/* invalid option format */
	break;
      }
    }
    else {
      int dlevel = ecadebug->get_debug_level(); /* hack to avoid printing the same info
						   multiple times */
      ecadebug->disable();
      interpret_global_option(*p);
      ecadebug->set_debug_level(dlevel);
      if (interpret_match_found() != true) {
	interpret_set_result(false, string("Invalid argument, unable to parse: '") + *p + "'");
	break;
      }
      else {
	if (interpret_result() != true) {
	  /* invalid option format */
	  break;
	}
      }
    }
    ++p;
  }

  if (other_matches + global_matches != optcount) {
    ecadebug->msg(string("(eca-chainsetup) Warning! While parsing options, only ") + 
		  kvu_numtostr(other_matches) +
		  "+" +
		  kvu_numtostr(global_matches) +
		  " matches were found; expected " +
		  kvu_numtostr(optcount) +
		  " options.");
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
void ECA_CHAINSETUP::interpret_general_option (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
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
      std::string temp = get_argument_number(1, argu);
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
      ecadebug->msg("(eca-chainsetup) Truncating outputs (overwrite-mode).");
      set_output_openmode(AUDIO_IO::io_write);
      break;
    }

  case 'X':
    {
      ecadebug->msg("(eca-chainsetup) Updating outputs (rw-mode).");
      set_output_openmode(AUDIO_IO::io_readwrite);
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
      else if (get_argument_number(1, argu) == "nodb") {
	ecadebug->msg("(eca-chainsetup) Double-buffering disabled.");
	toggle_double_buffering(false);
      }
      else if (get_argument_number(1, argu) == "intbuf") {
	ecadebug->msg("(eca-chainsetup) Enabling extra buffering on realtime devices.");
	toggle_max_buffers(true);
      }
      else if (get_argument_number(1, argu) == "nointbuf") {
	ecadebug->msg("(eca-chainsetup) Disabling extra buffering on realtime devices.");
	toggle_max_buffers(false);
      }
      else if (get_argument_number(1, argu) == "psr") {
	ecadebug->msg("(eca-chainsetup) Enabling precise-sample-rates with OSS audio devices.");
	toggle_precise_sample_rates(true);
      }
      else if (get_argument_number(1, argu) == "nopsr") {
	ecadebug->msg("(eca-chainsetup) Disabling precise-sample-rates with OSS audio devices.");
	toggle_precise_sample_rates(false);
      }
      else if (get_argument_number(1, argu) == "xruns") {
	ecadebug->msg("(eca-chainsetup) Processing is stopped if an xrun occurs.");
	toggle_ignore_xruns(false);
      }
      else if (get_argument_number(1, argu) == "noxruns") {
	ecadebug->msg("(eca-chainsetup) Ignoring xruns during processing.");
	toggle_ignore_xruns(true);
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
void ECA_CHAINSETUP::interpret_processing_control (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
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
	    ecadebug->msg("(eca-chainsetup) Looping enabled. Lenght of input objects will be used to set the loop point.");
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
void ECA_CHAINSETUP::interpret_chains (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
  // --------

  bool match = true;
  if (argu.size() < 2) return;  
  switch(argu[1]) {
  case 'a':
    {
      std::vector<std::string> schains = get_arguments(argu);
      if (find(schains.begin(), schains.end(), "all") != schains.end()) {
	select_all_chains();
	ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup) Selected all chains.");
      }
      else {
	select_chains(schains);
	add_new_chains(schains);
	MESSAGE_ITEM mtempa;
	mtempa << "(eca-chainsetup) Selected chain ids: ";
	for (std::vector<std::string>::const_iterator p = schains.begin(); p !=
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
void ECA_CHAINSETUP::interpret_audio_format (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
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
void ECA_CHAINSETUP::interpret_effect_preset (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
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
	 std::string name = get_argument_number(1,argu);
  	  const std::map<std::string,std::string>& preset_map = eca_preset_map->registered_objects(); 
	  std::map<std::string,std::string>::const_iterator p = preset_map.begin();
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
void ECA_CHAINSETUP::interpret_audioio_device (const std::string& argu) { 
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
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
	  interpret_set_result(false, string("(eca-chainsetup) I/O-mode 'io_read' not supported by ") + last_audio_object->name());
	}
	else {
	  last_audio_object->io_mode(AUDIO_IO::io_read);
	  last_audio_object->set_audio_format(default_audio_format_rep);
	  add_input(last_audio_object);
	}
      }
      else {
	interpret_set_result(false, string("(eca-chainsetup) Format of input ") +
			     tname + " not recognized.");
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
	  interpret_set_result(false, string("(eca-chainsetup) I/O-mode 'io_write' not supported by ") + last_audio_object->name());
	}
	else {
	  last_audio_object->io_mode(mode_tmp);
	  last_audio_object->set_audio_format(default_audio_format_rep);
	  add_output(last_audio_object);
	}
      }
      else {
	interpret_set_result(false, string("(eca-chainsetup) Format of output ") +
			     tname + " not recognized.");
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
}

/**
 * Handles MIDI-IO devices.
 *
 * require:
 *  argu.size() > 0
 *  argu[0] == '-'
 */
void ECA_CHAINSETUP::interpret_midi_device (const std::string& argu) { 
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
  // --------
 
  bool match = true;
  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 'M':
    {
      if (argu.size() < 3) return;
      switch(argu[2]) {
	case 'd': 
	  {
	 std::string tname = get_argument_number(1, argu);
	    ecadebug->msg(ECA_DEBUG::system_objects,"(eca-chainsetup) MIDI-config: Adding device \"" + tname + "\".");
	    MIDI_IO* mdev = 0;
	    mdev = ECA_OBJECT_FACTORY::create_midi_device(argu);
	    if (mdev != 0) {
	      if ((mdev->supported_io_modes() & MIDI_IO::io_readwrite) == MIDI_IO::io_readwrite) {
		mdev->io_mode(MIDI_IO::io_readwrite);
		add_midi_device(mdev);
	      }
	      else {
		ecadebug->msg(ECA_DEBUG::info, "(eca-chainsetup) MIDI-config: Warning! I/O-mode 'io_readwrite' not supported by " + mdev->name());
	      }
	    }
	    break;
	  }

      case 'm': 
	{
	  if (argu.size() < 4) return;
	  switch(argu[3]) {
	  case 'r': 
	    {
	      // FIXME: not implemented!
	      int id = atoi(get_argument_number(1, argu).c_str());
	      ecadebug->msg(ECA_DEBUG::info, 
			    "(eca-chainsetup) MIDI-config: Receiving MMC messages with id  \"" + 
			    kvu_numtostr(id) +
			    "\".");
	      midi_server_rep.set_mmc_receive_id(id);
	      break;
	    }
	  

	  case 's': 
	    {
	      int id = atoi(get_argument_number(1, argu).c_str());
	      ecadebug->msg(ECA_DEBUG::info, 
			    "(eca-chainsetup) MIDI-config: Adding MMC-send to device id \"" + 
			    kvu_numtostr(id) +
			    "\".");
	      midi_server_rep.add_mmc_send_id(id);
	      break;
	    }
	  }
	  break;
	}

      case 's': 
	{
	  if (argu.size() < 4) return;
	  switch(argu[3]) {
	  case 'r': 
	    {
	      // FIXME: not implemented
	      ecadebug->msg(ECA_DEBUG::info, 
			    "(eca-chainsetup) MIDI-config: Receiving MIDI-sync.");
	      midi_server_rep.toggle_midi_sync_receive(true);
	      break;
	    }
	  
	  case 's': 
	    {
	      // FIXME: not implemented
	      ecadebug->msg(ECA_DEBUG::info, 
			    "(eca-chainsetup) MIDI-config: Sending MIDI-sync.");
	      midi_server_rep.toggle_midi_sync_send(true);
	      break;
	    }
	  }
	  break;
	}

      }
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
void ECA_CHAINSETUP::interpret_chain_operator (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
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
CHAIN_OPERATOR* ECA_CHAINSETUP::create_ladspa_plugin (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

#ifdef HAVE_LADSPA_H
  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = 0;
 std::string prefix = get_argument_prefix(argu);
  if (prefix == "el" || prefix == "eli") {
 std::string unique = get_argument_number(1, argu);
    if (prefix == "el") 
      cop = ECA_OBJECT_FACTORY::ladspa_map_object(unique);
    else 
      cop = ECA_OBJECT_FACTORY::ladspa_map_object(atol(unique.c_str()));

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
#endif /* HAVE_LADSPA_H */
  return(0);
}

/**
 * Create a new VST1.0/2.0 plugin
 */
CHAIN_OPERATOR* ECA_CHAINSETUP::create_vst_plugin (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = 0;
 std::string prefix = get_argument_prefix(argu);

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
CHAIN_OPERATOR* ECA_CHAINSETUP::create_chain_operator (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  string prefix = get_argument_prefix(argu);

  MESSAGE_ITEM otemp;
  CHAIN_OPERATOR* cop = ECA_OBJECT_FACTORY::chain_operator_map_object(prefix);
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
void ECA_CHAINSETUP::interpret_controller (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
  // --------

  GENERIC_CONTROLLER* t = create_controller(argu);
  if (t != 0) {
    MIDI_CLIENT* p = dynamic_cast<MIDI_CLIENT*>(t->source_pointer());
    if (p != 0) {
      if (midi_devices.size() == 0) 
	interpret_midi_device("-Md:" + default_midi_device());
      p->register_server(&midi_server_rep);
    }
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
GENERIC_CONTROLLER* ECA_CHAINSETUP::create_controller (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  // --------

  if (argu.size() > 0 && argu[0] != '-') return(0);
  string prefix = get_argument_prefix(argu);
  if (prefix == "kx") {
    set_target_to_controller();
    ecadebug->msg(ECA_DEBUG::system_objects, "Selected controllers as parameter control targets.");
    return(0);
  }

  MESSAGE_ITEM otemp;

  GENERIC_CONTROLLER* gcontroller = ECA_OBJECT_FACTORY::controller_map_object(prefix);
  if (gcontroller != 0) {
    gcontroller = gcontroller->clone();

    ecadebug->control_flow("Chainsetup/Adding controller source \"" +  gcontroller->name() + "\"");

    gcontroller->init((double)buffersize() / sample_rate());

    otemp << "Setting parameters: ";
    int numparams = gcontroller->number_of_params();
    for(int n = 0; n < numparams; n++) {
      gcontroller->set_parameter(n + 1, atof(get_argument_number(n + 1, argu).c_str()));
      otemp << gcontroller->get_parameter_name(n + 1) << " = ";
      otemp << gcontroller->get_parameter(n +1);
      numparams = gcontroller->number_of_params(); // in case 'n_o_p()' varies
      if (n + 1 < numparams) otemp << ", ";
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
 * require:
 *   csrc != 0
 *   selected_chains().size() == 1
 */
void ECA_CHAINSETUP::add_controller(GENERIC_CONTROLLER* csrc) {
  // --------
  DBC_REQUIRE(csrc != 0);
  // --------

  AUDIO_STAMP_CLIENT* p = dynamic_cast<AUDIO_STAMP_CLIENT*>(csrc->source_pointer());
  if (p != 0) {
//      cerr << "Found a stamp client!" << endl;
    p->register_server(&stamp_server_rep);
  }

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
 * require:
 *   cotmp != 0
 *   selected_chains().size() == 1
 */
void ECA_CHAINSETUP::add_chain_operator(CHAIN_OPERATOR* cotmp) {
  // --------
  DBC_REQUIRE(cotmp != 0);
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

void ECA_CHAINSETUP::load_from_file(const std::string& filename) throw(ECA_ERROR&) { 
  std::ifstream fin (filename.c_str());
  if (!fin) throw(ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup read file: \"" + filename + "\".", ECA_ERROR::retry));

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

  setup_filename_rep = filename;
  options = COMMAND_LINE::combine(options);
}

void ECA_CHAINSETUP::save(void) throw(ECA_ERROR&) { 
  if (setup_filename_rep.empty() == true)
    setup_filename_rep = setup_name_rep + ".ecs";
  save_to_file(setup_filename_rep);
}

void ECA_CHAINSETUP::save_to_file(const std::string& filename) throw(ECA_ERROR&) {
  update_option_strings();

  std::ofstream fout (filename.c_str());
  if (!fout) {
    std::cerr << "Going to throw an exception...\n";
    throw(ECA_ERROR("ECA_CHAINSETUP", "Couldn't open setup save file: \"" +
  			filename + "\".", ECA_ERROR::retry));
  }
  else {
    fout << options_general << "\n";
    fout << midi_to_string() << "\n";
    fout << inputs_to_string() << "\n";
    fout << outputs_to_string() << "\n";
    fout << chains_to_string() << "\n";
    fout.close();
    setup_filename_rep = filename;
  }
}

void ECA_CHAINSETUP::update_option_strings(void) {
  options_general = general_options_to_string();
  
  while(options.size() > 0) options.pop_back();

 std::string temp = options_general + 
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

  if (output_openmode() == AUDIO_IO::io_write) 
    t << " -x";
  else
    t << " -X";

  if (max_buffers() == true) 
    t << " -z:intbuf";
  else
    t << " -z:nointbuf";

  if (ignore_xruns() == true) 
    t << " -z:noxruns";
  else
    t << " -z:xruns";

  if (double_buffering() == true) 
    t << " -z:db," << double_buffer_size();
  else
    t << " -z:nodb";

  if (precise_sample_rates() == true) 
    t << " -z:psr";
  else
    t << " -z:nopsr";

  t.setprecision(3);
  if (length_set()) {
    t << " -t:" << length_in_seconds();
    if (looping_enabled()) t << " -tl";
  }

  return(t.to_string());
}
