// ------------------------------------------------------------------------
// audioio-ogg.cpp: Interface for ogg vorbis ecoders and encoders.
// Copyright (C) 2000-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <cstdlib> /* atol() */
#include <unistd.h> /* stat() */
#include <sys/stat.h> /* stat() */

#include <kvu_message_item.h>
#include <kvu_numtostr.h>

#include "audioio-ogg.h"

#include "eca-logger.h"

string OGG_VORBIS_INTERFACE::default_ogg_input_cmd = "ogg123 -d raw --file=- %f";
string OGG_VORBIS_INTERFACE::default_ogg_output_cmd = "oggenc -b 128 --raw --raw-bits=%b --raw-chan=%c --raw-rate=%s --output=%f -";
long int OGG_VORBIS_INTERFACE::default_ogg_output_default_bitrate = 128000;

void OGG_VORBIS_INTERFACE::set_ogg_input_cmd(const std::string& value) { OGG_VORBIS_INTERFACE::default_ogg_input_cmd = value; }
void OGG_VORBIS_INTERFACE::set_ogg_output_cmd(const std::string& value) { OGG_VORBIS_INTERFACE::default_ogg_output_cmd = value; }

OGG_VORBIS_INTERFACE::OGG_VORBIS_INTERFACE(const std::string& name)
{
  set_label(name);
  finished_rep = false;
  bitrate_rep = OGG_VORBIS_INTERFACE::default_ogg_output_default_bitrate;
}

OGG_VORBIS_INTERFACE::~OGG_VORBIS_INTERFACE(void)
{
  if (is_open() == true) {
    close();
  }
}

void OGG_VORBIS_INTERFACE::open(void) throw (AUDIO_IO::SETUP_ERROR &)
{
  std::string urlprefix;
  triggered_rep = false;

  /**
   * FIXME: we have no idea about the audio format of the 
   *        stream we get from ogg123!
   */

  if (io_mode() == io_read) {
    struct stat buf;
    int ret = ::stat(label().c_str(), &buf);
    if (ret != 0) {
      size_t offset = label().find_first_of("://");
      if (offset == std::string::npos) {
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-OGG: Can't open file " + label() + "."));
      }
      else {
	urlprefix = std::string(label(), 0, offset);
	ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-ogg) Found url; protocol '" + urlprefix + "'.");
      }
    }
  }

  AUDIO_IO::open();
}

void OGG_VORBIS_INTERFACE::close(void)
{
  if (pid_of_child() > 0) {
      ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-mp3) Cleaning child process." + kvu_numtostr(pid_of_child()) + ".");
      clean_child();
      triggered_rep = false;
  }

  AUDIO_IO::close();
}

long int OGG_VORBIS_INTERFACE::read_samples(void* target_buffer, long int samples)
{
  if (triggered_rep != true) { 
    triggered_rep = true;
    fork_ogg_input();
  }

  if (f1_rep != 0) {
    bytes_rep = std::fread(target_buffer, 1, frame_size() * samples, f1_rep);
  }
  else {
    bytes_rep = 0;
  }

  if (bytes_rep < samples * frame_size() || bytes_rep == 0) {
    if (position_in_samples() == 0) 
      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-ogg) Can't start process \"" + OGG_VORBIS_INTERFACE::default_ogg_input_cmd + "\". Please check your ~/.ecasoundrc.");
    finished_rep = true;
    triggered_rep = false;
  }
  else 
    finished_rep = false;

  return(bytes_rep / frame_size());
}

void OGG_VORBIS_INTERFACE::write_samples(void* target_buffer, long int samples)
{
  if (triggered_rep != true) {
    triggered_rep = true;
    fork_ogg_output();
  }

  if (wait_for_child() != true) {
    finished_rep = true;
    triggered_rep = false;
  }
  else {
    if (fd_rep > 0) {
      bytes_rep = ::write(fd_rep, target_buffer, frame_size() * samples);
    }
    else {
      bytes_rep = 0;
    }
    if (bytes_rep < frame_size() * samples || bytes_rep == 0) {
      finished_rep = true;
      triggered_rep = false;
    }
    else finished_rep = false;
  }
}

void OGG_VORBIS_INTERFACE::seek_position(void) {
  if (pid_of_child() > 0) {
    ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-ogg) Cleaning child process." + kvu_numtostr(pid_of_child()) + ".");
    clean_child();
    triggered_rep = false;
  }
  set_position_in_samples(0);
}

void OGG_VORBIS_INTERFACE::set_parameter(int param, string value)
{
  switch (param) {
  case 1: 
    set_label(value);
    break;

  case 2: 
    long int numvalue = atol(value.c_str());
    if (numvalue > 0) 
      bitrate_rep = numvalue;
    else
      bitrate_rep = OGG_VORBIS_INTERFACE::default_ogg_output_default_bitrate;
    break;
  }
}

string OGG_VORBIS_INTERFACE::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(kvu_numtostr(bitrate_rep));
  }
  return("");
}

void OGG_VORBIS_INTERFACE::fork_ogg_input(void)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, OGG_VORBIS_INTERFACE::default_ogg_input_cmd);
  set_fork_command(OGG_VORBIS_INTERFACE::default_ogg_input_cmd);
  set_fork_file_name(label());
  set_fork_pipe_name();
  fork_child_for_read();
  if (child_fork_succeeded() == true) {
    fd_rep = file_descriptor();
    f1_rep = fdopen(fd_rep, "r"); /* not part of <cstdio> */
    if (f1_rep == 0) {
      finished_rep = true;
      triggered_rep = false;
    }
  }
  else
    f1_rep = 0;
}

void OGG_VORBIS_INTERFACE::fork_ogg_output(void)
{
  ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-ogg) Starting to encode " + label() + " with vorbize.");
  string command_rep = OGG_VORBIS_INTERFACE::default_ogg_output_cmd;
  if (command_rep.find("%f") != string::npos) {
    command_rep.replace(command_rep.find("%f"), 2, label());
  }
  if (command_rep.find("%B") != string::npos) {
    command_rep.replace(command_rep.find("%B"), 2, kvu_numtostr((long int)(bitrate_rep / 1000)));
  }

  set_fork_command(command_rep);
  set_fork_file_name(label());

  set_fork_bits(bits());
  set_fork_channels(channels());
  set_fork_sample_rate(samples_per_second());

  fork_child_for_write();
  if (child_fork_succeeded() == true) {
    fd_rep = file_descriptor();
  }
  else {
    fd_rep = 0;
  }
}
