// ------------------------------------------------------------------------
// audioio.cpp: Routines common for all audio IO-devices.
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

#include <kvutils/message_item.h>

#include "eca-error.h"
#include "audioio.h"
#include "samplebuffer.h"

#include "eca-debug.h"

const string& AUDIO_IO::SETUP_ERROR::message(void) const { return(message_rep); }
AUDIO_IO::SETUP_ERROR::Error_type AUDIO_IO::SETUP_ERROR::type(void) const { return(type_rep); }
AUDIO_IO::SETUP_ERROR::SETUP_ERROR(AUDIO_IO::SETUP_ERROR::Error_type type, 
				   const string& message) 
  : type_rep(type), message_rep(message) { }


// ===================================================================
// Attributes

/**
 * Returns info about supported I/O modes (bitwise-OR)
 *
 * By default, all I/O modes are supported.
 */
int AUDIO_IO::supported_io_modes(void) const { return(io_read | io_readwrite | io_write); }

/**
 * Whether device supports non-blocking I/O mode.
 *
 * By default, nonblocking mode is not supported.
 */
bool AUDIO_IO::supports_nonblocking_mode(void) const { return(false); }

/**
 * Whether device supports non-blocking I/O mode.
 *
 * By default, seeking is supported.
 */
bool AUDIO_IO::supports_seeking(void) const { return(true); }

/**
 * Whether audio stream has a distinct length. It's important
 * to note the difference between this attribute and 
 * 'supports_seeking()'. For example, a file read through 
 * a pipe mechanism is not seekable and its length is not 
 * known until 'finished()´ becomes true, but still, it is 
 * of finite length. A sine oscillator on the other hand 
 * can go on producing a signal forever, and is thus infinite.
 *
 * This attributes directly affects how 'finished()' should
 * to be interpreted. @see finished().
 *
 * By default, audio streams are finite length.
 */
bool AUDIO_IO::finite_length_stream(void) const { return(true); }

/**
 * Whether audio format is locked. If this is true, audio object
 * has a known audio format, and doesn't allow overriding it.
 *
 * By default, audio format is not locked.
 */
bool AUDIO_IO::locked_audio_format(void) const { return(false); }

// ===================================================================
// Configuration (setting and getting configuration parameters)

/**
 * Returns info about the current I/O mode.
 */
int AUDIO_IO::io_mode(void) const { return(io_mode_rep); }

/**
 * Set object input/output-mode. If the requested mode isn't
 * supported, the nearest supported mode is used. Because 
 * of this, it's wise to afterwards check whether the requested
 * mode was accepted.
 *
 * require:
 *  is_open() != true
 */
void AUDIO_IO::io_mode(int mode) { io_mode_rep = mode; }

/**
 * Sets object label. Label is used to identify the object.
 *
 * require:
 *  is_open() != true
 */
void AUDIO_IO::label(const string& id_label) { id_label_rep = id_label; }

/**
 * Enable/disbale nonblocking mode.
 *
 * require:
 *  is_open() != true
 */
void AUDIO_IO::toggle_nonblocking_mode(bool value) { nonblocking_rep =
						       value; }

/**
 * Returns the current label. Label can be used for object
 * identification (although not necessarily unique among all
 * objects). Device and file names are typical label values.
 */
const string& AUDIO_IO::label(void) const { return(id_label_rep); }


/**
 * Returns a string containing info about sample format parameters.
 */
string AUDIO_IO::format_info(void) const {
  MESSAGE_ITEM otemp;
  otemp << "(audio-io) ";

  if (locked_audio_format() == true && is_open() != true) {
    otemp << "Using audio format specified in file header data.";
  } else {
    otemp << "Format: " << format_string();
    otemp << ", channels " << channels();
    otemp << ", srate " << samples_per_second();
    if (interleaved_channels() == true) 
      otemp << ", interleaved.";
    else
      otemp << ", noninterleaved.";
  }
  return(otemp.to_string());
}

void AUDIO_IO::set_parameter(int param, 
			     string value) {
  if (param == 1) label(value);
}

string AUDIO_IO::get_parameter(int param) const {
  if (param == 1) return(label());
  return("");
}

// ===================================================================
// Runtime information

/**
 * If applicable, returns total length of the audio data stored
 * into current audio object. In many situations it's impossible
 * enquire the whole length of the object. For instance, if the 
 * object is streaming a finite length audio stream audio object
 * from other applications using some type of standard IPC, 
 * the actual length won't be known until the whole stream has
 * been read. As a general rule, if 'supports_seeking() == true', 
 * length can be known right after initialization. Then again,
 * if 'finite_length_stream() == true', the whole stream must
 * be processed before we know the actual length. In other
 * cases, length is unknown or infinite.
 */
ECA_AUDIO_TIME AUDIO_IO::length(void) const {
  return(ECA_AUDIO_TIME(length_in_samples(), samples_per_second()));
}

/**
 * Returns the current position.
 */
ECA_AUDIO_TIME AUDIO_IO::position(void) const {
  return(ECA_AUDIO_TIME(position_in_samples(), samples_per_second()));
}

/**
 * Is nonblocking mode is enabled?
 */
bool AUDIO_IO::nonblocking_mode(void) const { return(nonblocking_rep); }

/**
 * Is the audio object ready for reading? 
 */
bool AUDIO_IO::readable(void) const { return(is_open() && io_mode() != io_write); }

/**
 * Is the audio object ready for writing? 
 */
bool AUDIO_IO::writable(void) const { return(is_open() && io_mode() != io_read); }

/**
 * Sets the total length of audio object data.
 */
void AUDIO_IO::length(const ECA_AUDIO_TIME& v) {
  length_in_samples(v.samples());
}

/**
 * Sets the current position.
 */
void AUDIO_IO::position(const ECA_AUDIO_TIME& v) {
  position_in_samples(v.samples());
}

/**
 * Sets device's state to enabled or disabled.
 */
void AUDIO_IO::toggle_open_state(bool value) { open_rep = value; }

/**
 * Optional status string
 *
 * An unformatted text string describing the state and status of 
 * the current object.
 */
string AUDIO_IO::status(void) const {
  MESSAGE_ITEM mitem;
  mitem.setprecision(3);

  mitem << "position (" << position_in_seconds_exact();
  mitem << "/" << length_in_seconds_exact();
  mitem << ") seconds.\n -> ";
  
  if (is_open() == true) 
    mitem << "open, ";
  else 
    mitem << "closed, ";

  mitem << format_string() << "/" << channels() << "ch/" << samples_per_second();
  mitem << "Hz, buffer " << buffersize() << ".";

  return(mitem.to_string());
}

// ===================================================================
// Constructors and destructors

AUDIO_IO::~AUDIO_IO(void) { }

AUDIO_IO::AUDIO_IO(const string& name, 
		   int mode, 
		   const ECA_AUDIO_FORMAT& fmt)
  : ECA_AUDIO_POSITION(fmt)
{
  label(name);
  io_mode(mode);
 
  position_in_samples(0);

  nonblocking_rep = false;  
  readable_rep = writable_rep = open_rep = false;
}
