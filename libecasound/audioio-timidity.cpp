// ------------------------------------------------------------------------
// audioio-timidity.cpp: Interface class for Timidity++ input.
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

#include <kvutils/kvu_numtostr.h>

#include "audioio-timidity.h"
#include "eca-debug.h"

string TIMIDITY_INTERFACE::default_timidity_cmd = "timidity -Or1S -id -s %s -o - %f";

void TIMIDITY_INTERFACE::set_timidity_cmd(const string& value) { TIMIDITY_INTERFACE::default_timidity_cmd = value; }

TIMIDITY_INTERFACE::TIMIDITY_INTERFACE(const string& name) {
  finished_rep = false;
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
}

TIMIDITY_INTERFACE::~TIMIDITY_INTERFACE(void) { close(); }

void TIMIDITY_INTERFACE::open(void) { 
  set_channels(2);
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
  fork_timidity();
  if (wait_for_child() != true) {
    finished_rep = true;
  }
  triggered_rep = false;
  toggle_open_state(true);
}

void TIMIDITY_INTERFACE::close(void) {
  if (io_mode() == io_read) {
    kill_timidity();
  }
  toggle_open_state(false);
}

long int TIMIDITY_INTERFACE::read_samples(void* target_buffer, long int samples) {
  if (triggered_rep != true) triggered_rep = true;
  bytes_read_rep = ::read(fd_rep, target_buffer, frame_size() * samples);
  if (bytes_read_rep < samples * frame_size() || bytes_read_rep == 0) {
    if (position_in_samples() == 0) 
      ecadebug->msg(ECA_DEBUG::info, "(audioio-timidity) Can't start process \"" + TIMIDITY_INTERFACE::default_timidity_cmd + "\". Please check your ~/.ecasoundrc.");
    finished_rep = true;
  }
  else finished_rep = false;
  return(bytes_read_rep / frame_size());
}

void TIMIDITY_INTERFACE::seek_position(void) {
  if (is_open() == true && triggered_rep != true) return;
  if (is_open() == true) {
    if (io_mode() == io_read) {
      kill_timidity();
    }
  }
  fork_timidity();
  if (wait_for_child() != true) {
    finished_rep = true;
  }
}

void TIMIDITY_INTERFACE::kill_timidity(void) {
  if (is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-timidity) Killing Timidity++-child with pid " + kvu_numtostr(pid_of_child()) + ".");
    clean_child();
  }
}

void TIMIDITY_INTERFACE::fork_timidity(void) {
  if (!is_open()) {
    set_fork_command(TIMIDITY_INTERFACE::default_timidity_cmd);
    set_fork_file_name(label());
    set_fork_bits(bits());
    set_fork_channels(channels());
    set_fork_sample_rate(samples_per_second());
    fork_child_for_read();
    if (child_fork_succeeded() == true) {
      fd_rep = file_descriptor();
    }
  }
}
