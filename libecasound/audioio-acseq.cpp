// ------------------------------------------------------------------------
// audioio-audioseq.cpp: TBD
// Copyright (C) 2008 Kai Vehmanen
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
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#include <kvu_message_item.h>
#include <kvu_numtostr.h>
#include <kvu_dbc.h>

#include "eca-object-factory.h"
#include "samplebuffer.h"
#include "audioio-acseq.h"

#include "eca-error.h"
#include "eca-logger.h"

using std::cout;
using std::endl;
using SAMPLE_SPECS::sample_pos_t;

/**
 * FIXME notes  (last update 2008-03-04)
 *
 * - TBD
 */

AUDIO_CLIP_SEQUENCER::AUDIO_CLIP_SEQUENCER ()
{
}

AUDIO_CLIP_SEQUENCER::~AUDIO_CLIP_SEQUENCER(void)
{
}

AUDIO_CLIP_SEQUENCER* AUDIO_CLIP_SEQUENCER::clone(void) const
{
  AUDIO_CLIP_SEQUENCER* target = new AUDIO_CLIP_SEQUENCER();
  for(int n = 0; n < number_of_params(); n++) {
    target->set_parameter(n + 1, get_parameter(n + 1));
  }
  return target;
}

void AUDIO_CLIP_SEQUENCER::open(void) throw(AUDIO_IO::SETUP_ERROR &)
{
  AUDIO_SEQUENCER_BASE::toggle_looping(true);
  AUDIO_SEQUENCER_BASE::set_child_object_string(get_parameter(2));

  AUDIO_SEQUENCER_BASE::open();

  /* step: set additional child params (if any) */
  int numparams = child()->number_of_params();
  for(int n = 0; n < numparams; n++) {
    child()->set_parameter(n + 1, get_parameter(n + 3));
    numparams = child()->number_of_params(); // in case 'n_o_p()' varies
  }

#if 0
  /* step: set length of the used audio segment 
   *       (note, result may still be zero) */
  AUDIO_SEQUENCER_BASE::set_child_length(ECA_AUDIO_TIME(child()->length_in_samples(),
							child()->samples_per_second()));
#endif
}

void AUDIO_CLIP_SEQUENCER::close(void)
{
  if (child()->is_open() == true) 
    child()->close();

  AUDIO_IO_PROXY::close();
}

std::string AUDIO_CLIP_SEQUENCER::parameter_names(void) const
{
  return label() + 
    string("," + child()->parameter_names()); 
}

void AUDIO_CLIP_SEQUENCER::set_parameter(int param, string value)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
	      AUDIO_IO::parameter_set_to_string(param, value) + ".");

  if (param > static_cast<int>(params_rep.size())) params_rep.resize(param);

  if (param > 0)
    params_rep[param - 1] = value;
  
  if (param > 2 && 
      child() != 0) {
    child()->set_parameter(param - 2, value);
  }
}

string AUDIO_CLIP_SEQUENCER::get_parameter(int param) const
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects, 
	      AUDIO_IO::parameter_get_to_string(param) + ".");

  if (param > 0 && param < static_cast<int>(params_rep.size()) + 1) {
    if (param > 2 &&
	child() != 0) {
      params_rep[param - 1] = child()->get_parameter(param - 2);
    }
    return params_rep[param - 1];
  }

  return ""; 
}
