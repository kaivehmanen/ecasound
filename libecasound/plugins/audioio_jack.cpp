// ------------------------------------------------------------------------
// audioio-jack.cpp: Interface to JACK audio framework
// Copyright (C) 2001-2003 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#include "audioio.h"
#include "eca-version.h"
#include "eca-logger.h"

#include "audioio_jack.h"
#include "audioio_jack_manager.h"

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
/* see eca-static-object-maps.cpp */
static const char* audio_io_keyword_const = "jack";
static const char* audio_io_keyword_regex_const = "(^jack$)|(^jack_alsa$)|(^jack_auto$)|(^jack_generic$)";

AUDIO_IO* audio_io_descriptor(void) { return(new AUDIO_IO_JACK()); }
const char* audio_io_keyword(void){return(audio_io_keyword_const); }
const char* audio_io_keyword_regex(void){return(audio_io_keyword_regex_const); }
int audio_io_interface_version(void) { return(ecasound_library_version_current); }
#endif

AUDIO_IO_JACK::AUDIO_IO_JACK (void)
{
  ECA_LOG_MSG(ECA_LOGGER::functions, "(audioio-jack) constructor");
  
  jackmgr_rep = 0;
  myid_rep = 0;
  secondparam_rep = "";
}

AUDIO_IO_JACK::~AUDIO_IO_JACK(void)
{ 
  if (is_open() == true && is_running()) stop();
  if (is_open() == true) {
    close();
  }
}

AUDIO_IO_MANAGER* AUDIO_IO_JACK::create_object_manager(void) const
{
  return(new AUDIO_IO_JACK_MANAGER());
}

void AUDIO_IO_JACK::set_manager(AUDIO_IO_JACK_MANAGER* mgr, int id)
{
  string mgrname = (mgr == 0 ? mgr->name() : "null");
  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"(audioio-jack) setting manager to " + mgr->name());
  jackmgr_rep = mgr;
  myid_rep = id;
}

void AUDIO_IO_JACK::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack) open");

  set_sample_format(ECA_AUDIO_FORMAT::sfmt_f32_le);
  toggle_interleaved_channels(false);

  if (jackmgr_rep != 0) {
    string my_in_portname ("in"), my_out_portname ("out");

    if (label() == "jack_generic") {
      my_in_portname = my_out_portname = secondparam_rep;
    }

    // FIXME: required?
    // if (workstring.size() == 0) workstring = label();

    jackmgr_rep->open(myid_rep);

    if (jackmgr_rep->is_open() != true) {
      /* unable to open connection to jackd, exit */
      throw(SETUP_ERROR(SETUP_ERROR::unexpected, "AUDIOIO-JACK: Unable to open JACK-client"));
    }

    if (samples_per_second() != jackmgr_rep->samples_per_second()) {
      jackmgr_rep->close(myid_rep);
      throw(SETUP_ERROR(SETUP_ERROR::unexpected, 
			"AUDIOIO-JACK: Cannot connect open connection! Samplerate " +
			kvu_numtostr(samples_per_second()) + " differs from JACK server's sample rate of " + 
			kvu_numtostr(jackmgr_rep->samples_per_second()) + "."));
    }
    
    if (buffersize() != jackmgr_rep->buffersize()) {
      long int jackd_bsize = jackmgr_rep->buffersize();
      jackmgr_rep->close(myid_rep);
      throw(SETUP_ERROR(SETUP_ERROR::unexpected, 
			"AUDIOIO-JACK: Cannot connect open connection! Buffersize " +
			kvu_numtostr(buffersize()) + " differs from JACK server's buffersize of " + 
			kvu_numtostr(jackd_bsize) + "."));
    }

    if (io_mode() == AUDIO_IO::io_read) {
      jackmgr_rep->register_jack_ports(myid_rep, channels(), my_in_portname);
    }
    else {
      jackmgr_rep->register_jack_ports(myid_rep, channels(), my_out_portname);
    }

    if (label() != "jack_generic" && label() != "jack") {
      string in_aconn_portprefix, out_aconn_portprefix;

      if (label() == "jack_alsa") {
	in_aconn_portprefix = "alsa_pcm:capture_";
	out_aconn_portprefix = "alsa_pcm:playback_";
      }
      else if (label() == "jack_auto") {
	in_aconn_portprefix = secondparam_rep + ":out_";
	out_aconn_portprefix = secondparam_rep + ":in_";
      }
      
      for(int n = 0; n < channels(); n++) {
	if (io_mode() == AUDIO_IO::io_read) {
	  jackmgr_rep->auto_connect_jack_port(myid_rep, n + 1, in_aconn_portprefix + kvu_numtostr(n + 1));
	}
	else {
	  jackmgr_rep->auto_connect_jack_port(myid_rep, n + 1, out_aconn_portprefix + kvu_numtostr(n + 1));
	}
      }
    }
  }

  AUDIO_IO_DEVICE::open();
}

void AUDIO_IO_JACK::close(void)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack) close");

  if (jackmgr_rep != 0) {
    jackmgr_rep->unregister_jack_ports(myid_rep);
    jackmgr_rep->close(myid_rep);
  }
  
  AUDIO_IO_DEVICE::close();
}

bool AUDIO_IO_JACK::finished(void) const 
{
  if (is_open() != true ||
      jackmgr_rep == 0 ||
      jackmgr_rep->is_open() != true)
    return(true);

  return(false);
}

long int AUDIO_IO_JACK::read_samples(void* target_buffer, long int samples)
{
  if (jackmgr_rep != 0) {
    long int res = jackmgr_rep->read_samples(myid_rep, target_buffer, samples);
    return(res);
  }
  
  return(0);
}

void AUDIO_IO_JACK::write_samples(void* target_buffer, long int samples)
{
  /* FIXME: case where samples!=buffersize() not handled */

  if (jackmgr_rep != 0) {
    DBC_CHECK(samples == jackmgr_rep->buffersize());
    
    jackmgr_rep->write_samples(myid_rep, target_buffer, samples);
  }
}

void AUDIO_IO_JACK::prepare(void)
{
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack) prepare / " + label());
  AUDIO_IO_DEVICE::prepare();
}

void AUDIO_IO_JACK::start(void)
{ 
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack) start / " + label());
  AUDIO_IO_DEVICE::start();
}

void AUDIO_IO_JACK::stop(void)
{ 
  ECA_LOG_MSG(ECA_LOGGER::system_objects, "(audioio-jack) stop / " + label());
  AUDIO_IO_DEVICE::stop();
}

long int AUDIO_IO_JACK::latency(void) const
{
  return(jackmgr_rep == 0 ? 0 : jackmgr_rep->client_latency(myid_rep));
}

std::string AUDIO_IO_JACK::parameter_names(void) const
{ 
  if (label() == "jack_generic")
    return("label,portname");

  if (label() == "jack_auto")
    return("label,client");

  /* jack, jack_alsa */
  return("label");
}

void AUDIO_IO_JACK::set_parameter(int param, std::string value)
{
  switch(param) 
    {
    case 1: { set_label(value); break; }
    case 2: { secondparam_rep = value; break; }
    }
}

std::string AUDIO_IO_JACK::get_parameter(int param) const
{
  switch(param) 
    {
    case 1: { return(label()); }
    case 2: { return(secondparam_rep); }
    }  

  return("");
}
