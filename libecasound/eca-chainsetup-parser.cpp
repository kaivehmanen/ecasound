// ------------------------------------------------------------------------
// eca-chainsetup-parser.cpp: Functionality for parsing chainsetup 
//                            option syntax.
// Copyright (C) 2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <algorithm> /* find() */

#include <kvutils/dbc.h> /* DBC_* */
#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "audioio.h"
#include "file-preset.h"
#include "global-preset.h"
#include "midiio.h"
#include "midi-client.h"
#include "midi-server.h"
#include "generic-controller.h"
#include "eca-chain.h"

#include "eca-object-factory.h"
#include "eca-preset-map.h"

#include "eca-chainsetup.h"
#include "eca-chainsetup-parser.h"
#include "eca-chainsetup-bufparams.h"

ECA_CHAINSETUP_PARSER::ECA_CHAINSETUP_PARSER(ECA_CHAINSETUP* csetup) 
  : csetup_repp(csetup), 
    last_audio_object_repp(0) 
{

}

/**
 * Interprets one option. This is the most generic variant of
 * the interpretation routines; both global and object specific
 * options are handled.
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * 
 * @post (option succesfully interpreted && interpret_result() ==  true) ||
 *       (unknown or invalid option && interpret_result() != true)
 */
void ECA_CHAINSETUP_PARSER::interpret_option (const std::string& arg) {
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
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @post (option succesfully interpreted && interpretation_result() ==  true) ||
 *       (unknown or invalid option && interpretation_result() == false)
 */
void ECA_CHAINSETUP_PARSER::interpret_global_option (const std::string& arg) {
  interpret_entry();

  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup-parser) Interpreting global option \"" + arg + "\".");
  if (istatus_rep == false) interpret_general_option(arg);
  if (istatus_rep == false) interpret_processing_control(arg);
  if (istatus_rep == false) interpret_chains(arg);

  interpret_exit(arg);
}

/**
 * Interprets one option. All options not directly related to 
 * ecasound objects are ignored.
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * 
 * @post (option succesfully interpreted && interpretation_result() ==  true) ||
 *       (unknown or invalid option && interpretation_result() == false)
 */
void ECA_CHAINSETUP_PARSER::interpret_object_option (const std::string& arg) {
  interpret_entry();

  ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup-parser) Interpreting object option \"" + arg + "\".");
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
void ECA_CHAINSETUP_PARSER::interpret_options(vector<string>& opts) {
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

    /* note! below we make sure we don't calculate chain 
     *       definitions twice */
    if (interpret_match_found() == true &&
	!((*p)[0] == '-' && (*p)[1] == 'a')) global_matches++;

    ++p;
  }

  if (csetup_repp->chains.size() == 0) csetup_repp->add_default_chain();

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
    ecadebug->msg(string("(eca-chainsetup-parser) Warning! Only ") + 
		  kvu_numtostr(other_matches) +
		  "+" +
		  kvu_numtostr(global_matches) +
		  " of the expected " +
		  kvu_numtostr(optcount) +
		  " parameters were recognized succesfully.");
  }
}

void ECA_CHAINSETUP_PARSER::reset_interpret_status(void) {
  istatus_rep = false;
}

/**
 * Preprocesses a set of options.
 * 
 * Notes! See also ECA_SESSION::preprocess_options()
 * 
 * @post all options valid for interpret_option()
 */
void ECA_CHAINSETUP_PARSER::preprocess_options(std::vector<std::string>& opts) const {
  std::vector<std::string>::iterator p = opts.begin();
  while(p != opts.end()) {

    /* handle options not starting with an '-' sign */

    if (p->size() > 0 && (*p)[0] != '-') {
      /* hack1: rest as "-i:file" */
      ecadebug->msg("(eca-chainsetup-parser) Note! Interpreting option " +
		    *p +
		    " as -i:" +
		    *p +
		    ".");
      *p = "-i:" + *p;
    }
    ++p;
  }
}

