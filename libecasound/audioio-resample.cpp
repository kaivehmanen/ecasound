// ------------------------------------------------------------------------
// audioio-resample.cpp: A proxy class that resamples the child 
//                       object's data.
// Copyright (C) 2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <cstdlib> /* atoi() */
#include <iostream>

#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#include "audioio-resample.h"
#include "eca-logger.h"
#include "eca-object-factory.h"
#include "samplebuffer.h"

/**
 * Constructor.
 */
AUDIO_IO_RESAMPLE::AUDIO_IO_RESAMPLE (void)
{
  init_rep = false;
}

/**
 * Destructor.
 */
AUDIO_IO_RESAMPLE::~AUDIO_IO_RESAMPLE (void)
{
}

AUDIO_IO_RESAMPLE* AUDIO_IO_RESAMPLE::clone(void) const
{
  AUDIO_IO_RESAMPLE* target = new AUDIO_IO_RESAMPLE();
  for(int n = 0; n < number_of_params(); n++) {
    target->set_parameter(n + 1, get_parameter(n + 1));
  }
  return(target);
}

void AUDIO_IO_RESAMPLE::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-resample) open " + label() + ".");  

  if (init_rep != true) {
    AUDIO_IO* tmp = 0;
    if (params_rep.size() > 1) {
      /* 2nd-param: audio object name */
      string& type = params_rep[2];
      if (type.size() > 0) {
	tmp = ECA_OBJECT_FACTORY::create_audio_object(type);
      }
    }
    
    if (tmp != 0) {
      set_child(tmp);
    }

    int numparams = child()->number_of_params();
    for(int n = 0; n < numparams; n++) {
      child()->set_parameter(n + 1, get_parameter(n + 3));
      numparams = child()->number_of_params(); // in case 'n_o_p()' varies
    }

    init_rep = true; /* must be set after dyn. parameters */
  }

  psfactor_rep = 1.0f;
  if (io_mode() == AUDIO_IO::io_read) {
    psfactor_rep = static_cast<float>(samples_per_second()) / child_srate_rep;
  }
  else {
    psfactor_rep = static_cast<float>(child_srate_rep) / samples_per_second();
  }

  child_buffersize_rep = static_cast<long int>(buffersize() * (1.0f / psfactor_rep));

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
	      "(audioio-resample) open(); psfactor=" + kvu_numtostr(psfactor_rep) +
	      ", child_srate=" + kvu_numtostr(child_srate_rep) +
	      ", srate=" + kvu_numtostr(samples_per_second()) +
	      ", bsize=" + kvu_numtostr(child_buffersize_rep) + ".");
    
  /* note, we don't use pre_child_open() as 
   * we want to set srate differently */
  child()->set_buffersize(child_buffersize_rep);
  child()->set_io_mode(io_mode());
  child()->set_audio_format(audio_format());
  child()->set_samples_per_second(child_srate_rep);

  child()->open();

  /* same for the post processing */ 
  SAMPLE_SPECS::sample_rate_t orig_srate = samples_per_second();
  if (child()->locked_audio_format() == true) {
    set_audio_format(child()->audio_format());
    set_samples_per_second(orig_srate);
  }

  set_label(child()->label());
  set_length_in_samples(child()->length_in_samples());

  AUDIO_IO_PROXY::open();
}

void AUDIO_IO_RESAMPLE::close(void)
{
  if (child()->is_open() == true) child()->close();

  AUDIO_IO_PROXY::close();
}

bool AUDIO_IO_RESAMPLE::finished(void) const
{
  return(child()->finished());
}

string AUDIO_IO_RESAMPLE::parameter_names(void) const
{
  return(string("resample,srate,") + child()->parameter_names()); 
}

void AUDIO_IO_RESAMPLE::set_parameter(int param, string value)
{

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
	      "(audioio-resample) set_parameter " + label() +
	      " to value " + value + ".");  

  /* total of n+1 params, where n is number of childobj params */
  if (param > static_cast<int>(params_rep.size())) params_rep.resize(param);

  if (param > 0) {
    params_rep[param - 1] = value;
    if (param == 2) {
      child_srate_rep = std::atoi(value.c_str());
      ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		  "(audioio-resample) resampling w/ child srate of " + 
		  kvu_numtostr(child_srate_rep));
    }
  }
  
  if (param > 2 && init_rep == true) {
    child()->set_parameter(param - 2, value);
  }
}

string AUDIO_IO_RESAMPLE::get_parameter(int param) const
{

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"(audioio-resample) get_parameter " + label() + ".");

  if (param > 0 && param < static_cast<int>(params_rep.size()) + 1) {
    if (param > 2 && init_rep == true) {
      params_rep[param - 1] = child()->get_parameter(param - 2);
    }
    return(params_rep[param - 1]);
  }

  return(""); 
}

void AUDIO_IO_RESAMPLE::seek_position(void)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"(audioio-resample) seek_position " + kvu_numtostr(position_in_samples()) + ".");
  child()->seek_position_in_samples(position_in_samples());

  AUDIO_IO_PROXY::seek_position();
}

void AUDIO_IO_RESAMPLE::read_buffer(SAMPLE_BUFFER* sbuf)
{
  // std::cerr << "pre-pre-resample: " << child_buffersize_rep << " samples.\n";
  child()->read_buffer(sbuf);
  /* FIXME: not really rt-safe: */
  sbuf->resample_init_memory(child_srate_rep, samples_per_second());
  // std::cerr << "pre-resample: " << sbuf->length_in_samples() << " samples.\n";
  sbuf->resample(child_srate_rep, samples_per_second());
  // std::cerr << "post-resample: " << sbuf->length_in_samples() << " samples.\n";
  change_position_in_samples(sbuf->length_in_samples());
}

void AUDIO_IO_RESAMPLE::write_buffer(SAMPLE_BUFFER* sbuf)
{
  /* FIXME: not really rt-safe: */
  sbuf->resample_init_memory(samples_per_second(), child_srate_rep);
  sbuf->resample(samples_per_second(), child_srate_rep);
  child()->write_buffer(sbuf);
  change_position_in_samples(sbuf->length_in_samples());
}
