// ------------------------------------------------------------------------
// audioio-sine.cpp: Sine wave generator
//
// Adaptation to Ecasound:
// 
// Copyright (C) 2007 Kai Vehmanen (adaptation to Ecasound)
//
// Sources for sine generation (cmt-src-1.15/src/sine.cpp):
//
// Computer Music Toolkit - a library of LADSPA plugins. Copyright (C)
// 2000-2002 Richard W.E. Furse. The author may be contacted at
// richard@muse.demon.co.uk.
//
// Attributes:
//     eca-style-version: 3 (see Ecasound Programmer's Guide)
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

#include <kvu_message_item.h>
#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "eca-object-factory.h"
#include "samplebuffer.h"
#include "sample-specs.h"
#include "audioio-tone.h"

#include "eca-error.h"
#include "eca-logger.h"

using std::cout;
using std::endl;

/* Sine table size is given by (1 << SINE_TABLE_BITS). */
#define SINE_TABLE_BITS 14
#define SINE_TABLE_SHIFT (8 * sizeof(unsigned long) - SINE_TABLE_BITS)

SAMPLE_SPECS::sample_t *g_pfSineTable = NULL;
SAMPLE_SPECS::sample_t g_fPhaseStepBase = 0;

static void initialise_sine_wavetable(void)
{
  if (g_pfSineTable == NULL) {
    long lTableSize = (1 << SINE_TABLE_BITS);
    double dShift = (double(M_PI) * 2) / lTableSize;
    g_pfSineTable = new SAMPLE_SPECS::sample_t[lTableSize];
    if (g_pfSineTable != NULL)
      for (long lIndex = 0; lIndex < lTableSize; lIndex++)
	g_pfSineTable[lIndex] = SAMPLE_SPECS::sample_t(sin(dShift * lIndex));
  }
  if (g_fPhaseStepBase == 0) {
    g_fPhaseStepBase = (SAMPLE_SPECS::sample_t)pow(2, sizeof(unsigned long) * 8);
  }
}

AUDIO_IO_TONE::AUDIO_IO_TONE (const std::string& name)
{
  set_label(name);
  initialise_sine_wavetable();
}

AUDIO_IO_TONE::~AUDIO_IO_TONE(void)
{
}

AUDIO_IO_TONE* AUDIO_IO_TONE::clone(void) const
{
  AUDIO_IO_TONE* target = new AUDIO_IO_TONE();
  for(int n = 0; n < number_of_params(); n++) {
    target->set_parameter(n + 1, get_parameter(n + 1));
  }
  return target;
}

void AUDIO_IO_TONE::open(void) throw(AUDIO_IO::SETUP_ERROR &)
{
}

void AUDIO_IO_TONE::close(void)
{
}


void AUDIO_IO_TONE::read_buffer(SAMPLE_BUFFER* sbuf)
{
  /* write to sbuf->buffer[ch], similarly as the LADSPA
   * chainops */
}

void AUDIO_IO_TONE::write_buffer(SAMPLE_BUFFER* sbuf)
{
  /* NOP */
}

void AUDIO_IO_TONE::seek_position(void)
{
  /* note: must be implemented! */
}
