// ------------------------------------------------------------------------
// audioio-flac.cpp: Interface to FLAC decoders and encoders using UNIX 
//                   pipe i/o.
// Copyright (C) 2004 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include "audioio-flac.h"

#include "eca-logger.h"

/**
 * References:
 *    http://flac.sourceforge.net/
 *    http://flac.sourceforge.net/format.html
 */

string FLAC_FORKED_INTERFACE::default_input_cmd = "flac -d -c %f";
string FLAC_FORKED_INTERFACE::default_output_cmd = "flac -o %f --force-raw-format --channels=%c --bps=%b --sample-rate=%s --sign=%I --endian=%E -";

void FLAC_FORKED_INTERFACE::set_input_cmd(const std::string& value) { FLAC_FORKED_INTERFACE::default_input_cmd = value; }
void FLAC_FORKED_INTERFACE::set_output_cmd(const std::string& value) { FLAC_FORKED_INTERFACE::default_output_cmd = value; }

FLAC_FORKED_INTERFACE::FLAC_FORKED_INTERFACE(const std::string& name)
{
  set_label(name);
  finished_rep = false;
}

FLAC_FORKED_INTERFACE::~FLAC_FORKED_INTERFACE(void)
{
  if (is_open() == true) {
    close();
  }
}

void FLAC_FORKED_INTERFACE::open(void) throw (AUDIO_IO::SETUP_ERROR &)
{
  std::string urlprefix;
  triggered_rep = false;

  /**
   * FIXME: we have no idea about the audio format of the 
   *        stream we get from ogg123... maybe we should force decoder
   *        to generate RIFF wave to a named pipe and parse the header...?
   */

  if (io_mode() == io_read) {
    struct stat buf;
    int ret = ::stat(label().c_str(), &buf);
    if (ret != 0) {
      size_t offset = label().find_first_of("://");
      if (offset == std::string::npos) {
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-FLAC: Can't open file " + label() + "."));
      }
      else {
	urlprefix = std::string(label(), 0, offset);
	ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-flac) Found url; protocol '" + urlprefix + "'.");
      }
    }

    /* decoder supports: nothing configurable nor fixed
     * 
     * FIXME: we have no idea about the audio format of the 
     *        stream we get from the decoder... ybe we should force the decoder
     *        to generate RIFF wave to a named pipe and parse the header...? 
     * 
     *        - possibly copy FLAC__metadata_get_streaminfo() from flac 
     *          package... might be messy due to dependencies
     *        - see http://flac.sourceforge.net/format.html
     */
  }
  else {
    /* encoder supports: coding, channel-count, srate and endianess configurable */
  }

  AUDIO_IO::open();
}

void FLAC_FORKED_INTERFACE::close(void)
{
  if (pid_of_child() > 0) {
      ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-mp3) Cleaning child process." + kvu_numtostr(pid_of_child()) + ".");
      clean_child();
      triggered_rep = false;
  }

  AUDIO_IO::close();
}

long int FLAC_FORKED_INTERFACE::read_samples(void* target_buffer, long int samples)
{
  if (triggered_rep != true) { 
    triggered_rep = true;
    fork_input_process();
  }

  if (f1_rep != 0) {
    bytes_rep = std::fread(target_buffer, 1, frame_size() * samples, f1_rep);
  }
  else {
    bytes_rep = 0;
  }

  if (bytes_rep < samples * frame_size() || bytes_rep == 0) {
    if (position_in_samples() == 0) 
      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-flac) Can't start process \"" + FLAC_FORKED_INTERFACE::default_input_cmd + "\". Please check your ~/.ecasound/ecasoundrc.");
    finished_rep = true;
    triggered_rep = false;
  }
  else 
    finished_rep = false;

  return(bytes_rep / frame_size());
}

void FLAC_FORKED_INTERFACE::write_samples(void* target_buffer, long int samples)
{
  if (triggered_rep != true) {
    triggered_rep = true;
    fork_output_process();
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

void FLAC_FORKED_INTERFACE::seek_position(void) {
  if (pid_of_child() > 0) {
    ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-flac) Cleaning child process." + kvu_numtostr(pid_of_child()) + ".");
    clean_child();
    triggered_rep = false;
  }
  set_position_in_samples(0);
}

void FLAC_FORKED_INTERFACE::set_parameter(int param, string value)
{
  switch (param) {
  case 1: 
    set_label(value);
    break;
  }
}

string FLAC_FORKED_INTERFACE::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(label());
  }
  return("");
}

void FLAC_FORKED_INTERFACE::fork_input_process(void)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, FLAC_FORKED_INTERFACE::default_input_cmd);

  set_fork_command(FLAC_FORKED_INTERFACE::default_input_cmd);
  set_fork_file_name(label());

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

void FLAC_FORKED_INTERFACE::fork_output_process(void)
{
  ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-flac) Starting to encode FLAC stream to " + label() + ".");
  string command = FLAC_FORKED_INTERFACE::default_output_cmd;

  // replace with 'little/big' byteorder
  if (command.find("%E") != string::npos) {
    string byteorder ("big");
    if (sample_endianess() == ECA_AUDIO_FORMAT::se_little) byteorder = "little";
    command.replace(command.find("%E"), 2, byteorder);
  }

  // replace with 'signed/unsigned'
  if (command.find("%I") != string::npos) {
    string sign ("signed");
    if (sample_coding() == ECA_AUDIO_FORMAT::sc_unsigned) sign = "unsigned";
    command.replace(command.find("%I"), 2, sign);
  }

  set_fork_command(command);
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
