// ------------------------------------------------------------------------
// audioio-ewf.cpp: Ecasound wave format input/output
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

#include <algorithm>
#include <string>
#include <iostream.h>
#include <fstream.h>
#include <cmath>

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "eca-audio-objects.h"
#include "samplebuffer.h"
#include "audioio-ewf.h"

#include "eca-error.h"
#include "eca-debug.h"
  
EWFFILE::~EWFFILE(void) {
  if (is_open()) close();
  write_ewf_data();
  delete child;
}

void EWFFILE::open(void) throw(ECA_ERROR*) {
  child_active = false;

  ewf_rc.resource_file(label());
  read_ewf_data();

  ECA_AUDIO_OBJECTS t;
  child = t.create_audio_object(child_name_rep);
  if (child == 0) 
    throw(new ECA_ERROR("AUDIOIO-EWF", "Couldn't open child object.",ECA_ERROR::retry));

  ecadebug->msg(ECA_DEBUG::user_objects, "AUDIOIO-EWF: Opening ewf-child:" + child->label() + ".");
   
  child->open();

  if (child_length_rep.samples() == 0) child_length_rep = child->length();
  child_offset_rep.set_samples_per_second(child->samples_per_second());
  child_start_pos_rep.set_samples_per_second(child->samples_per_second());
  child_length_rep.set_samples_per_second(child->samples_per_second());

  tmp_buffer.number_of_channels(child->channels());
  tmp_buffer.length_in_samples(child->buffersize());
  tmp_buffer.sample_rate(child->samples_per_second());
 
  seek_position();
  toggle_open_state(true);
}

void EWFFILE::close(void) {
  child->close();
  toggle_open_state(false);
  child_active = false;
}

void EWFFILE::read_buffer(SAMPLE_BUFFER* sbuf) {
  if (child_active == true) {
//      cerr << "Pre:\n";
//      cerr << "Position: " << position_in_samples() << ", ";
//      cerr << "sbuf-length: " << sbuf->length_in_samples() << ", ";
//      cerr << "child-pos: " << child->position_in_samples() << ", ";
//      cerr << "child-offset: " << child_offset_rep.samples() << ", ";
//      cerr << "child-length(): " << child_length_rep.samples() << ".\n";

    if (position_in_samples() + sbuf->length_in_samples() < 
	child_offset_rep.samples() + child_length_rep.samples() - child_start_pos_rep.samples()) {
	// ---
	// case 1: reading child file
	// ---
      child->read_buffer(sbuf);
      position_in_samples_advance(sbuf->length_in_samples());
    }
    else {
	// ---
	// case 2: child file end is reached during this read
	// ---
      if (child_looping_rep == true) {
	// ---
	// case 2a: we're looping
	// ---
	ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-ewf) looping child-object" + label());
//    	cerr << "Looping-pre:\n";
//    	cerr << "Position: " << position_in_samples() << ", ";
//    	cerr << "child: " << child->position_in_samples() << ".\n";
//    	cerr << "child-len: " << child_length_rep.samples() << ".\n";
	child->read_buffer(sbuf);
 	long int extra = child->position_in_samples() - child_length_rep.samples();
	assert(extra < sbuf->length_in_samples());
	child->seek_position_in_samples(child_start_pos_rep.samples() + extra);
	position_in_samples(child_offset_rep.samples() + 
			    child->position_in_samples() - child_start_pos_rep.samples());
	long int save_bsize = child->buffersize();
	child->buffersize(extra, child->samples_per_second());
	child->read_buffer(&tmp_buffer);
	assert(tmp_buffer.length_in_samples() == extra);
	child->buffersize(save_bsize, child->samples_per_second());
	sbuf->copy_range(tmp_buffer, 
			 0,
			 tmp_buffer.length_in_samples(), 
			 sbuf->length_in_samples() - extra);
	position_in_samples(child_offset_rep.samples() + 
			    child->position_in_samples() - child_start_pos_rep.samples());
//    	cerr << "Looping-after:\n";
//    	cerr << "Position: " << position_in_samples() << ", ";
//    	cerr << "child: " << child->position_in_samples() << ".\n";
      }
      else {
	// ---
	// case 2b: no looping
	// ---
//  	cerr << "Looping-pre2:\n";
//  	cerr << "Position: " << position_in_samples() << ", ";
//  	cerr << "child: " << child->position_in_samples() << ".\n";
	child_active = false;
	child->read_buffer(sbuf);
//  	cerr << "Sbuf-length: " << sbuf->length_in_samples() << ", ";
//  	cerr << " child-length(): " <<  child_length_rep.samples() << ".\n";
//  	cerr << "T:";
//  	cerr << sbuf->length_in_samples() -
//  	  child->position_in_samples() +
//  	  child_offset_rep.samples() + 
//  	  child_length_rep.samples() << ".\n";
      
	assert(sbuf->length_in_samples() -
	       child->position_in_samples() +
	       child_offset_rep.samples() + 
	       child_length_rep.samples() >= 0);
	sbuf->make_silent_range(sbuf->length_in_samples() - 
				child->position_in_samples() +
				child_offset_rep.samples() +
				child_length_rep.samples(),
                                sbuf->length_in_samples());
	position_in_samples_advance(sbuf->length_in_samples());
//  	cerr << "Looping-after2:\n";
//  	cerr << "Position: " << position_in_samples() << ", ";
//  	cerr << "child: " << child->position_in_samples() << ".\n";
      }
    }
  }
  else {
    // ---
    // child not active
    // ---

    if (position_in_samples() < child_offset_rep.samples()) {
      // ---
      // case 3: child file start has not been reached
      // ---
      sbuf->make_silent();      
    } 
    else {
      // ---
      // case 4: we're over child start position
      // ---
      if (position_in_samples() < child_offset_rep.samples() + child_length_rep.samples()) {
	ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-ewf) child-object activated" + label());
	child_active = true;
	child->seek_position_in_samples(child_start_pos_rep.samples() +
					position_in_samples()
				      - child_offset_rep.samples());
	child->read_buffer(sbuf);
      }
      else {
	sbuf->make_silent();
      }
    }
    position_in_samples_advance(sbuf->length_in_samples());
  }
}

