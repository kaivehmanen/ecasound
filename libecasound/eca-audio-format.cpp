// ------------------------------------------------------------------------
// eca-audio-format.cpp: Class for representing audio format parameters
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include "eca-audio-format.h"
#include "eca-error.h"

ECA_AUDIO_FORMAT::ECA_AUDIO_FORMAT(int channels, 
				   long int srate,
				   ECA_AUDIO_FORMAT::Sample_format format, 
				   bool ileaved)
{
  set_channels(channels);
  set_samples_per_second(srate);
  set_sample_format(format);
  toggle_interleaved_channels(ileaved);
}

ECA_AUDIO_FORMAT::ECA_AUDIO_FORMAT(void)
{ 
  set_channels(0);
  set_samples_per_second(-1);
  set_sample_format(sfmt_none);
  toggle_interleaved_channels(true);

}

ECA_AUDIO_FORMAT::~ECA_AUDIO_FORMAT(void) 
{
}

ECA_AUDIO_FORMAT ECA_AUDIO_FORMAT::audio_format(void) const
{
  return(ECA_AUDIO_FORMAT(channels(),
			  samples_per_second(),
			  sample_format(),
			  interleaved_channels()));
}

/**
 * Sets audio format to that of 'f'.
 */
void ECA_AUDIO_FORMAT::set_audio_format(const ECA_AUDIO_FORMAT& f)
{
  set_channels(f.channels());
  set_sample_format(f.sample_format());
  set_samples_per_second(f.samples_per_second());
  toggle_interleaved_channels(f.interleaved_channels());
}

void ECA_AUDIO_FORMAT::set_sample_format(ECA_AUDIO_FORMAT::Sample_format v) throw(ECA_ERROR&)
{
  sfmt_rep = v;
  convert_to_host_byte_order();
  switch(sfmt_rep) 
    {
    case sfmt_none:
      align_rep = 0; 
      break;

    case sfmt_u8:
    case sfmt_s8:
      align_rep = 1;
      break;

    case sfmt_s16_le:
    case sfmt_s16_be:
      align_rep = 2;
      break;

    case sfmt_s24_le:
    case sfmt_s24_be:
      align_rep = 3;
      break;

    case sfmt_s32_le:
    case sfmt_s32_be:
    case sfmt_f32_le:
    case sfmt_f32_be:
      align_rep = 4;
      break;

    case sfmt_f64_le:
    case sfmt_f64_be:
      align_rep = 8;
      break;

    default: { throw(ECA_ERROR("ECA_AUDIO_FORMAT","Audio format not supported!")); }
    }
}

void ECA_AUDIO_FORMAT::convert_to_host_byte_order(void)
{
  switch(sfmt_rep) 
    {
#ifdef WORDS_BIGENDIAN
    case sfmt_s16: { sfmt_rep = sfmt_s16_be; break; }
    case sfmt_s24: { sfmt_rep = sfmt_s24_be; break; }
    case sfmt_s32: { sfmt_rep = sfmt_s32_be; break; }
    case sfmt_f32: { sfmt_rep = sfmt_f32_be; break; }
    case sfmt_f64: { sfmt_rep = sfmt_f64_be; break; }
#else
    case sfmt_s16: { sfmt_rep = sfmt_s16_le; break; }
    case sfmt_s24: { sfmt_rep = sfmt_s24_le; break; }
    case sfmt_s32: { sfmt_rep = sfmt_s32_le; break; }
    case sfmt_f32: { sfmt_rep = sfmt_f32_le; break; }
    case sfmt_f64: { sfmt_rep = sfmt_f64_le; break; }
#endif
    default: { }
    }
}

int ECA_AUDIO_FORMAT::bits(void) const
{
  return(align_rep * 8);
}

void ECA_AUDIO_FORMAT::set_channels(int v)
{
  channels_rep = v; 
}

void ECA_AUDIO_FORMAT::toggle_interleaved_channels(bool v)
{
  ileaved_rep = v; 
}

void ECA_AUDIO_FORMAT::set_sample_format_string(const std::string& f_str) throw(ECA_ERROR&)
{
  if (f_str == "u8") sfmt_rep = sfmt_u8;
  else if (f_str == "s16") sfmt_rep = sfmt_s16;
  else if (f_str == "s16_le") sfmt_rep = sfmt_s16_le;
  else if (f_str == "s16_be") sfmt_rep = sfmt_s16_be;
  else if (f_str == "s24") sfmt_rep = sfmt_s24;
  else if (f_str == "s24_le") sfmt_rep = sfmt_s24_le;
  else if (f_str == "s24_be") sfmt_rep = sfmt_s24_be;
  else if (f_str == "s32") sfmt_rep = sfmt_s32;
  else if (f_str == "s32_le") sfmt_rep = sfmt_s32_le;
  else if (f_str == "s32_be") sfmt_rep = sfmt_s32_be;
  else if (f_str == "f32") sfmt_rep = sfmt_f32;
  else if (f_str == "f32_le") sfmt_rep = sfmt_f32_le;
  else if (f_str == "f32_be") sfmt_rep = sfmt_f32_be;
  else if (f_str == "8") sfmt_rep = sfmt_u8;
  else if (f_str == "16") sfmt_rep = sfmt_s16;
  else if (f_str == "24") sfmt_rep = sfmt_s24;
  else if (f_str == "32") sfmt_rep = sfmt_s32;
  else {
    throw(ECA_ERROR("ECA_AUDIO_FORMAT", "Unknown sample format \""
			+ f_str + "\"."));
  }
  set_sample_format(sfmt_rep);
}

string ECA_AUDIO_FORMAT::format_string(void) const throw(ECA_ERROR&)
{
  switch(sfmt_rep) 
    {
    case sfmt_none: return("none");
    case sfmt_u8: return("u8");
    case sfmt_s8: return("s8");
    case sfmt_s16_le: return("s16_le");
    case sfmt_s16_be: return("s16_be");
    case sfmt_s24_le: return("s24_le");
    case sfmt_s24_be: return("s24_be");
    case sfmt_s32_le: return("s32_le");
    case sfmt_s32_be: return("s32_be");
    case sfmt_f32_le: return("f32_le");
    case sfmt_f32_be: return("f32_be");
    default: { throw(ECA_ERROR("ECA_AUDIO_FORMAT","Audio format not supported!")); }
    }
}
