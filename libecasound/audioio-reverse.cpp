// ------------------------------------------------------------------------
// audioio-reverse.cpp: A proxy class that reverts the child 
//                      object's data.
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

#include <iostream>

#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>

#include "audioio-null.h"
#include "audioio-reverse.h"
#include "eca-debug.h"
#include "eca-object-factory.h"
#include "samplebuffer.h"

/**
 * Constructor.
 */
AUDIO_IO_REVERSE::AUDIO_IO_REVERSE (void)
{
  
  child_repp = new NULLFILE("uninitialized");
  tempbuf_repp = new SAMPLE_BUFFER();
  init_rep = false;
  finished_rep = false;
}

/**
 * Destructor.
 */
AUDIO_IO_REVERSE::~AUDIO_IO_REVERSE (void)
{
  delete child_repp; // either null or the actual child object
}

void AUDIO_IO_REVERSE::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-reverse) open " + label() + ".");  

  if (io_mode() != AUDIO_IO::io_read) {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-REVERSE: Reversed writing not supported!"));
  }
  
  if (init_rep != true) {
    AUDIO_IO* tmp = 0;
    if (params_rep.size() > 1) {
      /* 2nd-param: audio object name */
      string& type = params_rep[1];
      if (type.size() > 0) {
	tmp = ECA_OBJECT_FACTORY::create_audio_object(type);
      }
    }
    
    if (tmp != 0) {
      delete child_repp; // the placeholder null object
      child_repp = tmp;
    }

    int numparams = child_repp->number_of_params();
    for(int n = 0; n < numparams; n++) {
      child_repp->set_parameter(n + 1, get_parameter(n + 2));
      numparams = child_repp->number_of_params(); // in case 'n_o_p()' varies
    }

    init_rep = true; /* must be set after dyn. parameters */
  }
    
  if (child_repp->finite_length_stream() != true) {
    throw(SETUP_ERROR(SETUP_ERROR::dynamic_params, "AUDIOIO-REVERSE: Unable to reverse an infinite length audio object " + child_repp->label() + "."));
  }

  if (child_repp->supports_seeking() != true) {
    throw(SETUP_ERROR(SETUP_ERROR::dynamic_params, "AUDIOIO-REVERSE: Unable to reverse audio object types that don't support seek (" + child_repp->label() + ")."));
  }
  
  child_repp->set_buffersize(buffersize());
  child_repp->set_io_mode(io_mode());
  child_repp->set_audio_format(audio_format());
  child_repp->open();
  if (child_repp->locked_audio_format() == true) {
    set_audio_format(child_repp->audio_format());
  }
  set_label(child_repp->label());

  AUDIO_IO::open();
}

void AUDIO_IO_REVERSE::close(void)
{
  if (child_repp->is_open() == true) child_repp->close();

  AUDIO_IO::close();
}

bool AUDIO_IO_REVERSE::finished(void) const
{
  return(finished_rep);
}

string AUDIO_IO_REVERSE::parameter_names(void) const
{
  return(string("reverse,") + child_repp->parameter_names()); 
}

void AUDIO_IO_REVERSE::set_parameter(int param, string value)
{

  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-reverse) set_parameter " + label() + ".");  

  /* total of n+1 params, where n is number of childobj params */
  if (param > static_cast<int>(params_rep.size())) params_rep.resize(param);

  if (param > 0) {
    params_rep[param - 1] = value;
  }
  
  if (param > 1 && init_rep == true) {
    child_repp->set_parameter(param - 1, value);
  }
}

string AUDIO_IO_REVERSE::get_parameter(int param) const
{

  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-reverse) get_parameter " + label() + ".");

  if (param > 0 && param < static_cast<int>(params_rep.size()) + 1) {
    if (param > 1 && init_rep == true) {
      params_rep[param - 1] = child_repp->get_parameter(param - 1);
    }
    return(params_rep[param - 1]);
  }

  return(""); 
}

void AUDIO_IO_REVERSE::seek_position(void)
{
  finished_rep = false;
  ecadebug->msg(ECA_DEBUG::user_objects, 
		"(audioio-reverse) seek_position " + kvu_numtostr(position_in_samples()) + ".");
}

void AUDIO_IO_REVERSE::read_buffer(SAMPLE_BUFFER* sbuf)
{
  tempbuf_repp->number_of_channels(sbuf->number_of_channels());

  /* phase 1: seek to correct position and read one buffer */
  SAMPLE_SPECS::sample_pos_t curpos = position_in_samples();
  SAMPLE_SPECS::sample_pos_t newpos = child_repp->length_in_samples() - curpos - buffersize();
  if (newpos <= 0) {
    child_repp->seek_position_in_samples(0);
    int oldbufsize = child_repp->buffersize();
    child_repp->set_buffersize(-newpos);
    child_repp->read_buffer(tempbuf_repp);
    child_repp->set_buffersize(oldbufsize);
    finished_rep = true;
    DBC_CHECK(-newpos == tempbuf_repp->length_in_samples());
  }
  else {
    child_repp->seek_position_in_samples(newpos);
    child_repp->read_buffer(tempbuf_repp);
    DBC_CHECK(buffersize() == tempbuf_repp->length_in_samples());
  }
  curpos += tempbuf_repp->length_in_samples();
  position_in_samples(curpos);

  /* phase 2: copy the data reversed from tempbuf
   *          to sbuf */
  sbuf->length_in_samples(tempbuf_repp->length_in_samples());
  for(int c = 0; c < sbuf->number_of_channels(); c++) {
    for(SAMPLE_BUFFER::buf_size_t n = 0; n < sbuf->length_in_samples(); n++) {
      sbuf->buffer[c][n] = tempbuf_repp->buffer[c][sbuf->length_in_samples() - n];
    }
  }
}