/**
 * Resets the interpretation logic.
 *
 * @post interpret_status() != true
 */
void ECA_CHAINSETUP_PARSER::interpret_entry(void) {
  istatus_rep = false;
  interpret_set_result(true, "");

  DBC_ENSURE(interpret_match_found() != true);
}

/**
 * Exits the interpretation logic.
 *
 * @post interpret_result() == true && interpret_result_verbose() == "" ||
 *       interpret_result() == false && interpret_result_verbose() != ""
 */
void ECA_CHAINSETUP_PARSER::interpret_exit(const std::string& arg) {
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
 * Handle general options. 
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre istatus_rep == false
 */
void ECA_CHAINSETUP_PARSER::interpret_general_option (const std::string& argu) {
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
      int bsize = atoi(get_argument_number(1, argu).c_str());
      csetup_repp->set_buffersize(bsize);
      MESSAGE_ITEM mitemb;
      mitemb << "(eca-chainsetup-parser) Setting buffersize to (samples) " << bsize << ".";
      ecadebug->msg(mitemb.to_string()); 
      break;
    }

  case 'B':
    {
      std::string temp = get_argument_number(1, argu);
      if (temp == "auto") {
	csetup_repp->set_buffering_mode(ECA_CHAINSETUP::cs_bmode_auto);
	ecadebug->msg("(eca-chainsetup-parser) Buffering mode is selected automatically.");
      }
      else if (temp == "nonrt") {
	csetup_repp->set_buffering_mode(ECA_CHAINSETUP::cs_bmode_nonrt);
	ecadebug->msg("(eca-chainsetup-parser) Buffering mode 'nonrt' selected.");
      }
      else if (temp == "rt") {
	csetup_repp->set_buffering_mode(ECA_CHAINSETUP::cs_bmode_rt);
	ecadebug->msg("(eca-chainsetup-parser) Buffering mode 'rt' selected.");
      }
      else if (temp == "rtlowlatency") {
	csetup_repp->set_buffering_mode(ECA_CHAINSETUP::cs_bmode_rtlowlatency);
	ecadebug->msg("(eca-chainsetup-parser) Buffering mode 'rtlowlatency' selected.");
      }
      else {
	csetup_repp->set_buffering_mode(ECA_CHAINSETUP::cs_bmode_auto);
	ecadebug->msg("(eca-chainsetup-parser) Unknown buffering mode; 'auto' mode is used instead.");
      }
      break;
    }

  case 'm':      // mixmode
    {
      std::string temp = get_argument_number(1, argu);
      if (temp == "auto") {
	csetup_repp->set_mixmode(ECA_CHAINSETUP::ep_mm_auto);
	ecadebug->msg("(eca-chainsetup-parser) Mix-mode is selected automatically.");
      }
      else if (temp == "mthreaded") {
	ecadebug->msg("(eca-chainsetup-parser) Multithreaded mixmode selected.");
	csetup_repp->set_mixmode(ECA_CHAINSETUP::ep_mm_mthreaded);
      }
      else if (temp == "simple") {
	ecadebug->msg("(eca-chainsetup-parser) Simple mixmode selected.");
	csetup_repp->set_mixmode(ECA_CHAINSETUP::ep_mm_simple);
      }
      else if (temp == "normal") {
	ecadebug->msg("(eca-chainsetup-parser) Normal mixmode selected.");
	csetup_repp->set_mixmode(ECA_CHAINSETUP::ep_mm_normal);
      }
      break;
    }

  case 'n':
    {
      csetup_repp->set_name(get_argument_number(1, argu));
      ecadebug->msg("(eca-chainsetup-parser) Setting chainsetup name to \""
		    + csetup_repp->name() + "\".");
      break;
    }

  case 'r':
    {
      int prio = ::atoi(get_argument_number(1, argu).c_str());
      if (prio < 0) {
	ecadebug->msg("(eca-chainsetup) Raised-priority mode disabled.");
	csetup_repp->toggle_raised_priority(false);
      }
      else {
	if (prio == 0) prio = 50;
	csetup_repp->set_sched_priority(prio);
	ecadebug->msg("(eca-chainsetup) Raised-priority mode enabled. (prio:" + 
		      kvu_numtostr(prio) + ")");
	csetup_repp->toggle_raised_priority(true);
      }
      break;
    }

  case 's':
    {
      if (argu.size() > 2 && argu[2] == 'r') {
	ecadebug->msg("(eca-chainsetup-parser) Option '-sr' is obsolete. Use syntax '-f:sfmt,bits,srate,ileaving' instead.");
      }
      break;
    }

  case 'x':
    {
      ecadebug->msg("(eca-chainsetup-parser) Truncating outputs (overwrite-mode).");
      csetup_repp->set_output_openmode(AUDIO_IO::io_write);
      break;
    }

  case 'X':
    {
      ecadebug->msg("(eca-chainsetup-parser) Updating outputs (rw-mode).");
      csetup_repp->set_output_openmode(AUDIO_IO::io_readwrite);
      break;
    }

  case 'z':
    {
      if (get_argument_number(1, argu) == "db") {
	long int bufs = atol(get_argument_number(2, argu).c_str());
	if (bufs == 0) bufs = 100000;
	csetup_repp->set_double_buffer_size(bufs);
	ecadebug->msg("(eca-chainsetup-parser) Using double-buffer of " + 
		      kvu_numtostr(bufs) + " sample frames.");
	csetup_repp->toggle_double_buffering(true);
      }
      else if (get_argument_number(1, argu) == "nodb") {
	ecadebug->msg("(eca-chainsetup-parser) Double-buffering disabled.");
	csetup_repp->toggle_double_buffering(false);
      }
      else if (get_argument_number(1, argu) == "intbuf") {
	ecadebug->msg("(eca-chainsetup-parser) Enabling extra buffering on realtime devices.");
	csetup_repp->toggle_max_buffers(true);
      }
      else if (get_argument_number(1, argu) == "nointbuf") {
	ecadebug->msg("(eca-chainsetup-parser) Disabling extra buffering on realtime devices.");
	csetup_repp->toggle_max_buffers(false);
      }
      else if (get_argument_number(1, argu) == "multitrack") {
	ecadebug->msg("(eca-chainsetup-parser) Enabling multitrack-mode (override).");
	csetup_repp->multitrack_mode_override_rep = true;
	csetup_repp->multitrack_mode_rep = true;
      }
      else if (get_argument_number(1, argu) == "nomultitrack") {
	ecadebug->msg("(eca-chainsetup-parser) Disabling multitrack-mode (override).");
	csetup_repp->multitrack_mode_override_rep = true;
	csetup_repp->multitrack_mode_rep = false;
      }
      else if (get_argument_number(1, argu) == "psr") {
	ecadebug->msg("(eca-chainsetup-parser) Enabling precise-sample-rates with OSS audio devices.");
	csetup_repp->toggle_precise_sample_rates(true);
      }
      else if (get_argument_number(1, argu) == "nopsr") {
	ecadebug->msg("(eca-chainsetup-parser) Disabling precise-sample-rates with OSS audio devices.");
	csetup_repp->toggle_precise_sample_rates(false);
      }
      else if (get_argument_number(1, argu) == "xruns") {
	ecadebug->msg("(eca-chainsetup-parser) Processing is stopped if an xrun occurs.");
	csetup_repp->toggle_ignore_xruns(false);
      }
      else if (get_argument_number(1, argu) == "noxruns") {
	ecadebug->msg("(eca-chainsetup-parser) Ignoring xruns during processing.");
	csetup_repp->toggle_ignore_xruns(true);
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
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre istatus_rep == false
 */
void ECA_CHAINSETUP_PARSER::interpret_processing_control (const std::string& argu) {
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
	  csetup_repp->length_in_seconds(atof(get_argument_number(1, argu).c_str()));
	  ecadebug->msg("(eca-chainsetup-parser) Set processing time to "
			+ kvu_numtostr(csetup_repp->length_in_seconds_exact()) + ".");
	  break;
	}
	
      case 'l': 
	{
	  csetup_repp->toggle_looping(true);
	  if (csetup_repp->length_set() != true)
	    ecadebug->msg("(eca-chainsetup-parser) Looping enabled. Lenght of input objects will be used to set the loop point.");
	  else
	    ecadebug->msg("(eca-chainsetup-parser) Looping enabled.");
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
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre istatus_rep == false
 */
void ECA_CHAINSETUP_PARSER::interpret_chains (const std::string& argu) {
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
	csetup_repp->select_all_chains();
	ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup-parser) Selected all chains.");
      }
      else {
	csetup_repp->select_chains(schains);
	csetup_repp->add_new_chains(schains);
	MESSAGE_ITEM mtempa;
	mtempa << "(eca-chainsetup-parser) Selected chain ids: ";
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
 * @pre argu.size() > 0
 * @pre  argu[0] == '-'
 * @pre istatus_rep == false
 */
void ECA_CHAINSETUP_PARSER::interpret_audio_format (const std::string& argu) {
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

      csetup_repp->set_default_audio_format(active_sinfo);
      
      MESSAGE_ITEM ftemp;
      ftemp << "(eca-chainsetup-parser) Set active format to (bits/channels/srate/interleave): ";
      ftemp << csetup_repp->default_audio_format().format_string() 
	    << "/" << csetup_repp->default_audio_format().channels() 
	    << "/" << csetup_repp->default_audio_format().samples_per_second();
      if (csetup_repp->default_audio_format().interleaved_channels() == true) {
	ftemp << "/i";
      }
      else { 
	ftemp << "/n";
      }
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
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre istatus_rep == false
 */
void ECA_CHAINSETUP_PARSER::interpret_effect_preset (const std::string& argu) {
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
      ecadebug->msg(ECA_DEBUG::system_objects, "(eca-chainsetup-parser) Interpreting preset \"" + argu + "\".");
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
	  const PRESET* preset = ECA_OBJECT_FACTORY::preset_object(name);
	  if (preset != 0)
	    cop = dynamic_cast<CHAIN_OPERATOR*>(preset->new_expr());
	  else
	    cop = 0;
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
          csetup_repp->add_chain_operator(cop);
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
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 */
void ECA_CHAINSETUP_PARSER::interpret_audioio_device (const std::string& argu) { 
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
  // --------
 
  string tname = get_argument_number(1, argu);

  bool match = true;
  if (argu.size() < 2) return;
  switch(argu[1]) {
  case 'i':
    {
      last_audio_object_repp = ECA_OBJECT_FACTORY::create_audio_object(argu);
      if (last_audio_object_repp == 0) 
	last_audio_object_repp = ECA_OBJECT_FACTORY::create_loop_input(argu, &csetup_repp->loop_map);
      if (last_audio_object_repp != 0) {
	if ((last_audio_object_repp->supported_io_modes() &
	    AUDIO_IO::io_read) != AUDIO_IO::io_read) {
	  interpret_set_result(false, string("(eca-chainsetup-parser) I/O-mode 'io_read' not supported by ") + last_audio_object_repp->name());
	}
	else {
	  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-chainsetup-parser) adding file \"" + tname + "\".");
	  last_audio_object_repp->set_io_mode(AUDIO_IO::io_read);
	  last_audio_object_repp->set_audio_format(csetup_repp->default_audio_format());
	  csetup_repp->add_input(last_audio_object_repp);
	}
      }
      else {
	interpret_set_result(false, string("(eca-chainsetup-parser) Unknown input object type '") + tname + "´.");
      }
      break;
    }

  case 'o':
    {
      last_audio_object_repp = ECA_OBJECT_FACTORY::create_audio_object(argu);
	
      if (last_audio_object_repp == 0) last_audio_object_repp = ECA_OBJECT_FACTORY::create_loop_output(argu, &csetup_repp->loop_map);
      if (last_audio_object_repp != 0) {
	int mode_tmp = csetup_repp->output_openmode();
	if (mode_tmp == AUDIO_IO::io_readwrite) {
	  if ((last_audio_object_repp->supported_io_modes() &
	      AUDIO_IO::io_readwrite) != AUDIO_IO::io_readwrite) {
	    mode_tmp = AUDIO_IO::io_write;
	  }
	}
	if ((last_audio_object_repp->supported_io_modes() &
	    mode_tmp != mode_tmp)) {
	  interpret_set_result(false, string("(eca-chainsetup-parser) I/O-mode 'io_write' not supported by ") + last_audio_object_repp->name());
	}
	else {
	  ecadebug->msg(ECA_DEBUG::system_objects,"(eca-chainsetup-parser) adding file \"" + tname + "\".");
	  last_audio_object_repp->set_io_mode(mode_tmp);
	  last_audio_object_repp->set_audio_format(csetup_repp->default_audio_format());
	  csetup_repp->add_output(last_audio_object_repp);
	}
      }
      else {
	interpret_set_result(false, string("(eca-chainsetup-parser) Format of output ") +
			     tname + " not recognized.");
      }
      break;
    }

  case 'y':
    {
      if (last_audio_object_repp == 0)
	ecadebug->msg("Error! No audio object defined.");

      last_audio_object_repp->seek_position_in_seconds(atof(get_argument_number(1, argu).c_str()));
      if (last_audio_object_repp->io_mode() == AUDIO_IO::io_read) {
	csetup_repp->input_start_pos[csetup_repp->input_start_pos.size() - 1] = last_audio_object_repp->position_in_seconds_exact();
      }
      else {
	csetup_repp->output_start_pos[csetup_repp->output_start_pos.size() - 1] = last_audio_object_repp->position_in_seconds_exact();
      }

      ecadebug->msg("(eca-chainsetup-parser) Set starting position for audio object \""
		    + last_audio_object_repp->label() 
		    + "\": "
		    + kvu_numtostr(last_audio_object_repp->position_in_seconds_exact()) 
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
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 */
void ECA_CHAINSETUP_PARSER::interpret_midi_device (const std::string& argu) { 
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
	    ecadebug->msg(ECA_DEBUG::system_objects,"(eca-chainsetup-parser) MIDI-config: Adding device \"" + tname + "\".");
	    MIDI_IO* mdev = 0;
	    mdev = ECA_OBJECT_FACTORY::create_midi_device(argu);
	    if (mdev != 0) {
	      if ((mdev->supported_io_modes() & MIDI_IO::io_readwrite) == MIDI_IO::io_readwrite) {
		mdev->io_mode(MIDI_IO::io_readwrite);
		csetup_repp->add_midi_device(mdev);
	      }
	      else {
		ecadebug->msg(ECA_DEBUG::info, "(eca-chainsetup-parser) MIDI-config: Warning! I/O-mode 'io_readwrite' not supported by " + mdev->name());
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
			    "(eca-chainsetup-parser) MIDI-config: Receiving MMC messages with id  \"" + 
			    kvu_numtostr(id) +
			    "\".");
	      csetup_repp->midi_server_repp->set_mmc_receive_id(id);
	      break;
	    }
	  

	  case 's': 
	    {
	      int id = atoi(get_argument_number(1, argu).c_str());
	      ecadebug->msg(ECA_DEBUG::info, 
			    "(eca-chainsetup-parser) MIDI-config: Adding MMC-send to device id \"" + 
			    kvu_numtostr(id) +
			    "\".");
	      csetup_repp->midi_server_repp->add_mmc_send_id(id);
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
			    "(eca-chainsetup-parser) MIDI-config: Receiving MIDI-sync.");
	      csetup_repp->midi_server_repp->toggle_midi_sync_receive(true);
	      break;
	    }
	  
	  case 's': 
	    {
	      // FIXME: not implemented
	      ecadebug->msg(ECA_DEBUG::info, 
			    "(eca-chainsetup-parser) MIDI-config: Sending MIDI-sync.");
	      csetup_repp->midi_server_repp->toggle_midi_sync_send(true);
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
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre istatus_rep == false
 */
void ECA_CHAINSETUP_PARSER::interpret_chain_operator (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
  // --------

  CHAIN_OPERATOR* t = ECA_OBJECT_FACTORY::create_chain_operator(argu);
  if (t == 0) t = ECA_OBJECT_FACTORY::create_ladspa_plugin(argu);
  if (t == 0) t = ECA_OBJECT_FACTORY::create_vst_plugin(argu);
  if (t != 0) {
    csetup_repp->add_chain_operator(t);
    istatus_rep = true;
  }
  else 
    interpret_effect_preset(argu);
}

/**
 * Handle controller sources and general controllers.
 *
 * @pre argu.size() > 0
 * @pre argu[0] == '-'
 * @pre istatus_rep == false
 */
void ECA_CHAINSETUP_PARSER::interpret_controller (const std::string& argu) {
  // --------
  DBC_REQUIRE(argu.size() > 0);
  DBC_REQUIRE(argu[0] == '-');
  DBC_REQUIRE(istatus_rep == false);
  // --------

  string prefix = get_argument_prefix(argu);
  if (prefix == "kx") {
    csetup_repp->set_target_to_controller();
    ecadebug->msg(ECA_DEBUG::system_objects, "Selected controllers as parameter control targets.");
    istatus_rep = true;
  }
  else {
    GENERIC_CONTROLLER* t = ECA_OBJECT_FACTORY::create_controller(argu);
    if (t != 0) {
      MIDI_CLIENT* p = dynamic_cast<MIDI_CLIENT*>(t->source_pointer());
      if (p != 0) {
	if (csetup_repp->midi_devices.size() == 0) 
	  interpret_midi_device("-Md:" + csetup_repp->default_midi_device());
	p->register_server(csetup_repp->midi_server_repp);
      }
      csetup_repp->add_controller(t);
      istatus_rep = true;
    }
  }
}

std::string ECA_CHAINSETUP_PARSER::general_options_to_string(void) const {
  MESSAGE_ITEM t;

  int setparams = csetup_repp->override_buffering_parameters().number_of_set();
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(eca-chainsetup-parser) genopts tostring - " + kvu_numtostr(setparams) +
		" overridden parameters.");

  if (setparams > 0) {
    t << "-b:" << csetup_repp->buffersize();
    
    if (csetup_repp->raised_priority() == true)
      t << " -r:" << csetup_repp->sched_priority();
    else
      t << " -r:-1";

    if (csetup_repp->max_buffers() == true) 
      t << " -z:intbuf";
    else
      t << " -z:nointbuf";
    
    if (csetup_repp->double_buffering() == true) 
      t << " -z:db," << csetup_repp->double_buffer_size();
    else
      t << " -z:nodb";
  }
  else {
    switch(csetup_repp->active_buffering_mode_rep) 
      {
      case ECA_CHAINSETUP::cs_bmode_nonrt: 
	{
	  t << "-B:nonrt";
	  break; 
	}
      case ECA_CHAINSETUP::cs_bmode_rt: 
	{ 
	  t << "-B:rt";
	  break; 
	}
      case ECA_CHAINSETUP::cs_bmode_rtlowlatency: 
	{ 
	  t << "-B:rtlowlatency";
	  break;
	}
      default: 
	{ 
	  t << " -B:auto";
	}
      }
  }

  t << " -sr:" << csetup_repp->samples_per_second();

  t << " -n:" << csetup_repp->name();

  switch(csetup_repp->mixmode()) {
  case ECA_CHAINSETUP::ep_mm_simple: {
    t << " -m:simple";
    break;
  }
  case ECA_CHAINSETUP::ep_mm_normal: {
     t << " -m:normal";
    break;
  }
  default: { }
  }

  if (csetup_repp->output_openmode() == AUDIO_IO::io_write) 
    t << " -x";
  else
    t << " -X";


  // FIXME: -z:multitrack, -z:nomultitrack not saved

  if (csetup_repp->ignore_xruns() == true) 
    t << " -z:noxruns";
  else
    t << " -z:xruns";


  if (csetup_repp->precise_sample_rates() == true) 
    t << " -z:psr";
  else
    t << " -z:nopsr";

  t.setprecision(3);
  if (csetup_repp->length_set()) {
    t << " -t:" << csetup_repp->length_in_seconds();
    if (csetup_repp->looping_enabled()) t << " -tl";
  }

  return(t.to_string());
}

std::string ECA_CHAINSETUP_PARSER::midi_to_string(void) const { 
  MESSAGE_ITEM t;
  t.setprecision(3);

  std::vector<MIDI_IO*>::size_type p = 0;
  while (p < csetup_repp->midi_devices.size()) {
    t << "-Md:";
    for(int n = 0; n < csetup_repp->midi_devices[p]->number_of_params(); n++) {
      t << csetup_repp->midi_devices[p]->get_parameter(n + 1);
      if (n + 1 < csetup_repp->midi_devices[p]->number_of_params()) t << ",";
    }
    ++p;
    if (p < csetup_repp->midi_devices.size()) t << " ";
  }

  return(t.to_string());
}

std::string ECA_CHAINSETUP_PARSER::inputs_to_string(void) const { 
  MESSAGE_ITEM t; 
  t.setprecision(3);
  size_t p = 0;
  while (p < csetup_repp->inputs.size()) {
    t << "-a:";
    std::vector<std::string> c = csetup_repp->get_attached_chains_to_input(csetup_repp->inputs[p]);
    std::vector<std::string>::const_iterator cp = c.begin();
    while (cp != c.end()) {
      t << *cp;
      ++cp;
      if (cp != c.end()) t << ",";
    }
    t << " ";
    t << audioio_to_string(csetup_repp->inputs[p], "i");

    if (csetup_repp->input_start_pos[p] != 0) {
      t << " -y:" << csetup_repp->input_start_pos[p];
    }

    ++p;

    if (p < csetup_repp->inputs.size()) t << "\n";
  }

  return(t.to_string());
}

std::string ECA_CHAINSETUP_PARSER::outputs_to_string(void) const { 
  MESSAGE_ITEM t; 
  t.setprecision(3);
  std::vector<AUDIO_IO*>::size_type p = 0;
  while (p < csetup_repp->outputs.size()) {
    t << "-a:";
    std::vector<std::string> c = csetup_repp->get_attached_chains_to_output(csetup_repp->outputs[p]);
    std::vector<std::string>::const_iterator cp = c.begin();
    while (cp != c.end()) {
      t << *cp;
      ++cp;
      if (cp != c.end()) t << ",";
    }
    t << " ";
    t << audioio_to_string(csetup_repp->outputs[p], "o");

    if (csetup_repp->output_start_pos[p] != 0) {
      t << " -y:" << csetup_repp->output_start_pos[p];
    }

    ++p;

    if (p < csetup_repp->outputs.size()) t << "\n";
  }

  return(t.to_string());
}

std::string ECA_CHAINSETUP_PARSER::chains_to_string(void) const { 
  MESSAGE_ITEM t;

  std::vector<CHAIN*>::size_type p = 0;
  while (p < csetup_repp->chains.size()) {
    string tmpstr = csetup_repp->chains[p]->to_string();
    if (tmpstr.size() > 0) {
      t << "-a:" << csetup_repp->chains[p]->name() << " ";
      t << tmpstr;
      if (p + 1 < csetup_repp->chains.size()) t << "\n";
    }
    ++p;
  }

  return(t.to_string());
}

std::string ECA_CHAINSETUP_PARSER::audioio_to_string(const AUDIO_IO* aiod, const std::string& direction) const {
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
