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
  fork_ogg123();
  triggered_rep = false;
  toggle_open_state(true);
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
  if (triggered_rep != true) triggered_rep = true;
  bytes_rep = ::fread(target_buffer, 1, frame_size() * samples, f1_rep);
  if (bytes_rep < samples * frame_size() || bytes_rep == 0) {
    if (position_in_samples() == 0) 
      ecadebug->msg(ECA_DEBUG::info, "(audioio-ogg) Can't start process \"" + OGG_VORBIS_INTERFACE::default_ogg_input_cmd + "\". Please check your ~/.ecasoundrc.");
    finished_rep = true;
  }
  else 
    finished_rep = false;

  return(bytes_rep / frame_size());
}

void OGG_VORBIS_INTERFACE::write_samples(void* target_buffer, long int samples) {
  if (triggered_rep != true) triggered_rep = true;
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
  if (is_open() == true && triggered_rep != true) return;
  if (is_open() == true) {
    finished_rep = false;
    if (io_mode() == io_read) {
      kill_ogg123();
    }
    else {
      kill_vorbize();
    }
  }
  if (io_mode() == io_read) {
    fork_ogg123();
  }
  else
    fork_vorbize();
}

void OGG_VORBIS_INTERFACE::kill_ogg123(void) {
  if (is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-ogg) Killing ogg123-child." + kvu_numtostr(pid_of_child()) + ".");
    clean_child();
    fclose(f1_rep);
  }
}

void OGG_VORBIS_INTERFACE::fork_ogg123(void) {
  ecadebug->msg(ECA_DEBUG::user_objects, OGG_VORBIS_INTERFACE::default_ogg_input_cmd);
  set_fork_command(OGG_VORBIS_INTERFACE::default_ogg_input_cmd);
  set_fork_file_name(label());
  set_fork_pipe_name();
  fork_child_for_read();
  if (child_fork_succeeded() == true) {
    fd_rep = file_descriptor();
    f1_rep = fdopen(fd_rep, "r");
    if (f1_rep == 0) finished_rep = true;
  }
}

void OGG_VORBIS_INTERFACE::kill_vorbize(void) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-ogg) Killing vorbize-child with pid " + kvu_numtostr(pid_of_child()) + ".");
  clean_child();
}

void OGG_VORBIS_INTERFACE::fork_vorbize(void) {
  ecadebug->msg("(audioio-ogg) Starting to encode " + label() + " with vorbize.");
  string command_rep = OGG_VORBIS_INTERFACE::default_ogg_output_cmd;
  if (command_rep.find("%f") != string::npos) {
    command_rep.replace(command_rep.find("%f"), 2, label());
  }
  set_fork_command(command_rep);
  set_fork_file_name(label());
  fork_child_for_write();
  if (child_fork_succeeded() == true) {
    fd_rep = file_descriptor();
  }
}
