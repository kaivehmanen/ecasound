// ------------------------------------------------------------------------
// audioio-af.cpp: Interface to SGI audiofile library.
// Copyright (C) 1999-2001 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

#include <audiofile.h>

#include <kvu_message_item.h>
#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "audioio_af.h"
#include "samplebuffer.h"
#include "eca-version.h"
#include "eca-error.h"
#include "eca-debug.h"

static const char* audio_io_keyword_const = "audiofile_aiff_au_snd";
static const char* audio_io_keyword_regex_const = "(aif*$)|(au$)|(snd$)";

const char* audio_io_keyword(void){return(audio_io_keyword_const); }
const char* audio_io_keyword_regex(void){return(audio_io_keyword_regex_const); }
int audio_io_interface_version(void) { return(ecasound_library_version_current); }

AUDIOFILE_INTERFACE::AUDIOFILE_INTERFACE (const string& name)
{
  finished_rep = false;
  set_label(name);
}

AUDIOFILE_INTERFACE::~AUDIOFILE_INTERFACE(void)
{
  if (is_open() == true) {
    close();
  }
}

void AUDIOFILE_INTERFACE::format_query(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  // --------
  DBC_REQUIRE(is_open() != true);
  // --------

  int sample_format, sample_width;
    
  if (io_mode() == io_read) {
    afhandle = ::afOpenFile(label().c_str(), "r", NULL);
    if (afhandle == AF_NULL_FILEHANDLE) {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-AF: Can't open file \"" + label()
			+ "\" using libaudiofile."));
    }
    else {
      set_samples_per_second((long int)::afGetRate(afhandle, AF_DEFAULT_TRACK));
      set_channels(::afGetChannels(afhandle, AF_DEFAULT_TRACK));
      ::afGetSampleFormat(afhandle, AF_DEFAULT_TRACK, &sample_format, &sample_width);
      string format;
      switch(sample_format) 
	{
	case AF_SAMPFMT_TWOSCOMP: { format = "s"; break; }
        case AF_SAMPFMT_UNSIGNED: { format = "u"; break; }
	case AF_SAMPFMT_FLOAT: { format = "f"; break; }
	case AF_SAMPFMT_DOUBLE: { format = "f"; break; }
	}
      format += kvu_numtostr(sample_width);

      //        if (afGetByteOrder(afhandle, AF_DEFAULT_TRACK) == AF_BYTEORDER_BIGENDIAN)
      //  	format += "_be";
      //        else
      //  	format += "_le";
	
      set_sample_format_string(format);

      set_length_in_samples(::afGetFrameCount(afhandle, AF_DEFAULT_TRACK));
      ::afCloseFile(afhandle);
    }
  }

  // -------
  DBC_ENSURE(is_open() != true);
  // -------
}

void AUDIOFILE_INTERFACE::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  AUDIO_IO::open();

  switch(io_mode()) {
  case io_read:
    {
      ecadebug->msg("(audioio-af) Using audiofile library to open file \"" +
		    label() + "\" for reading.");

      afhandle = ::afOpenFile(label().c_str(), "r", NULL);
      if (afhandle == AF_NULL_FILEHANDLE) {
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-AF: Can't open file \"" + label()
			    + "\" using libaudiofile."));
      }
      break;
    }
  case io_write:
    {
      ecadebug->msg("(audioio-af) Using audiofile library to open file \"" +
		    label() + "\" for writing.");

      AFfilesetup fsetup;
      fsetup = afNewFileSetup();

      int file_format = -1;
      string teksti = label();
      kvu_to_lowercase(teksti);

      if (strstr(teksti.c_str(),".aiffc") != 0) { file_format = AF_FILE_AIFFC; }
      else if (strstr(teksti.c_str(),".aifc") != 0) { file_format = AF_FILE_AIFFC; }
      else if (strstr(teksti.c_str(),".aiff") != 0) { file_format = AF_FILE_AIFF; }
      else if (strstr(teksti.c_str(),".aif") != 0) { file_format = AF_FILE_AIFF; }
      else if (strstr(teksti.c_str(),".au") != 0) { file_format = AF_FILE_NEXTSND; }
      else if (strstr(teksti.c_str(),".snd") != 0) { file_format = AF_FILE_NEXTSND; }
      else {
	ecadebug->msg("(audioio-af) Warning! Unknown audio format, using raw format instead.");
	file_format = AF_FILE_RAWDATA;
      }
      ::afInitFileFormat(fsetup, file_format);
      ::afInitChannels(fsetup, AF_DEFAULT_TRACK, channels());

      if (format_string()[0] == 'u')
	::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_UNSIGNED, bits());
      else if (format_string()[0] == 's')
	::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, bits());
      else if (format_string()[0] == 'f') {
	if (bits() == 32) 
	  ::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_FLOAT, bits());
	else
	  ::afInitSampleFormat(fsetup, AF_DEFAULT_TRACK, AF_SAMPFMT_DOUBLE, bits());
      }

      ::afInitRate(fsetup, AF_DEFAULT_TRACK, static_cast<double>(samples_per_second()));

      afhandle = ::afOpenFile(label().c_str(), "w", fsetup);
      if (afhandle == AF_NULL_FILEHANDLE) 
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-AF: Can't open file \"" + label()
			  + "\" using libaudiofile."));
     break;
    }
  
  case io_readwrite:
    {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-AF: Simultaneous intput/ouput not supported."));
    }
  }

  // --
  // Get byteorder
  // --
  //  if (SAMPLE_BUFFER::is_system_littleendian)
  //    afSetVirtualByteOrder(afhandle, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
  //  else
  //    afSetVirtualByteOrder(afhandle, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);

  debug_print_type();
}

void AUDIOFILE_INTERFACE::close(void)
{
  AUDIO_IO::close();
  ::afCloseFile(afhandle);
}


void AUDIOFILE_INTERFACE::debug_print_type(void) {
  int temp = ::afGetFileFormat(afhandle, 0);
  ecadebug->msg(ECA_DEBUG::user_objects, "(audioio-af) afFileformat: " + kvu_numtostr(temp) + "."); 
}

bool AUDIOFILE_INTERFACE::finished(void) const
{
  if (finished_rep == true || 
      (io_mode() == io_read && out_position())) return(true);

  return false;
}

long int AUDIOFILE_INTERFACE::read_samples(void* target_buffer, 
					   long int samples)
{
  samples_read = ::afReadFrames(afhandle, AF_DEFAULT_TRACK,
				target_buffer, samples);
  finished_rep = (samples_read < samples) ? true : false;
  return(samples_read);
}

void AUDIOFILE_INTERFACE::write_samples(void* target_buffer, 
					long int samples)
{
  ::afWriteFrames(afhandle, AF_DEFAULT_TRACK, target_buffer, samples);
}

void AUDIOFILE_INTERFACE::seek_position(void)
{
  ::afSeekFrame(afhandle, AF_DEFAULT_TRACK, position_in_samples());
  finished_rep = false;
}
