// ------------------------------------------------------------------------
// qecutevent.cpp: Cut specified range to clipboard
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

#include <cstdio>

#include "qecutevent.h"

QECutEvent::QECutEvent(ECA_CONTROLLER* ctrl,
		       const string& input,
		       const string& output,
		       long int start_pos, 
		       long int length) 
  : QEBlockingEvent(ctrl),
    ectrl(ctrl),
    start_pos_rep(start_pos),
    length_rep(length),
    input_rep(input),
    output_rep(output) {

  init("cutevent-phase1", "default");
  set_input(input_rep);
  set_input_position(start_pos);
  set_length(length);
  set_default_audio_format(input_rep);
  ectrl->set_chainsetup_output_mode(AUDIO_IO::io_write);
  set_output(output_rep);
  set_output_position(0);
}

void QECutEvent::start(void) {
  // copy before cut-area to clipboard
  blocking_start(); 

  // copy before-cut to temporary file
  init("cutevent-phase2", "default");
  set_input(input_rep);
  set_input_position(0);
  set_default_audio_format(input_rep);
  string tmpfile = string(tmpnam(0)) + ".wav";
  ectrl->set_chainsetup_output_mode(AUDIO_IO::io_write);
  set_output(tmpfile);
  set_length(start_pos_rep);
  blocking_start();

  // copy after-cut to temporary file
  init("cutevent-phase3", "default");
  set_input(input_rep);
  set_input_position(start_pos_rep + length_rep);
  set_default_audio_format(input_rep);
  set_length(0);
  set_output(tmpfile);
  ectrl->set_chainsetup_output_mode(AUDIO_IO::io_readwrite);  
  blocking_start();

  // copy temporary over the input file  
  init("cutevent-phase4", "default");
  ectrl->set_chainsetup_output_mode(AUDIO_IO::io_write);  
  set_input(tmpfile);
  set_default_audio_format(tmpfile);
  set_output(input_rep);
  set_length(0);
  blocking_start();
}


