// ------------------------------------------------------------------------
// audioio-mikmod.cpp: Interface class for MikMod input. Uses FIFO pipes.
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

#include <string>
#include <unistd.h>

#include <kvutils/kvu_numtostr.h>

#include "audioio-mikmod.h"
#include "eca-error.h"
#include "eca-debug.h"

string MIKMOD_INTERFACE::default_mikmod_cmd = "mikmod -d stdout -o 16s -q -f %s -p 0 --noloops %f";

void MIKMOD_INTERFACE::set_mikmod_cmd(const string& value) { MIKMOD_INTERFACE::default_mikmod_cmd = value; }

MIKMOD_INTERFACE::MIKMOD_INTERFACE(const string& name) {
  finished_rep = false;
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
}

MIKMOD_INTERFACE::~MIKMOD_INTERFACE(void) { close(); }

void MIKMOD_INTERFACE::open(void) { toggle_open_state(false); }

void MIKMOD_INTERFACE::close(void) {
  if (io_mode() == io_read) {
    kill_mikmod();
  }
  toggle_open_state(false);
}

long int MIKMOD_INTERFACE::read_samples(void* target_buffer, long int samples) {
  if (is_open() == false) fork_mikmod();
  if (wait_for_child() != true) {
    finished_rep = true;
    return(0);
  }
  bytes_read_rep =  ::read(fd_rep, target_buffer, frame_size() * samples);
  if (bytes_read_rep < samples * frame_size() || bytes_read_rep == 0) finished_rep = true;
  else finished_rep = false;
  return(bytes_read_rep / frame_size());
}

void MIKMOD_INTERFACE::seek_position(void) {
  if (is_open() == true) {
    if (io_mode() == io_read) {
      kill_mikmod();
    }
  }
}

void MIKMOD_INTERFACE::kill_mikmod(void) {
  if (is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mikmod) Killing mikmod-child with pid " + kvu_numtostr(pid_of_child()) + ".");
    clean_child();
    toggle_open_state(false);
  }
}

void MIKMOD_INTERFACE::fork_mikmod(void) throw(ECA_ERROR&) {
  if (!is_open()) {
    set_fork_command(MIKMOD_INTERFACE::default_mikmod_cmd);
    set_fork_file_name(label());
    set_fork_sample_rate(samples_per_second());
    fork_child_for_read();
    if (child_fork_succeeded() != true) {
      throw(ECA_ERROR("AUDIOIO-MIKMOD","Can't start mikmod-thread! Check that 'mikmod' is installed properly."));
    }
    fd_rep = file_descriptor();
    toggle_open_state(true);
  }
}
