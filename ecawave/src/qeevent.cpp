// ------------------------------------------------------------------------
// qeevent.cpp: Virtual base for classes representing libecasound
//              processing events
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

#include <qprogressdialog.h>
#include <ecasound/eca-error.h>

#include "qeevent.h"

QEEvent::QEEvent(ECA_CONTROLLER* ctrl) 
  : ectrl(ctrl) {
#ifdef NDEBUG
  ecadebug->set_debug_level(0);
#else
  ecadebug->set_debug_level(ECA_DEBUG::info |
			    ECA_DEBUG::module_flow |
			    ECA_DEBUG::user_objects);
#endif
}

QEEvent::~QEEvent(void) { 
  if (ectrl->is_running() == true) ectrl->stop();
  if (ectrl->is_connected() == true) ectrl->disconnect_chainsetup();
  if (initialized_cs_rep.empty() != true) {
    ectrl->select_chainsetup(initialized_cs_rep);
    if (ectrl->selected_chainsetup() == initialized_cs_rep) ectrl->remove_chainsetup();
  }
}

void QEEvent::init(const string& chainsetup, const string& chain) {
  // --------
  REQUIRE(ectrl != 0);
  // --------

  triggered_rep = false;
  input_object = 0;
  output_object = 0;
  ectrl->toggle_interactive_mode(true);

  if (ectrl->is_running() == true) ectrl->stop();
  if (ectrl->is_connected() == true) ectrl->disconnect_chainsetup();

  if (initialized_cs_rep.empty() != true) {
    ectrl->select_chainsetup(initialized_cs_rep);
    if (ectrl->selected_chainsetup() == initialized_cs_rep) ectrl->remove_chainsetup();
  }
  if (ectrl->selected_chainsetup() != chainsetup) ectrl->add_chainsetup(chainsetup);
  else {
    ectrl->remove_chainsetup();
    ectrl->add_chainsetup(chainsetup);
  }
  ectrl->clear_chains();
  if (chain != "") ectrl->add_chain(chain);
  initialized_cs_rep = chainsetup;

  // --------
  ENSURE(ectrl->selected_chainsetup() == initialized_cs_rep);
  // --------
}

void QEEvent::blocking_start(void) {
  // --------
  REQUIRE(ectrl->is_valid() == true);
  REQUIRE(ectrl->is_selected() == true);
  REQUIRE(is_triggered() == false);
  // --------

  try {
    ectrl->connect_chainsetup();
    ectrl->start();
    toggle_triggered_state(true);

    struct timespec sleepcount;
    sleepcount.tv_sec = 0;
    sleepcount.tv_nsec = 20000000;
    
    int progress_length = static_cast<int>(ectrl->length_in_seconds_exact() * 10.0);
    if (progress_length == 0 && input_object != 0) 
      progress_length = static_cast<int>(input_object->length_in_seconds_exact() * 10.0);
    double progress_start = ectrl->position_in_seconds_exact();

    QProgressDialog progress ("Processing data...", 0,
			      progress_length - progress_start * 10, 0, 0, true);

    progress.setProgress(0);
    progress.show();
    while(ectrl->is_finished() == false) {
      nanosleep(&sleepcount, NULL);
      progress.setProgress(static_cast<int>((ectrl->position_in_seconds_exact() - progress_start) * 10.0));
    }
    toggle_triggered_state(false);
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nlibecasound error while processing event: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }

  // --------
  ENSURE(is_triggered() == false);
  // --------
}

void QEEvent::nonblocking_start(void) {
  // --------
  REQUIRE(ectrl->is_valid() == true);
  REQUIRE(ectrl->is_selected() == true);
  REQUIRE(is_triggered() == false);
  // --------

  ectrl->connect_chainsetup();
  ectrl->start();
  toggle_triggered_state(true);

  // --------
  ENSURE(is_triggered() == true || ectrl->is_running() == false);
  // --------
}

void QEEvent::set_input(const string& name) {
  // --------
  REQUIRE(name.empty() == false);
  // --------
  try {
    ectrl->add_audio_input(name);
    input_object = ectrl->get_audio_object();
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nlibecasound error while setting event input: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }

  // --------
  ENSURE(input_object != 0);
  // --------
}

void QEEvent::set_default_audio_format(const string& name) {
  // --------
  REQUIRE(name.empty() == false);
  // --------
  try {
    ectrl->select_audio_object(name);
    ECA_AUDIO_FORMAT aio_params = ectrl->get_audio_format();
    ectrl->set_default_audio_format(aio_params);
    //    cerr << "Setting sample rate of: " << aio_params.samples_per_second() << ".\n";
    ectrl->set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nlibecasound error while setting event input: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
}

void QEEvent::set_input_position(long int pos) {
  // --------
  REQUIRE(ectrl->is_running() == false);
  REQUIRE(input_object != 0);
  REQUIRE(pos >= 0);
  // --------
  input_object->seek_position_in_samples(pos);
  // --------
  ENSURE(pos == input_object->position_in_samples());
  // --------
}

void QEEvent::set_output(const string& name) {
  // --------
  REQUIRE(name.empty() == false);
  // --------
  try {
    ectrl->add_audio_output(name);
    ectrl->select_audio_object(name);
    output_object = ectrl->get_audio_object();
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nlibecasound error while setting event input: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  // --------
  ENSURE(output_object != 0);
  // --------
}

void QEEvent::set_output_position(long int pos) {
  // --------
  REQUIRE(ectrl->is_running() == false);
  REQUIRE(output_object != 0);
  REQUIRE(pos >= 0);
  // --------
  output_object->seek_position_in_samples(pos);
  // --------
  ENSURE(pos == output_object->position_in_samples());
  // --------
}

void QEEvent::set_length(long int pos) {
  // --------
  REQUIRE(ectrl->is_running() == false);
  // --------
  ectrl->set_chainsetup_processing_length_in_samples(pos);
}
