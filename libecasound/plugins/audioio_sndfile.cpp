// ------------------------------------------------------------------------
// audioio_sndfile.cpp: Interface to the sndfile library.
// Copyright (C) 2003 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

#include <kvu_message_item.h>
#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "audioio_sndfile.h"
#include "samplebuffer.h"
#include "eca-version.h"
#include "eca-error.h"
#include "eca-logger.h"

#ifdef ECA_ENABLE_AUDIOIO_PLUGINS
static const char* audio_io_keyword_const = "sndfile";
static const char* audio_io_keyword_regex_const = "(^sndfile$)|(w64$)";

const char* audio_io_keyword(void){return(audio_io_keyword_const); }
const char* audio_io_keyword_regex(void){return(audio_io_keyword_regex_const); }
int audio_io_interface_version(void) { return(ecasound_library_version_current); }
#endif

#ifdef WORDS_BIGENDIAN
static const ECA_AUDIO_FORMAT::Sample_format audioio_sndfile_sfmt = ECA_AUDIO_FORMAT::sfmt_f32_be;
#else
static const ECA_AUDIO_FORMAT::Sample_format audioio_sndfile_sfmt = ECA_AUDIO_FORMAT::sfmt_f32_le;
#endif

using namespace std;

SNDFILE_INTERFACE::SNDFILE_INTERFACE (const string& name)
{
  finished_rep = false;
  snd_repp = 0;
  closing_rep = false;
  set_label(name);
}

SNDFILE_INTERFACE::~SNDFILE_INTERFACE(void)
{
  if (is_open() == true) {
    close();
  }
}

SNDFILE_INTERFACE* SNDFILE_INTERFACE::clone(void) const
{
  SNDFILE_INTERFACE* target = new SNDFILE_INTERFACE();
  for(int n = 0; n < number_of_params(); n++) {
    target->set_parameter(n + 1, get_parameter(n + 1));
  }
  return(target);
}

/**
 * Parses the information given in 'sfinfo'. 
 */ 
void SNDFILE_INTERFACE::open_parse_info(const SF_INFO* sfinfo) throw(AUDIO_IO::SETUP_ERROR&)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, "(audioio-sndfile) audio file format: " + kvu_numtostr(sfinfo->format & SF_FORMAT_SUBMASK)); 

  string format;

  set_samples_per_second(static_cast<long int>(sfinfo->samplerate));
  set_channels(sfinfo->channels);

  switch(sfinfo->format & SF_FORMAT_SUBMASK) 
    {
    case SF_FORMAT_PCM_S8: { format = "s8"; break; }
    case SF_FORMAT_PCM_U8: { format = "u8"; break; }
    case SF_FORMAT_PCM_16: { format = "s16"; break; }
    case SF_FORMAT_PCM_24: { format = "s24"; break; }
    case SF_FORMAT_PCM_32: { format = "s32"; break; }
    default: 
      {
	/* SF_FORMAT_FLOAT */ 
	format = "f32"; 
	break; 
      }
    }
  
  if (sfinfo->format & SF_ENDIAN_LITTLE) 
    format += "_le";
  else if (sfinfo->format & SF_ENDIAN_BIG) 
    format += "_be";
  
  set_sample_format_string(format);
  set_length_in_samples(sfinfo->frames);
}

void SNDFILE_INTERFACE::open(void) throw(AUDIO_IO::SETUP_ERROR&)
{
  AUDIO_IO::open();

  SF_INFO sfinfo;

  string real_filename = label();
  if (real_filename == "sndfile") {
    real_filename = opt_filename_rep;
  }

  if (io_mode() == io_read) {
    ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-sndfile) Using libsndfile to open file \"" +
		real_filename + "\" for reading.");


    snd_repp = sf_open(real_filename.c_str(), SFM_READ, &sfinfo);
    if (snd_repp == NULL) {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Can't open file \"" + real_filename
			+ "\" for reading."));
    }
    else {
      open_parse_info(&sfinfo);
    }
  }
  else {
    /* write or readwrite */

    int file_format = -1;
    string teksti = real_filename;
    kvu_to_lowercase(teksti);

    // FIXME: add support for more output types
    
    if (strstr(teksti.c_str(),".wav") != 0) { file_format = SF_FORMAT_WAV; }
    else if (strstr(teksti.c_str(),".w64") != 0) { file_format = SF_FORMAT_W64; }
    else if (strstr(teksti.c_str(),".aiff") != 0) { file_format = SF_FORMAT_AIFF; }
    else if (strstr(teksti.c_str(),".aifc") != 0) { file_format = SF_FORMAT_AIFF; }
    else if (strstr(teksti.c_str(),".raw") != 0) { file_format = SF_FORMAT_RAW; }
    else if (strstr(teksti.c_str(),".au") != 0) { file_format = SF_FORMAT_AU; }
    else if (strstr(teksti.c_str(),".paf") != 0) { file_format = SF_FORMAT_PAF; }
    else if (strstr(teksti.c_str(),".iff") != 0) { file_format = SF_FORMAT_SVX; }
    else if (strstr(teksti.c_str(),".svx") != 0) { file_format = SF_FORMAT_SVX; }
    else if (strstr(teksti.c_str(),".nist") != 0) { file_format = SF_FORMAT_NIST; }
    else if (strstr(teksti.c_str(),".voc") != 0) { file_format = SF_FORMAT_VOC; }
    else if (strstr(teksti.c_str(),".xi") != 0) { file_format = SF_FORMAT_XI; }
    else if (strstr(teksti.c_str(),".pvf") != 0) { file_format = SF_FORMAT_PVF; }
    else if (strstr(teksti.c_str(),".htk") != 0) { file_format = SF_FORMAT_HTK; }
    else {
      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-sndfile) Warning! Unknown audio format, using WAV format instead.");
      file_format = SF_FORMAT_WAV;
    }
    
    if (format_string()[0] == 'u' && bits() == 8)
      file_format |= SF_FORMAT_PCM_S8;
    else if (format_string()[0] == 's') {
      if (bits() == 8) { file_format |= SF_FORMAT_PCM_S8; }
      else if (bits() == 16) { file_format |= SF_FORMAT_PCM_16; }
      else if (bits() == 24) { file_format |= SF_FORMAT_PCM_24; }
      else if (bits() == 32) { file_format |= SF_FORMAT_PCM_32; }
      else { file_format = 0; }
    }
    else { file_format = 0; }
    
    if (file_format == 0) {
      throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Error! Unknown audio format requested."));
    }

    // FIXME: set endianess

    /* set samplerate and channels */
    sfinfo.samplerate = samples_per_second();
    sfinfo.channels = channels();
    sfinfo.format = file_format;
  
    if (io_mode() == io_write) {
      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-sndfile) Using libsndfile to open file \"" +
		  real_filename + "\" for writing.");

      /* open the file */
      snd_repp = sf_open(real_filename.c_str(), SFM_WRITE, &sfinfo);
      if (snd_repp == NULL) {
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Can't open file \"" + real_filename
			  + "\" for writing."));
      }
    }
    else {
      ECA_LOG_MSG(ECA_LOGGER::info, "(audioio-sndfile) Using libsndfile to open file \"" +
		  real_filename + "\" for read/write.");

      /* io_readwrite */
      snd_repp = sf_open(real_filename.c_str(), SFM_RDWR, &sfinfo);
      if (snd_repp == NULL) {
	throw(SETUP_ERROR(SETUP_ERROR::io_mode, "AUDIOIO-SNDFILE: Can't open file \"" + real_filename
			  + "\" for updating (read/write)."));
      }
      else {
	open_parse_info(&sfinfo);
      }
    }
  }

  /* we need to reserve extra memory as we using 32bit 
   * floats as the internal sample unit */
  reserve_buffer_space((sizeof(float) * channels()) * buffersize());
}