void EWFFILE::write_buffer(SAMPLE_BUFFER* sbuf) {
  if (child_active == false) {
    child_active = true;
    child_offset_rep.set_samples(position_in_samples());
    MESSAGE_ITEM m;
    m << "(audioio-ewf) found child_offset_rep " << child_offset_rep.seconds() << ".";
    ecadebug->msg(ECA_DEBUG::user_objects, m.to_string());
  }
  
  child->write_buffer(sbuf);
  position_in_samples_advance(sbuf->length_in_samples());
  extend_position();
}

void EWFFILE::seek_position(void) {
  if (is_open()) {
    if (position_in_samples() >= child_offset_rep.samples()) {
      child->seek_position_in_samples(position_in_samples() - child_offset_rep.samples());
    }
    else {
      child_active = false;
      child->seek_first();
    }
  }
}

void EWFFILE::init_default_child(void) throw(ECA_ERROR*) {
  string::const_iterator e = find(label().begin(), label().end(), '.');
  if (e == label().end()) {
    throw(new ECA_ERROR("AUDIOIO-EWF", "Invalid file name; unable to open file.",ECA_ERROR::retry));
  }

  child_name_rep = string(label().begin(), e);
  child_name_rep += ".wav";

  ewf_rc.resource("source", child_name_rep);
}

void EWFFILE::read_ewf_data(void) throw(ECA_ERROR*) {
  if (ewf_rc.has("source"))
    child_name_rep = ewf_rc.resource("source");
  else 
    init_default_child();

  if (ewf_rc.has("offset")) {
    child_offset_rep.set_seconds(atof(ewf_rc.resource("offset").c_str()));
  }
  else
    child_offset_rep.set_samples(0);

  if (ewf_rc.has("start-position")) {
    child_start_pos_rep.set_seconds(atof(ewf_rc.resource("start-position").c_str()));
  }
  else
    child_start_pos_rep.set_samples(0);
    
  if (ewf_rc.has("length")) {
    child_length_rep.set_seconds(atof(ewf_rc.resource("length").c_str()));
  }
  else
    child_length_rep.set_samples(0);

  child_looping_rep = ewf_rc.boolean_resource("looping");
}

void EWFFILE::write_ewf_data(void) {
  ewf_rc.resource("source", child_name_rep);
  if (child_offset_rep.samples() != 0) ewf_rc.resource("offset", kvu_numtostr(child_offset_rep.seconds(),6));
  if (child_start_pos_rep.samples() != 0) ewf_rc.resource("start-position", kvu_numtostr(child_start_pos_rep.seconds(), 6));
  if (child_looping_rep == true) ewf_rc.resource("looping","true");
  if (child_length_rep.samples() != child->length_in_samples()) 
    ewf_rc.resource("length", kvu_numtostr(child_length_rep.seconds(), 6));
}

void EWFFILE::child_offset(const ECA_AUDIO_TIME& v) { child_offset_rep = v; }
void EWFFILE::child_start_position(const ECA_AUDIO_TIME& v) { child_start_pos_rep = v; }
void EWFFILE::child_length(const ECA_AUDIO_TIME& v) { child_length_rep = v; }

bool EWFFILE::finished(void) const {
  if (child->finished() ||
      position_in_samples() > child_offset_rep.samples() +
      child_length_rep.samples())
    return(true);
  return(false);
}

long EWFFILE::length_in_samples(void) const {
  return(child_offset_rep.samples() + child_length_rep.samples());
}
