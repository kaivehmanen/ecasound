// ------------------------------------------------------------------------
// audioio-mp3.cpp: Interface for mp3 decoders and encoders that support 
//                  input/output using standard streams. Defaults to
//                  mpg123 and lame.
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

#include <cmath>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#include <unistd.h>

#include <kvutils/message_item.h>
#include <kvutils/kvu_numtostr.h>

#include "audioio-mp3.h"
#include "audioio-mp3_impl.h"
#include "samplebuffer.h"
#include "audioio.h"

#include "eca-error.h"
#include "eca-debug.h"

FILE* audioio_mp3_pipe;

string MP3FILE::default_mp3_input_cmd = "mpg123 -b 0 -q -s -k %o %f";
string MP3FILE::default_mp3_output_cmd = "lame -b 128 -x -S - %f";

void MP3FILE::set_mp3_input_cmd(const string& value) { MP3FILE::default_mp3_input_cmd = value; }
void MP3FILE::set_mp3_output_cmd(const string& value) { MP3FILE::default_mp3_output_cmd = value; }

MP3FILE::MP3FILE(const string& name) {
  label(name);
  finished_rep = false;
}

MP3FILE::~MP3FILE(void) { close(); }

void MP3FILE::open(void) { 
  if (io_mode() == io_read) {
    get_mp3_params(label());
  }
}

void MP3FILE::close(void) {
  if (io_mode() == io_read) {
    kill_mpg123();
  }
  else {
    kill_lame();
  }
  toggle_open_state(false);
}

long int MP3FILE::read_samples(void* target_buffer, long int samples) {
  if (is_open() == false) fork_mpg123();
  bytes_rep =  ::read(fd_rep, target_buffer, frame_size() * samples);
  if (bytes_rep < samples * frame_size() || bytes_rep == 0) finished_rep = true;
  else finished_rep = false;

  return(bytes_rep / frame_size());
}

void MP3FILE::write_samples(void* target_buffer, long int samples) {
  if (is_open() == false) fork_lame();
  if (waitpid(pid_of_child_rep, 0, WNOHANG) < 0) { 
    finished_rep = true;
  }
  else {
    bytes_rep = ::write(fd_rep, target_buffer, frame_size() * samples);
    if (bytes_rep < frame_size() * samples || bytes_rep == 0) finished_rep = true;
    else finished_rep = false;
  }
}

void MP3FILE::seek_position(void) {
  if (is_open() == true) {
    finished_rep = false;
    if (io_mode() == io_read) {
      kill_mpg123();
    }
    else {
      kill_lame();
    }
  }
}

void MP3FILE::get_mp3_params(const string& fname) throw(ECA_ERROR*) {
  Layer newlayer;
  newlayer.get(fname.c_str());

  struct stat buf;
  ::stat(fname.c_str(), &buf);
  double fsize = (double)buf.st_size;

  double bitrate = ((double)newlayer.bitrate() * 1000.0);
  double bsecond = (double)bytes_per_second();
  MESSAGE_ITEM m;
  m << "MP3 file size: " << fsize << "\n.";
  m << "MP3 length_value: " << newlayer.length() << "\n.";
  m << "bsecond: " << bsecond << "\n.";
  m << "bitrate: " << bitrate << "\n.";
  length_in_samples((long)ceil(8.0 * fsize / bitrate * bsecond / frame_size()));

  m << "Setting MP3 length_value: " << length_in_seconds() << "\n.";
  pcm_rep = newlayer.pcmPerFrame();

  m << "MP3 pcm value: " << pcm_rep << ".";
  ecadebug->msg(ECA_DEBUG::user_objects,m.to_string());

  set_samples_per_second(newlayer.sfreq());
  set_channels((newlayer.mode() == Layer::MPG_MD_MONO) ? 1 : 2);
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
}

void MP3FILE::kill_mpg123(void) {
  if (is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Killing mpg123-child with pid " + kvu_numtostr(pid_of_child_rep) + ".");
    kill(pid_of_child_rep, SIGKILL);
    waitpid(pid_of_child_rep, 0, 0);
    ::close(fd_rep);
    toggle_open_state(false);
  }
}

void MP3FILE::fork_mpg123(void) throw(ECA_ERROR*) {
  if (!is_open()) {
    
    string cmd = MP3FILE::default_mp3_input_cmd;
    if (cmd.find("%f") != string::npos) {
      cmd.replace(cmd.find("%f"), 2, label());
    }

    if (cmd.find("%o") != string::npos) {
      cmd.replace(cmd.find("%o"), 2, kvu_numtostr((long)(position_in_samples() / pcm_rep)));
    }

    ecadebug->msg(ECA_DEBUG::user_objects,cmd);
   
    int fpipes[2];
    if (pipe(fpipes) == 0) {
      pid_of_child_rep = fork();
      if (pid_of_child_rep == -1) { 
	// ---
	// error
	// ---
	throw(new ECA_ERROR("ECA-MP3","Can't start mpg123-thread!"));
      }
      else if (pid_of_child_rep == 0) { 
	// ---
	// child 
	// ---
	::close(1);
	dup2(fpipes[1], 1);
	::close(fpipes[0]);
	::close(fpipes[1]);
	freopen("/dev/null", "w", stderr);
	vector<string> temp = string_to_words(cmd);
	if (temp.size() > 1024) temp.resize(1024);
	const char* args[1024];
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
	fd_rep = fpipes[0];
	//	if (fobject == NULL) {
	ecadebug->msg("(audioio-mp3) Forked mpg123-child with pid " + kvu_numtostr(pid_of_child_rep) + ".");
	toggle_open_state(true);
      }
    }
    else 
      throw(new ECA_ERROR("ECA-MP3","Can't start mpg123-thread! Check that 'mpg123' is installed properly."));
  }
}

void MP3FILE::kill_lame(void) {
  if (is_open()) {
    ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Killing lame-child with pid " + kvu_numtostr(pid_of_child_rep) + ".");
    kill(pid_of_child_rep, SIGKILL);
    waitpid(pid_of_child_rep, 0, 0);
    ::close(fd_rep);
    toggle_open_state(false);
  }
}

void MP3FILE::fork_lame(void) throw(ECA_ERROR*) {
  if (!is_open()) {

    string cmd = MP3FILE::default_mp3_output_cmd;
    if (cmd.find("%f") != string::npos) {
      cmd.replace(cmd.find("%f"), 2, label());
    }
  
    ecadebug->msg("(audioio-mp3) Starting to encode " + label() + " with lame.");
    ecadebug->msg(ECA_DEBUG::user_objects, cmd);

    int fpipes[2];
    if (pipe(fpipes) == 0) {
      pid_of_child_rep = fork();
      if (pid_of_child_rep == -1) { 
	// ---
	// error
	// ---
	throw(new ECA_ERROR("ECA-MP3","Can't start lame-child!"));
      }
      else if (pid_of_child_rep == 0) { 
	// ---
	// child 
	// ---
	::close(0);
	::dup2(fpipes[0],0);
	::close(fpipes[0]);
	::close(fpipes[1]);
	freopen("/dev/null", "w", stderr);
	vector<string> temp = string_to_words(cmd);
	if (temp.size() > 1024) temp.resize(1024);
	const char* args[1024];
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
	fd_rep = fpipes[1];

	ecadebug->msg("(audioio-mp3) Forked lame-child with pid " + kvu_numtostr(pid_of_child_rep) + ".");
	toggle_open_state(true);
      }
    }
    else 
      throw(new ECA_ERROR("ECA-MP3","Can't start lame-child! Check that 'lame' is installed properly."));
  }
}
