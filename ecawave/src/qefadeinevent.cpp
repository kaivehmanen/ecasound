// ------------------------------------------------------------------------
// qefadeinevent.cpp: Fade-in effect
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <kvutils/kvu_numtostr.h>
#include <ecasound/eca-chainsetup.h>
#include <ecasound/sample-specs.h>
#include "qefadeinevent.h"

QEFadeInEvent::QEFadeInEvent(ECA_CONTROL* ctrl,
			     const string& input,
			     const string& output,
			     long int start_pos, 
			     long int length) 
  : QEBlockingEvent(ctrl),
    ectrl(ctrl) {

  init("fadeinevent", "default");
  set_input(input);
  set_input_position(start_pos);
  set_length(length);
  set_default_audio_format(input);
  set_output(output);
  set_output_position(start_pos);

  long int srate = SAMPLE_SPECS::sample_rate_default;
  ECA_CHAINSETUP* cs_rep = ectrl->get_chainsetup();
  if (cs_rep != 0) {
    srate = cs_rep->sample_rate();
  }
  double fade_length = length;
  fade_length /= srate;
  ectrl->add_chain_operator("-ea:100");
  ectrl->add_controller("-kl:1,0,100," + kvu_numtostr(fade_length, 32));
}
