// ------------------------------------------------------------------------
// audioio-resample.cpp: A proxy class that resamples the child 
//                       object's data.
// Copyright (C) 2002,2003 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <cmath>   /* ceil(), floor() */
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
  quality_rep = 50;
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

    /* FIXME: add check for real-time devices, resample does _not_
     *        work with them (rt API not proxied properly)
     */
    
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

  if (child_srate_rep == 0) {
    /* query the sampling rate from child object */
    child()->set_io_mode(io_mode());
    child()->open();
    child_srate_rep = child()->samples_per_second();
    child()->close();
  }

  psfactor_rep = 1.0f;
  if (io_mode() == AUDIO_IO::io_read) {
    psfactor_rep = static_cast<float>(samples_per_second()) / child_srate_rep;
    child_buffersize_rep = static_cast<long int>(std::floor(buffersize() * (1.0f / psfactor_rep)));
  }
  else {
    psfactor_rep = static_cast<float>(child_srate_rep) / samples_per_second();
    child_buffersize_rep = static_cast<long int>(std::floor((buffersize() * (1.0f / psfactor_rep)) + 0.5f));
  }

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

    if (param == 1) {
      if (value == "resample-hq") {
	quality_rep = 100;
      }
      else {
	quality_rep = 50;
      }
    }
    else if (param == 2) {
      if (value == "auto") {
	child_srate_rep = 0;
	ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		  "(audioio-resample) resampling with automatic detection of child srate");
      }
      else {
	child_srate_rep = std::atoi(value.c_str());
	ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		  "(audioio-resample) resampling w/ child srate of " + 
		  kvu_numtostr(child_srate_rep));
      }
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

void AUDIO_IO_RESAMPLE::set_audio_format(const ECA_AUDIO_FORMAT& f_str)
{
  AUDIO_IO::set_audio_format(f_str);
  child()->set_audio_format(f_str);
  
  /* set_audio_format() also sets the sample rate so we need to 
     reset the value back to the correct one */
  child()->set_samples_per_second(child_srate_rep);
}

void AUDIO_IO_RESAMPLE::set_samples_per_second(SAMPLE_SPECS::sample_rate_t v)
{
  AUDIO_IO::set_samples_per_second(v);

  /* the child srate is only set in open */
}

void AUDIO_IO_RESAMPLE::read_buffer(SAMPLE_BUFFER* sbuf)
{
  /* FIXME: add temp buffer with preallocated mem area */

  child()->read_buffer(sbuf);

  /* FIXME: not really rt-safe: */
  sbuf->resample_init_memory(child_srate_rep, samples_per_second());

  sbuf->resample_set_quality(quality_rep);
  sbuf->resample(child_srate_rep, samples_per_second());

  /* FIXME: the rabbit-code may sometimes a length of blen-1 */
  // DBC_CHECK(sbuf->length_in_samples() == buffersize());
  //
  // if (sbuf->length_in_samples() != buffersize()) {
  //   std::cerr << "sbuf->length_in_samples()=" << sbuf->length_in_samples() << std::endl;
  //   std::cerr << "buffersize()" << buffersize() << std::endl;
  // }

  change_position_in_samples(sbuf->length_in_samples());
}

void AUDIO_IO_RESAMPLE::write_buffer(SAMPLE_BUFFER* sbuf)
{
  /* FIXME: add temp buffer with preallocated mem area */

  /* FIXME: not really rt-safe: */
  sbuf->resample_init_memory(samples_per_second(), child_srate_rep);
  sbuf->resample_set_quality(quality_rep);
  sbuf->resample(samples_per_second(), child_srate_rep);
  child()->write_buffer(sbuf);
  change_position_in_samples(sbuf->length_in_samples());
}
