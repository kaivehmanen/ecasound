// ------------------------------------------------------------------------
// audioio-proxy.cpp: Generic interface for objects that act as
//                    proxies for other objects of type AUDIO_IO.
// Copyright (C) 2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <kvu_dbc.h>

#include "samplebuffer.h"
#include "audioio-null.h"
#include "audioio-proxy.h"

AUDIO_IO_PROXY::AUDIO_IO_PROXY(void) 
  : buffersize_rep(0)
{
  child_repp = new NULLFILE("uninitialized");
}

AUDIO_IO_PROXY::~AUDIO_IO_PROXY(void)
{ 
  delete child_repp; // either null or the actual child object
}

/**
 * Sets a new proxy target object. The old 
 * target (if any) is deleted.
 */
void AUDIO_IO_PROXY::set_child(AUDIO_IO* v)
{ 
  if (v != 0) {
    delete child_repp; // the placeholder null object
    child_repp = v;
  }
}

void AUDIO_IO_PROXY::set_buffersize(long int samples)
{
  buffersize_rep = samples;
  child_repp->set_buffersize(samples);
}

void AUDIO_IO_PROXY::set_channels(SAMPLE_SPECS::channel_t v)
{
  AUDIO_IO::set_channels(v);
  child_repp->set_channels(v);
}

void AUDIO_IO_PROXY::set_sample_format(Sample_format v) throw(ECA_ERROR&)
{
  AUDIO_IO::set_sample_format(v);
  child_repp->set_sample_format(v);
}

void AUDIO_IO_PROXY::set_position_in_samples(SAMPLE_SPECS::sample_pos_t pos)
{
  /* only way to update the current position */
  child_repp->seek_position_in_samples(pos);
  AUDIO_IO::set_position_in_samples(pos);
}

void AUDIO_IO_PROXY::set_length_in_samples(SAMPLE_SPECS::sample_pos_t pos)
{
  AUDIO_IO::set_length_in_samples(pos);
}
