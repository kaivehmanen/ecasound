// ------------------------------------------------------------------------
// audioio-mp3.cpp: Interface to mpg123 (input) and lame (output).
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

#include "audioio-mp3.h"
#include "audioio-mp3_impl.h"
#include "samplebuffer.h"
#include "audioio.h"

#include "eca-error.h"
#include "eca-debug.h"

FILE* audioio_mp3_pipe;

string MP3FILE::default_mpg123_path = "mpg123";
string MP3FILE::default_mpg123_args = "-b 0";
string MP3FILE::default_lame_path = "lame";
string MP3FILE::default_lame_args = "-b 128";

void MP3FILE::set_mpg123_path(const string& value) { MP3FILE::default_mpg123_path = value; }
void MP3FILE::set_mpg123_args(const string& value) { MP3FILE::default_mpg123_args = value; }
void MP3FILE::set_lame_path(const string& value) { MP3FILE::default_lame_path = value; }
void MP3FILE::set_lame_args(const string& value) { MP3FILE::default_lame_args = value; }

MP3FILE::MP3FILE(const string& name, const SIMODE mode, const ECA_AUDIO_FORMAT& fmt) 
  :  AUDIO_IO_FILE(name, mode, fmt) 
{
  toggle_open_state(false);
  finished_rep = false;
  if (io_mode() == si_read) {
    get_mp3_params(label());
  }
}

MP3FILE::~MP3FILE(void) { close(); }

void MP3FILE::open(void) { }

void MP3FILE::close(void) {
  if (io_mode() == si_read) {
    kill_mpg123();
  }
  else {
    kill_lame();
  }
  toggle_open_state(false);
}

long int MP3FILE::read_samples(void* target_buffer, long int samples) {
  if (is_open() == false) fork_mpg123();
//    if (waitpid(-1, 0, WNOHANG) < 0) { 
//      finished_rep = true;
//      return(0);
//    }
  bytes =  ::read(fd, target_buffer, frame_size() * samples);
  if (bytes < samples * frame_size() || bytes == 0) finished_rep = true;
  else finished_rep = false;
  return(bytes / frame_size());
}

void MP3FILE::write_samples(void* target_buffer, long int samples) {
  if (is_open() == false) fork_lame();
  if (waitpid(pid_of_child, 0, WNOHANG) < 0) { 
    finished_rep = true;
  }
  else {
    bytes = ::write(fd, target_buffer, frame_size() * samples);
    if (bytes < frame_size() * samples || bytes == 0) finished_rep = true;
    else finished_rep = false;
  }
}

void MP3FILE::seek_position(void) {
  if (is_open() == true) {
    finished_rep = false;
    if (io_mode() == si_read) {
      kill_mpg123();
    }
    else {
      kill_lame();
    }
  }
}

void MP3FILE::get_mp3_params(const string& fname) throw(ECA_ERROR*) {
  Layer newlayer;
  FILE *temp;
  temp = fopen(fname.c_str(),"r");
  if (!temp)
    throw(new ECA_ERROR("ECA-MP3","Unable to open temp file " + fname, ECA_ERROR::retry));
  newlayer.get(temp);
  fseek(temp, 0, SEEK_END);
  double fsize = (double)ftell(temp);
  double bitrate = ((double)newlayer.bitrate() * 1000.0);
  double bsecond = (double)bytes_per_second();
  MESSAGE_ITEM m;
  m << "MP3 file size: " << fsize << "\n.";
  m << "MP3 length_value: " << newlayer.length() << "\n.";
  m << "bsecond: " << bsecond << "\n.";
  m << "bitrate: " << bitrate << "\n.";
  length_in_samples((long)ceil(8.0 * fsize / bitrate * bsecond / frame_size()));

  m << "Setting MP3 length_value: " << length_in_seconds() << "\n.";
  pcm = newlayer.pcmPerFrame();

  m << "MP3 pcm value: " << pcm << ".";
  ecadebug->msg(4,m.to_string());
  fclose(temp);
}

void MP3FILE::kill_mpg123(void) {
  if (is_open()) {
    ecadebug->msg(1, "(audioio-mp3) Killing mpg123-child with pid " + kvu_numtostr(pid_of_child) + ".");
    kill(pid_of_child, SIGKILL);
    waitpid(pid_of_child, 0, 0);
    ::close(fd);
    toggle_open_state(false);
  }
}

void MP3FILE::fork_mpg123(void) throw(ECA_ERROR*) {
  if (!is_open()) {
    
    string komen = MP3FILE::default_mpg123_path;
    komen += " ";
    komen += MP3FILE::default_mpg123_args;
    komen += " -q -s -k ";
    komen += kvu_numtostr((long)(position_in_samples() / pcm));
    komen += " " + label();
    //    komen += " '" + label() + "' 2>/dev/null";
    ecadebug->msg(2,komen);
   
    int fpipes[2];
    if (pipe(fpipes) == 0) {
      pid_of_child = fork();
      if (pid_of_child == -1) { 
	// ---
	// error
	// ---
	throw(new ECA_ERROR("ECA-MP3","Can't start mpg123-thread!"));
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
	//	fobject = fdopen(fpipes[0], "r");
	::close(fpipes[1]);
	fd = fpipes[0];
	//	if (fobject == NULL) {
	ecadebug->msg("(audioio-mp3) Forked mpg123-child with pid " + kvu_numtostr(pid_of_child) + ".");
	toggle_open_state(true);
      }
    }
    else 
      throw(new ECA_ERROR("ECA-MP3","Can't start mpg123-thread! Check that 'mpg123' is installed properly."));
  }
}

void MP3FILE::kill_lame(void) {
  if (is_open()) {
    ecadebug->msg(1, "(audioio-mp3) Killing lame-child with pid " + kvu_numtostr(pid_of_child) + ".");
    kill(pid_of_child, SIGKILL);
    waitpid(pid_of_child, 0, 0);
    ::close(fd);
    toggle_open_state(false);
  }
}

void MP3FILE::fork_lame(void) throw(ECA_ERROR*) {
  if (!is_open()) {
    
    string komen = MP3FILE::default_lame_path;
    komen += " ";
    komen += MP3FILE::default_lame_args;
    komen += " -x -S - " + label();
    // "'"-  komen << label() << "' 2>/dev/null";
    ecadebug->msg("(audioio-mp3) Starting to encode " + label() + " with lame.");
    ecadebug->msg(2,komen);

    int fpipes[2];
    if (pipe(fpipes) == 0) {
      pid_of_child = fork();
      if (pid_of_child == -1) { 
	// ---
	// error
	// ---
	throw(new ECA_ERROR("ECA-MP3","Can't start lame-child!"));
      }
      else if (pid_of_child == 0) { 
	// ---
	// child 
	// ---
	::close(0);
	::dup2(fpipes[0],0);
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
	::close(0);
	exit(0);
	cerr << "You shouln't see this!\n";
      }
      else { 
	// ---
	// parent
	// ---
	::close(fpipes[0]);
	fd = fpipes[1];

	ecadebug->msg("(audioio-mp3) Forked lame-child with pid " + kvu_numtostr(pid_of_child) + ".");
	toggle_open_state(true);
      }
    }
    else 
      throw(new ECA_ERROR("ECA-MP3","Can't start lame-child! Check that 'lame' is installed properly."));
  }
}
