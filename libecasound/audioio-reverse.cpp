// ------------------------------------------------------------------------
// audioio-reverse.cpp: A proxy class that reverts the child 
//                      object's data.
// Copyright (C) 2002,2005,2008 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
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

#include <iostream>

#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#include "audioio-reverse.h"
#include "eca-logger.h"
#include "eca-object-factory.h"
#include "samplebuffer.h"

/**
 * Constructor.
 */
AUDIO_IO_REVERSE::AUDIO_IO_REVERSE (void)
{
  
  tempbuf_repp = new SAMPLE_BUFFER();
  init_rep = false;
  finished_rep = false;
}

/**
 * Destructor.
 */
AUDIO_IO_REVERSE::~AUDIO_IO_REVERSE (void)
{
}

AUDIO_IO_REVERSE* AUDIO_IO_REVERSE::clone(void) const
{
  AUDIO_IO_REVERSE* target = new AUDIO_IO_REVERSE();
  for(int n = 0; n < number_of_params(); n++) {
    target->set_parameter(n + 1, get_parameter(n + 1));
  }
  return target;
}

void AUDIO_IO_REVERSE::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "open " + label() + ".");  

  if (io_mode() != AUDIO_IO::io_read) {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-REVERSE: Reversed writing not supported!"));
  }
  
  if (init_rep != true) {
    AUDIO_IO* tmp = 
      ECA_OBJECT_FACTORY::create_audio_object(
        child_params_as_string(1 + AUDIO_IO_REVERSE::child_parameter_offset, &params_rep));

    if (tmp != 0) {
      set_child(tmp);
    }

    int numparams = child()->number_of_params();
    for(int n = 0; n < numparams; n++) {
      child()->set_parameter(n + 1, get_parameter(n + 1 + AUDIO_IO_REVERSE::child_parameter_offset));
      if (child()->variable_params())
	numparams = child()->number_of_params();
    }

    init_rep = true; /* must be set after dyn. parameters */
  }

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"checking whether child is a finite object");  
    
  pre_child_open();
  child()->open();
  post_child_open();

  if (child()->finite_length_stream() != true) {
    child()->close();
    throw(SETUP_ERROR(SETUP_ERROR::dynamic_params, "AUDIOIO-REVERSE: Unable to reverse an infinite length audio object " + child()->label() + "."));
  }

  if (child()->supports_seeking() != true) {
    child()->close();
    throw(SETUP_ERROR(SETUP_ERROR::dynamic_params, "AUDIOIO-REVERSE: Unable to reverse audio object types that don't support seek (" + child()->label() + ")."));
  }

  AUDIO_IO::open();
}

void AUDIO_IO_REVERSE::close(void)
{
  if (child()->is_open() == true) child()->close();

  AUDIO_IO::close();
}

bool AUDIO_IO_REVERSE::finished(void) const
{
  return finished_rep;
}

string AUDIO_IO_REVERSE::parameter_names(void) const
{
  return string("reverse,") + child()->parameter_names(); 
}

void AUDIO_IO_REVERSE::set_parameter(int param, string value)
{

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"set_parameter " + label() + ".");  

  /* total of n+1 params, where n is number of childobj params */
  if (param > static_cast<int>(params_rep.size())) params_rep.resize(param);

  if (param > 0) {
    params_rep[param - 1] = value;
  }
  
  if (param > AUDIO_IO_REVERSE::child_parameter_offset && init_rep == true) {
    child()->set_parameter(param - AUDIO_IO_REVERSE::child_parameter_offset, value);
  }
}

string AUDIO_IO_REVERSE::get_parameter(int param) const
{

  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"get_parameter " + label() + ".");

  if (param > 0 && param < static_cast<int>(params_rep.size()) + 1) {
    if (param > AUDIO_IO_REVERSE::child_parameter_offset 
	&& init_rep == true) {
      params_rep[param - 1] = 
	child()->get_parameter(param - AUDIO_IO_REVERSE::child_parameter_offset);
    }
    return params_rep[param - 1];
  }

  return ""; 
}

SAMPLE_SPECS::sample_pos_t AUDIO_IO_REVERSE::seek_position(SAMPLE_SPECS::sample_pos_t pos)
{
  finished_rep = false;
  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
		"seek_position " + kvu_numtostr(pos) + ".");
  return AUDIO_IO_PROXY::seek_position(pos);
}

void AUDIO_IO_REVERSE::read_buffer(SAMPLE_BUFFER* sbuf)
{
  tempbuf_repp->number_of_channels(sbuf->number_of_channels());

  /* phase 1: seek to correct position and read one buffer */
  SAMPLE_SPECS::sample_pos_t curpos = position_in_samples();
  SAMPLE_SPECS::sample_pos_t newpos = child()->length_in_samples() - curpos - buffersize();
  if (newpos <= 0) {
    child()->seek_position_in_samples(0);
    int oldbufsize = child()->buffersize();
    child()->set_buffersize(-newpos);
    child()->read_buffer(tempbuf_repp);
    child()->set_buffersize(oldbufsize);
    finished_rep = true;
    DBC_CHECK(-newpos == tempbuf_repp->length_in_samples());
  }
  else {
    child()->seek_position_in_samples(newpos);
    child()->read_buffer(tempbuf_repp);
    DBC_CHECK(buffersize() == tempbuf_repp->length_in_samples());
  }
  curpos += tempbuf_repp->length_in_samples();
  set_position_in_samples(curpos);

  /* phase 2: copy the data reversed from tempbuf
   *          to sbuf */
  sbuf->length_in_samples(tempbuf_repp->length_in_samples());
  for(int c = 0; c < sbuf->number_of_channels(); c++) {
    for(SAMPLE_BUFFER::buf_size_t n = 0; n < sbuf->length_in_samples(); n++) {
      sbuf->buffer[c][n] = tempbuf_repp->buffer[c][sbuf->length_in_samples() - n - 1];
    }
  }
}