void SNDFILE_INTERFACE::close(void)
{
  if (is_open() == true) {
    DBC_CHECK(closing_rep != true);
    if (snd_repp != 0 && closing_rep != true) {
      closing_rep = true;
      sf_close(snd_repp);
      snd_repp = 0;
    }
  }
  AUDIO_IO::close();
  closing_rep = false;
}

bool SNDFILE_INTERFACE::finished(void) const
{
  if (finished_rep == true || 
      (io_mode() == io_read && out_position())) return(true);

  return false;
}

long int SNDFILE_INTERFACE::read_samples(void* target_buffer, 
					 long int samples)
{
  // samples_read = sf_read_raw(snd_repp, target_buffer, frame_size() * samples);
  // samples_read /= frame_size();
  samples_read = sf_readf_float(snd_repp, (float*)target_buffer, samples);
  finished_rep = (samples_read < samples) ? true : false;
  return(samples_read);
}

void SNDFILE_INTERFACE::write_samples(void* target_buffer, 
				      long int samples)
{
  //sf_write_raw(snd_repp, target_buffer, frame_size() * samples);
  sf_writef_float(snd_repp, (float*)target_buffer, samples);
}

void SNDFILE_INTERFACE::read_buffer(SAMPLE_BUFFER* sbuf)
{
  // --------
  DBC_REQUIRE(get_iobuf() != 0);
  DBC_REQUIRE(static_cast<long int>(get_iobuf_size()) >= buffersize() * frame_size());
  // --------

  /* note! modified from audioio-buffered.cpp */

  DBC_CHECK(interleaved_channels() == true);

  /* in normal conditions this won't cause memory reallocs */
  reserve_buffer_space((sizeof(float) * channels()) * buffersize());

  sbuf->import_interleaved(get_iobuf(),
			   read_samples(get_iobuf(), buffersize()),
			   audioio_sndfile_sfmt,
			   channels());
  change_position_in_samples(sbuf->length_in_samples());
}

void SNDFILE_INTERFACE::write_buffer(SAMPLE_BUFFER* sbuf)
{
  // --------
  DBC_REQUIRE(get_iobuf() != 0);
  DBC_REQUIRE(static_cast<long int>(get_iobuf_size()) >= buffersize() * frame_size());
  // --------

  /* note! modified from audioio-buffered.cpp */
  
  DBC_CHECK(interleaved_channels() == true);

  /* in normal conditions this won't cause memory reallocs */
  reserve_buffer_space((sizeof(float) * channels()) * buffersize());

  set_buffersize(sbuf->length_in_samples());

  sbuf->export_interleaved(get_iobuf(),
			   audioio_sndfile_sfmt,
			   channels());
  write_samples(get_iobuf(), sbuf->length_in_samples());
  change_position_in_samples(sbuf->length_in_samples());
  extend_position();
}

void SNDFILE_INTERFACE::seek_position(void)
{
  // FIXME: check if format supports seeking
  sf_seek(snd_repp, position_in_samples(), SEEK_SET);
  finished_rep = false;
}

void SNDFILE_INTERFACE::set_parameter(int param, 
				      string value)
{
  switch (param) {
  case 1: 
    set_label(value);
    break;

  case 2: 
    opt_filename_rep = value;
    break;
  }
}

string SNDFILE_INTERFACE::get_parameter(int param) const
{
  switch (param) {
  case 1: 
    return(label());

  case 2: 
    return(opt_filename_rep);
  }
  return("");
}
