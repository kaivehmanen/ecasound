// ------------------------------------------------------------------------
// audioio_alsa_hw.cpp: ALSA PCM device (hw/plugin) input/output.
// Copyright (C) 2001,2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include "eca-version.h"
#include "audioio_alsa.h"

static const char* audio_io_keyword_const = "alsahw_09";
static const char* audio_io_keyword_regex_const = "(^alsahw_09$)|(^alsaplugin_09$)";

const char* audio_io_keyword(void){return(audio_io_keyword_const); }
const char* audio_io_keyword_regex(void){return(audio_io_keyword_regex_const); }
AUDIO_IO* audio_io_descriptor(void) { return(new AUDIO_IO_ALSA_PCM()); }
int audio_io_interface_version(void) { return(ecasound_library_version_current); }

