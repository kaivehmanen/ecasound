// ------------------------------------------------------------------------
// audioio-mikmod.cpp: Interface class for MikMod input. Uses FIFO pipes.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <cmath>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#include <unistd.h>

#include <kvutils.h>

#include "samplebuffer.h"
#include "audioio.h"
#include "audioio-mikmod.h"

#include "eca-error.h"
#include "eca-debug.h"

string MIKMOD_INTERFACE::default_mikmod_path = "mikmod";
string MIKMOD_INTERFACE::default_mikmod_args = "-p 0 --noloops";

void MIKMOD_INTERFACE::set_mikmod_path(const string& value) { MIKMOD_INTERFACE::default_mikmod_path = value; }
void MIKMOD_INTERFACE::set_mikmod_args(const string& value) { MIKMOD_INTERFACE::default_mikmod_args = value; }

MIKMOD_INTERFACE::MIKMOD_INTERFACE(const string& name, const SIMODE mode, const ECA_AUDIO_FORMAT& fmt) 
  :  AUDIO_IO_FILE(name, mode, fmt) 
{
  toggle_open_state(false);
  finished_rep = false;
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
}

MIKMOD_INTERFACE::~MIKMOD_INTERFACE(void) { close(); }

void MIKMOD_INTERFACE::open(void) { }

void MIKMOD_INTERFACE::close(void) {
  if (io_mode() == si_read) {
    kill_mikmod();
  }
  toggle_open_state(false);
}

long int MIKMOD_INTERFACE::read_samples(void* target_buffer, long int samples) {
  if (is_open() == false) fork_mikmod();
  if (waitpid(pid_of_child, 0, WNOHANG) < 0) { 
    finished_rep = true;
    return(0);
  }
  bytes_read =  ::read(fd, target_buffer, frame_size() * samples);
  if (bytes_read < samples * frame_size() || bytes_read == 0) finished_rep = true;
  else finished_rep = false;
  return(bytes_read / frame_size());
}

void MIKMOD_INTERFACE::seek_position(void) {
  if (is_open() == true) {
    if (io_mode() == si_read) {
      kill_mikmod();
    }
  }
}

void MIKMOD_INTERFACE::kill_mikmod(void) {
  if (is_open()) {
    ecadebug->msg(1, "(audioio-mikmod) Killing mikmod-child with pid " + kvu_numtostr(pid_of_child) + ".");
    kill(pid_of_child, SIGKILL);
    waitpid(pid_of_child, 0, 0);
    ::close(fd);
    toggle_open_state(false);
  }
}

void MIKMOD_INTERFACE::fork_mikmod(void) {
  if (!is_open()) {
    string komen = MIKMOD_INTERFACE::default_mikmod_path;
    komen += " -d stdout -o 16s -q -f " + 
                   kvu_numtostr(samples_per_second()) +
                   + " " + MIKMOD_INTERFACE::default_mikmod_args +
                   " " + label();
    ecadebug->msg(2,komen);
   
    int fpipes[2];
    if (pipe(fpipes) == 0) {
      pid_of_child = fork();
      if (pid_of_child == -1) { 
	// ---
	// error
	// ---
	throw(new ECA_ERROR("AUDIOIO-MIKMOD","Can't start mikmod-child!"));
      }
      else if (pid_of_child == 0) { 
	// ---
	// child 
	// ---
	::close(1);
	dup2(fpipes[1], 1);
	::close(fpipes[0]);
	::close(fpipes[1]);
	freopen("/dev/null", "w", stderr);
	vector<string> temp = string_to_words(komen);
	const char* args[temp.size() + 1];
	// = new char* [temp.size() + 1];
	vector<string>::size_type p = 0;
	while(p < temp.size()) {
	  args[p] = temp[p].c_str();
	  ++p;
	}
	args[p] = 0;
	execvp(temp[0].c_str(), const_cast<char**>(args));
	::close(1);
	exit(0);
	cerr << "You shouln't see this!\n";
      }
      else { 
	// ---
	// parent
	// ---
	::close(fpipes[1]);
	fd = fpipes[0];
	ecadebug->msg("(audioio-mikmod) Forked mikmod-child with pid " + kvu_numtostr(pid_of_child) + ".");
	toggle_open_state(true);
      }
    }
    else 
      throw(new ECA_ERROR("AUDIOIO-MIKMOD","Can't start mikmod-child! Check that 'mikmod' is installed properly."));
  }
}

