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

#include "eca-debug.h"

FILE* audioio_mp3_pipe;

string MP3FILE::default_mp3_input_cmd = "mpg123 -b 0 -q -s -k %o %f";
string MP3FILE::default_mp3_output_cmd = "lame -b 128 -x -S - %f";

void MP3FILE::set_mp3_input_cmd(const string& value) { MP3FILE::default_mp3_input_cmd = value; }
void MP3FILE::set_mp3_output_cmd(const string& value) { MP3FILE::default_mp3_output_cmd = value; }

MP3FILE::MP3FILE(const string& name) {
  label(name);
  finished_rep = false;
  mono_input_rep = false;
  pcm_rep = 1;
}

MP3FILE::~MP3FILE(void) { close(); }

void MP3FILE::open(void) throw(SETUP_ERROR&) { 
  if (io_mode() == io_read) {
    get_mp3_params(label());
  }
  fork_mpg123();
  toggle_open_state(true);
  triggered_rep = false;
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

void MP3FILE::process_mono_fix(char* target_buffer, long int bytes_rep) {
  for(long int n = 0; n < bytes_rep;) {
    target_buffer[n + 2] = target_buffer[n];
    target_buffer[n + 3] = target_buffer[n + 1];
    n += 4;
  }
}

long int MP3FILE::read_samples(void* target_buffer, long int samples) {
  if (triggered_rep != true) triggered_rep = true;
//    bytes_rep =  ::read(fd_rep, target_buffer, frame_size() * samples);
  bytes_rep = ::fread(target_buffer, 1, frame_size() * samples, f1_rep);
  if (mono_input_rep == true)
    process_mono_fix(static_cast<char*>(target_buffer), bytes_rep);
  if (bytes_rep < samples * frame_size() || bytes_rep == 0) {
    if (position_in_samples() == 0) 
      ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Can't start process \"" + MP3FILE::default_mp3_input_cmd + "\". Please check your ~/.ecasoundrc.");
    finished_rep = true;
  }
  else finished_rep = false;
  
  return(bytes_rep / frame_size());
}

void MP3FILE::write_samples(void* target_buffer, long int samples) {
  if (triggered_rep != true) triggered_rep = true;
  if (wait_for_child() != true) {
    finished_rep = true;
  }
  else {
    bytes_rep = ::write(fd_rep, target_buffer, frame_size() * samples);
    if (bytes_rep < frame_size() * samples || bytes_rep == 0) {
      if (position_in_samples() == 0) 
	ecadebug->msg(ECA_DEBUG::info, "(audioio-mp3) Can't start process \"" + MP3FILE::default_mp3_output_cmd + "\". Please check your ~/.ecasoundrc.");
      finished_rep = true;
    }
    else finished_rep = false;
  }
}

void MP3FILE::seek_position(void) {
  if (is_open() == true) {
    finished_rep = false;
    if (io_mode() == io_read)
      kill_mpg123();
    else
      kill_lame();
  }
//    cerr << "Just killed mpg123." << endl;
  if (io_mode() == io_read) {
//      cerr << "Forking mpg123." << endl;
    fork_mpg123(); 
  }
  else
    fork_lame();
}

void MP3FILE::get_mp3_params(const string& fname) throw(SETUP_ERROR&) {
  Layer newlayer;

  if (newlayer.get(fname.c_str()) != true) {
    throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-CDR: Can't open " + label() + " for reading."));
  }

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
  if (bitrate != 0)
    length_in_samples((long)ceil(8.0 * fsize / bitrate * bsecond / frame_size()));
  
  if (bitrate == 0 ||
      length_in_samples() < 0) length_in_samples(0);
  
  m << "Setting MP3 length_value: " << length_in_seconds() << "\n.";
  pcm_rep = newlayer.pcmPerFrame();
  
  m << "MP3 pcm value: " << pcm_rep << ".";
  ecadebug->msg(ECA_DEBUG::user_objects,m.to_string());
  
  // notice! mpg123 always outputs 16bit stereo
  mono_input_rep = (newlayer.mode() == Layer::MPG_MD_MONO) ? true : false;
  set_channels(2); /*  */
  if (mono_input_rep)
    set_samples_per_second(bsecond / 8);
  else
    set_samples_per_second(bsecond / 4);
  set_sample_format(ECA_AUDIO_FORMAT::sfmt_s16_le);
}

void MP3FILE::kill_mpg123(void) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Killing mpg123-child." + kvu_numtostr(pid_of_child()) + ".");
  clean_child();
}

void MP3FILE::fork_mpg123(void) {
  string cmd = MP3FILE::default_mp3_input_cmd;
  if (cmd.find("%o") != string::npos) {
    cmd.replace(cmd.find("%o"), 2, kvu_numtostr((long)(position_in_samples() / pcm_rep)));
  }
  ecadebug->msg(ECA_DEBUG::user_objects,cmd);
  set_fork_command(cmd);
  set_fork_file_name(label());
  fork_child_for_read();
  if (child_fork_succeeded() == true) {
//      cerr << "Child fork succeeded!" << endl;
    fd_rep = file_descriptor();
    f1_rep = fdopen(fd_rep, "r");
    if (f1_rep == 0) finished_rep = true;
  }
}

void MP3FILE::kill_lame(void) {
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-mp3) Killing lame-child with pid " + kvu_numtostr(pid_of_child()) + ".");
  clean_child();
}

void MP3FILE::fork_lame(void) {
  ecadebug->msg("(audioio-mp3) Starting to encode " + label() + " with lame.");
  set_fork_command(MP3FILE::default_mp3_output_cmd);
  set_fork_file_name(label());
  fork_child_for_write();
  if (child_fork_succeeded() == true) {
    fd_rep = file_descriptor();
  }
}
