// ------------------------------------------------------------------------
// audioio-ogg.cpp: Interface for ogg vorbis ecoders and encoders.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <string>
#include <unistd.h>

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "audioio-ogg.h"

#include "eca-error.h"
#include "eca-debug.h"

string OGG_VORBIS_INTERFACE::default_ogg_input_cmd = "ogg123 -d wav -o file:%F %f";
string OGG_VORBIS_INTERFACE::default_ogg_output_cmd = "vorbize --raw --write=%f";

void OGG_VORBIS_INTERFACE::set_ogg_input_cmd(const string& value) { OGG_VORBIS_INTERFACE::default_ogg_input_cmd = value; }
void OGG_VORBIS_INTERFACE::set_ogg_output_cmd(const string& value) { OGG_VORBIS_INTERFACE::default_ogg_output_cmd = value; }

OGG_VORBIS_INTERFACE::OGG_VORBIS_INTERFACE(const string& name) {
  label(name);
  finished_rep = false;
  toggle_open_state(false);
}

OGG_VORBIS_INTERFACE::~OGG_VORBIS_INTERFACE(void) { close(); }

void OGG_VORBIS_INTERFACE::open(void) { 

}

void OGG_VORBIS_INTERFACE::close(void) {
  if (io_mode() == io_read) {
    kill_ogg123();
  }
  else {
    kill_vorbize();
  }
  toggle_open_state(false);
}

long int OGG_VORBIS_INTERFACE::read_samples(void* target_buffer, long int samples) {
  if (is_open() != true) fork_ogg123();
  if (wait_for_child() != true) {
    finished_rep = true;
    return(0);
  }
  bytes_rep = ::fread(target_buffer, 1, frame_size() * samples, f1_rep);
  if (bytes_rep < samples * frame_size() || bytes_rep == 0)
    finished_rep = true;
  else 
    finished_rep = false;

  return(bytes_rep / frame_size());
}

void OGG_VORBIS_INTERFACE::write_samples(void* target_buffer, long int samples) {
  if (is_open() == false) fork_vorbize();
  if (wait_for_child() != true) {
    finished_rep = true;
  }
  else {
    bytes_rep = ::write(fd_rep, target_buffer, frame_size() * samples);
    if (bytes_rep < frame_size() * samples || bytes_rep == 0) finished_rep = true;
    else finished_rep = false;
  }
}

void OGG_VORBIS_INTERFACE::seek_position(void) {
  if (is_open() == true) {
    finished_rep = false;
    if (io_mode() == io_read) {
      kill_ogg123();
    }
    else {
      kill_vorbize();
    }
  }
}

void OGG_VORBIS_INTERFACE::kill_ogg123(void) {
  if (is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-ogg) Killing ogg123-child." + kvu_numtostr(pid_of_child()) + ".");
    clean_child();
    fclose(f1_rep);
    toggle_open_state(false);
  }
}

void OGG_VORBIS_INTERFACE::fork_ogg123(void) throw(ECA_ERROR&) {
  if (!is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, OGG_VORBIS_INTERFACE::default_ogg_input_cmd);
    set_fork_command(OGG_VORBIS_INTERFACE::default_ogg_input_cmd);
    set_fork_file_name(label());
    set_fork_pipe_name();
    fork_child_for_read();
    if (child_fork_succeeded() != true) {
      throw(ECA_ERROR("ECA-OGG","Can't start ogg123-thread! Check that 'ogg123' is installed properly."));
    }
    fd_rep = file_descriptor();
    f1_rep = fdopen(fd_rep, "r");
    toggle_open_state(true);
  }
}

void OGG_VORBIS_INTERFACE::kill_vorbize(void) {
  if (is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-ogg) Killing vorbize-child with pid " + kvu_numtostr(pid_of_child()) + ".");
    clean_child();
    toggle_open_state(false);
  }
}

void OGG_VORBIS_INTERFACE::fork_vorbize(void) throw(ECA_ERROR&) {
  if (!is_open()) {
    ecadebug->msg("(audioio-ogg) Starting to encode " + label() + " with vorbize.");
    set_fork_command(OGG_VORBIS_INTERFACE::default_ogg_output_cmd);
    set_fork_file_name(label());
    fork_child_for_write();
    if (child_fork_succeeded() != true) {
      throw(ECA_ERROR("AUDIOIO-OGG","Can't start vorbize-thread! Check that 'vorbize' is installed properly."));
    }
    fd_rep = file_descriptor();
    toggle_open_state(true);
  }
}
