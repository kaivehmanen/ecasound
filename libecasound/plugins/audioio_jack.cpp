// ------------------------------------------------------------------------
// audioio-jack.cpp: Interface to JACK audio framework
// Copyright (C) 2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <iostream>


#include <jack/jack.h>
#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>

#include "audioio.h"
#include "eca-version.h"
#include "eca-debug.h"

#include "audioio_jack.h"
#include "audioio_jack_manager.h"

static const char* audio_io_keyword_const = "jack_generic";
static const char* audio_io_keyword_regex_const = "(^jack_alsa$)|(^jack_mono$)|(^jack_multi$)|(^jack_generic$)";

AUDIO_IO* audio_io_descriptor(void) { return(new AUDIO_IO_JACK()); }
const char* audio_io_keyword(void){return(audio_io_keyword_const); }
const char* audio_io_keyword_regex(void){return(audio_io_keyword_regex_const); }
int audio_io_interface_version(void) { return(ECASOUND_LIBRARY_VERSION_CURRENT); }


AUDIO_IO_JACK::AUDIO_IO_JACK (void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) constructor");
  
  jackmgr_rep = 0;
  myid_rep = 0;
 
}

AUDIO_IO_JACK::~AUDIO_IO_JACK(void) { 
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) destructor");
}

AUDIO_IO_MANAGER* AUDIO_IO_JACK::create_object_manager(void) const {
  return(new AUDIO_IO_JACK_MANAGER());
}

void AUDIO_IO_JACK::set_manager(AUDIO_IO_JACK_MANAGER* mgr, int id) {
  string mgrname = (mgr == 0 ? mgr->name() : "null");
  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audioio-jack) setting manager to " + mgr->name());
  jackmgr_rep = mgr;
  myid_rep = id;
}

void AUDIO_IO_JACK::open(void) throw(AUDIO_IO::SETUP_ERROR&) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) open");

  curpos_rep = 0;
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_f32_le);
  toggle_interleaved_channels(false);

  if (label() == "jack_mono") {
   set_channels(1);
  }

  if (jackmgr_rep != 0) {
    jackmgr_rep->open(myid_rep);
    jackmgr_rep->register_jack_ports(myid_rep, channels(), jackname_rep);

    if (label() != "jack_generic") {
      if (label() == "jack_alsa") {
	for(int n = 0; n < channels(); n++) {
	  if (io_mode() == AUDIO_IO::io_read) {
	    jackmgr_rep->auto_connect_jack_port(myid_rep, n + 1, "alsa:in_" + kvu_numtostr(n + 1));
	  }
	  else {
	    jackmgr_rep->auto_connect_jack_port(myid_rep, n + 1, "alsa:out_" + kvu_numtostr(n + 1));
	  }
	}
      }
      else if (label() == "jack_multi") {
	for(int n = 0; n < channels(); n++) {
	  jackmgr_rep->auto_connect_jack_port(myid_rep, n + 1, deststring_rep + "_" + kvu_numtostr(n + 1));
	}
      }
      else if (label() == "jack_mono") {
	jackmgr_rep->auto_connect_jack_port(myid_rep, 1, deststring_rep);
      }
    }
  }

  toggle_open_state(true);
}

void AUDIO_IO_JACK::close(void) {
  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) close");

  if (jackmgr_rep != 0) {
    jackmgr_rep->unregister_jack_ports(myid_rep);
    jackmgr_rep->close(myid_rep);
  }

  toggle_open_state(false);
}

long int AUDIO_IO_JACK::read_samples(void* target_buffer, long int samples) {
  if (is_running() == true) {
    if (jackmgr_rep != 0) {
      long int res = jackmgr_rep->read_samples(myid_rep, target_buffer, samples);
      curpos_rep += res;
      return(res);
    }
  }

  return(0);
}

void AUDIO_IO_JACK::write_samples(void* target_buffer, long int samples) { 
  if (is_running() == true) {
    if (jackmgr_rep != 0) {
      curpos_rep += samples;
      return(jackmgr_rep->write_samples(myid_rep, target_buffer, samples));
    }
  }
}


void AUDIO_IO_JACK::stop(void) { 
  AUDIO_IO_DEVICE::stop();

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) stop / " + jackname_rep);

  if (jackmgr_rep != 0) {
    jackmgr_rep->stop(myid_rep);
  }
}

void AUDIO_IO_JACK::start(void) { 
  AUDIO_IO_DEVICE::start();

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) start / " + jackname_rep);

  if (jackmgr_rep != 0) {
    jackmgr_rep->start(myid_rep);
  }
}

void AUDIO_IO_JACK::prepare(void) {
  AUDIO_IO_DEVICE::prepare();

  ecadebug->msg(ECA_DEBUG::system_objects, "(audioio-jack) prepare / " + jackname_rep);
}

SAMPLE_SPECS::sample_pos_t AUDIO_IO_JACK::position_in_samples(void) const {
  return(curpos_rep);
}

std::string AUDIO_IO_JACK::parameter_names(void) const { 
  if (label() == "jack_alsa")
    return("label,portgroup");
  if (label() == "jack_multi")
    return("label,portgroup,client:destgroup");
  if (label() == "jack_mono")
    return("label,portgroup,client:destport");

  return("label,portgroup");
}

void AUDIO_IO_JACK::set_parameter(int param, std::string value) {
  switch(param) 
    {
    case 1: { label(value); break; }
    case 2: { jackname_rep = value; break; }
    case 3: { deststring_rep = value; break; }
    }
}

std::string AUDIO_IO_JACK::get_parameter(int param) const {
  switch(param) 
    {
    case 1: { return(label()); }
    case 2: { return(jackname_rep); }
    case 3: { return(deststring_rep); }
    }  
  return("");
}

