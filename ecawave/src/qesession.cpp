// ------------------------------------------------------------------------
// qesession.cpp: Class representing an ecawave session
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
#include <sys/stat.h>
#include <unistd.h>

#include <qapplication.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qaccel.h>

#include <ecasound/eca-session.h>
#include <ecasound/eca-controller.h>
#include <ecasound/qebuttonrow.h>
#include <ecasound/eca-version.h>

#include "qefile.h"
#include "qeopenfiledialog.h"
#include "qesavefiledialog.h"
#include "qesession.h"

#include "qestatusbar.h"
#include "qeevent.h"
#include "qenonblockingevent.h"
#include "qeplayevent.h"
#include "qesaveevent.h"
#include "qechainopevent.h"
#include "qecopyevent.h"
#include "qepasteevent.h"
#include "qecutevent.h"

#include "version.h"

#define COMPILE_ALSA
#include <ecasound/audioio-alsa2.h>
#include <ecasound/qeaudioformatinput.h>
#include <ecasound/qeaudioobjectconfiguration.h>
#include <ecasound/qechainoperatorinput.h>
#include <ecasound/qefilenameinput.h>
#include <ecasound/qestringdialog.h>
#include <ecasound/qeobjectmap.h>
#include <ecasound/eca-static-object-maps.h>
#include <ecasound/audiofx_compressor.h>
#include <ecasound/qeokcancelinput.h>
#include <ecasound/qeoperatorconfiguration.h>

void QESession::show_event(void) {
  //  ALSA_PCM2_DEVICE alsa (0,0); QWidget* widget1 = new QEAudioObjectConfiguration(&alsa, 0, "test1");
  //  QWidget* widget2 = new QEAudioFormatInput(0, "test2");
  //  QWidget* widget2 = new QEChainOperatorInput(0, "test2");
  //  QWidget* widget4 = new QEObjectMap(&eca_chain_operator_map, 0, "test4");
  //  ADVANCED_COMPRESSOR cop; QWidget* widget6 = new QEOperatorConfiguration(&cop, 0, "test6");
  //  widget1->show();
}

QESession::QESession (const string& filename, 
		      const ECA_AUDIO_FORMAT& frm, 
		      bool use_wave_cache, 
		      bool force_refresh, 
		      bool direct_mode, 
		      QWidget *parent, 
		      const char *name)
        : QWidget( parent, name ),
	  state_rep(state_orig_file),
	  orig_filename_rep(filename),
	  file(0),
	  nb_event(0),
	  refresh_toggle_rep(force_refresh),
	  wcache_toggle_rep(use_wave_cache),
          direct_mode_rep(direct_mode) {
  setCaption(string("ecawave " + 
		    ecawave_version + 
		    " [" +
		    ecasound_library_version +
		    "] . (C) 1999-2000 Kai Vehmanen . ").c_str());

  active_filename_rep = orig_filename_rep;
  if (orig_filename_rep.empty() == true) {
    active_filename_rep = string(tmpnam(NULL)) + ".wav";
    state_rep = state_new_file;
  }

  if (direct_mode_rep == true) {
    state_rep = state_orig_direct;
    statusbar->toggle_editing(true);
  }

  esession = new ECA_SESSION();
  ectrl = new ECA_CONTROLLER(esession);

  auto_ptr<ECA_SESSION> p (esession);
  auto_esession = p;

  auto_ptr<ECA_CONTROLLER> q (ectrl);
  auto_ectrl = q;

  vlayout = new QVBoxLayout(this);
  init_layout();
  assert(file != 0);
  file->set_audio_format(frm);

  startTimer(100);

  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(position_update()));
  timer->start(500, false);

  start_pos = 0;
  sel_length = 0;

  // --------
  ENSURE(file != 0);
  ENSURE(active_filename_rep.empty() == false);
  // --------
}

void QESession::timerEvent( QTimerEvent * ) {
  if (nb_event != 0) {
    if (nb_event->is_finished() == true &&
	nb_event->is_triggered() == true)
      nb_event->stop();
  }
}

QESession::~QESession(void) {
  if (nb_event != 0) {
    if (nb_event->is_triggered() &&
  	ectrl->is_running()) nb_event->stop();
    delete nb_event;
  }

  remove_temps();
 
  while(child_sessions.size() > 0) {
    delete child_sessions.back();
    child_sessions.pop_back();
  }
}

void QESession::remove_temps(void) {
  if (state_rep == state_edit_file || 
      state_rep == state_new_file) {
    assert(active_filename_rep.empty() != true);
    remove(active_filename_rep.c_str());
    remove((active_filename_rep + ".ews").c_str());
  }
}

void QESession::position_update(void) {
  static bool toggle = false;
  long int pos = 0;
  if (ectrl->is_running() == true) {
    toggle = true;
    if (nb_event != 0)
      pos = nb_event->position_in_samples();
    else
      pos = ectrl->position_in_samples() - ectrl->get_chainsetup()->buffersize();
  }
  if (toggle == true) {
    if (pos > 0) file->current_position(pos);
    else file->current_position(0);
  }
  if (ectrl->is_running() != true) toggle = false;
}

void QESession::init_layout(void) {
  buttonrow = new QEButtonRow(this, "buttonrow");
  buttonrow->add_button(new QPushButton("(N)ew session",buttonrow), 
		       ALT+Key_N,
		       this, SLOT(new_session()));
  buttonrow->add_button(new QPushButton("New (f)ile",buttonrow), 
		       ALT+Key_F, this, SLOT(new_file()));
  buttonrow->add_button(new QPushButton("(O)pen",buttonrow), 
		       ALT+Key_O, this, SLOT(open_file()));
  buttonrow->add_button(new QPushButton("Sa(v)e",buttonrow), 
		       ALT+Key_V, this, SLOT(save_event()));
  buttonrow->add_button(new QPushButton("Save (a)s",buttonrow), 
		       ALT+Key_A, this, SLOT(save_as_event()));
  buttonrow->add_button(new QPushButton("(Q)uit",buttonrow), ALT+Key_Q, this, SLOT(close()));
  vlayout->addWidget(buttonrow);

  buttonrow2 = new QEButtonRow(this, "buttonrow2");
  buttonrow2->add_button(new QPushButton("S(t)art",buttonrow2), ALT+Key_T,
			this, SLOT(play_event()));
  buttonrow2->add_button(new QPushButton("(S)top",buttonrow2), ALT+Key_S,
			this, SLOT(stop_event()));
  buttonrow2->add_button(new QPushButton("(E)ffect",buttonrow2), ALT+Key_E,
			this, SLOT(effect_event()));
  buttonrow2->add_button(new QPushButton("Cop(y)",buttonrow2), ALT+Key_Y,
			this, SLOT(copy_event()));
  buttonrow2->add_button(new QPushButton("C(u)t",buttonrow2), ALT+Key_U,
			this, SLOT(cut_event()));
  buttonrow2->add_button(new QPushButton("(P)aste",buttonrow2), ALT+Key_P,
			this, SLOT(paste_event()));
  vlayout->addWidget(buttonrow2);

  QAccel* a = new QAccel (this);
  a->connectItem(a->insertItem(ALT+Key_D), this, SLOT(debug_event()));
  a->connectItem(a->insertItem(ALT+Key_B), this, SLOT(show_event()));

  if (state_rep == state_orig_file ||
      state_rep == state_orig_direct) 
    file = new QEFile(active_filename_rep,
		      wcache_toggle_rep,
		      refresh_toggle_rep, 
		      this, 
		      "sessionfile");
  else 
    file = new QEFile(this, 
		      "sessionfile");

  vlayout->addWidget(file,1);

  statusbar = new QEStatusBar(ectrl, active_filename_rep, this);
  statusbar->visible_area(ECA_AUDIO_TIME(0, file->samples_per_second()),
			  ECA_AUDIO_TIME(file->length(), file->samples_per_second()));
  vlayout->addWidget(statusbar);

  QObject::connect(file, 
		   SIGNAL(visible_area_changed(ECA_AUDIO_TIME,
					       ECA_AUDIO_TIME)), 
		   statusbar, 
		   SLOT(visible_area(ECA_AUDIO_TIME, ECA_AUDIO_TIME)));
  QObject::connect(file, 
		   SIGNAL(marked_area_changed(ECA_AUDIO_TIME,
					      ECA_AUDIO_TIME)), 
		   statusbar, 
		   SLOT(marked_area(ECA_AUDIO_TIME, ECA_AUDIO_TIME)));
  QObject::connect(file, 
		   SIGNAL(current_position_changed(ECA_AUDIO_TIME)), 
		   statusbar,
		   SLOT(current_position(ECA_AUDIO_TIME)));
  QObject::connect(this, 
		   SIGNAL(filename_changed(const string&)), 
		   statusbar,
		   SLOT(filename(const string&)));
}

void QESession::debug_event(void) {
  cerr << "----------------" << endl;
  cerr << "- ecawave-debug:" << endl;
  cerr << "orig_filename_rep: " << orig_filename_rep << endl;;
  cerr << "active_filename_rep: " << active_filename_rep << endl;;

  cerr << "start_pos: " << start_pos << endl;
  cerr << "sel_length: " << sel_length << endl;

  cerr << "child_sessions size: " << child_sessions.size() << endl;
  
  if (file != 0) cerr << "qefile-filename: " << file->filename() <<
		   endl;
  else cerr << "qefile not created." << endl;

  if (refresh_toggle_rep) cerr << "refresh_toggle_rep: true" << endl;
  else cerr << "refresh_toggle_rep: false" << endl;

  if (wcache_toggle_rep) cerr << "wcache_toggle_rep: true" << endl;
  else cerr << "wcache_toggle_rep: false" << endl;

  if (direct_mode_rep) cerr << "direct_mode_rep: true" << endl;
  else cerr << "direct_mode_rep: false" << endl;
 
  if (ectrl == 0) cerr << "ectrl not created." << endl;
  else {
    cerr << "ectrl status: " << ectrl->engine_status() << endl;
    cerr << "ectrl position: " << ectrl->position_in_seconds_exact()
	 << "sec" << endl;
  }
}

void QESession::new_session(void) {
  child_sessions.push_back(new QESession());
  child_sessions.back()->show();
}

void QESession::new_file(void) {
  stop_event();
  remove_temps();
  int old_height_hint = file->sizeHint().height();
  file->new_file();
  orig_filename_rep = "";
  active_filename_rep = string(tmpnam(NULL)) + ".wav";
  state_rep = state_new_file;
  emit filename_changed(active_filename_rep);
  resize(width(), height() + file->sizeHint().height() - old_height_hint);
}


void QESession::open_file(void) {
  QEOpenFileDialog* fdialog = new QEOpenFileDialog();
  if (fdialog->exec() == QEOpenFileDialog::Accepted) {
    stop_event();
    remove_temps();

    ECA_AUDIO_FORMAT frm (fdialog->result_channels(), 
			  (long int)fdialog->result_srate(), 
			  ECA_AUDIO_FORMAT::sfmt_s16);
    if (fdialog->result_bits() == 8)
      frm.set_sample_format(ECA_AUDIO_FORMAT::sfmt_u8);

    file->toggle_wave_cache(fdialog->result_wave_cache_toggle());
    file->toggle_cache_refresh(fdialog->result_cache_refresh_toggle());
    direct_mode_rep = fdialog->result_direct_mode_toggle();
    int old_height_hint = file->sizeHint().height();
    remove_temps();
    file->new_file(fdialog->result_filename());
    state_rep = state_orig_file;
    if (direct_mode_rep == true) {
      state_rep = state_orig_direct;
      statusbar->toggle_editing(true);
    }
    orig_filename_rep = file->filename();
    active_filename_rep = orig_filename_rep;
    emit filename_changed(orig_filename_rep);
    resize(width(), height() + file->sizeHint().height() - old_height_hint);
  }
}

void QESession::prepare_event(void) { 
  // --------
  REQUIRE(file != 0);
  // --------

  start_pos = 0;
  sel_length = 0;
  if (file->is_valid() == true) {
    if (file->is_marked() == true) {
      start_pos = file->marked_area_start();
      sel_length = file->marked_area_end() - file->marked_area_start(); 
    }
    else {
      start_pos = file->current_position();
      sel_length = file->length();
    }
    
    if (start_pos > file->length() ||
	start_pos < 0) {
      start_pos = 0;
    }
    if (sel_length == 0 ||
	start_pos + sel_length > file->length()) {
      sel_length = file->length() - start_pos;
    }
  }

  // --------
  ENSURE(start_pos >= 0);
  ENSURE(sel_length >= 0);
  ENSURE(start_pos + sel_length <= file->length());
  // --------
}

void QESession::effect_event(void) {
  stop_event();
  prepare_event();
  if (file->is_valid() == false) return;
  prepare_temp();
  if (state_rep == state_invalid) return;

  QEChainopEvent* p = new QEChainopEvent(ectrl, active_filename_rep, active_filename_rep,
					 start_pos, sel_length);

  QObject::connect(p, SIGNAL(finished()), this, SLOT(update_wave_data()));
  p->show();
  nb_event = p;
}

void QESession::update_wave_data(void) {
  // --------
  // require:
  assert(file != 0);
  // --------

  if (file != 0) {
    if (file->filename() != active_filename_rep) {
      file->new_file(active_filename_rep);
      state_rep = state_new_file;
    }
    file->update_wave_form_data();
    file->emit_status();
  }
}

bool QESession::temp_file_created(void) {
  struct stat stattemp1;
  struct stat stattemp2;
  stat(active_filename_rep.c_str(), &stattemp1);
  stat(active_filename_rep.c_str(), &stattemp2);
  if (stattemp1.st_size != stattemp2.st_size) return(false);
  return(true);
}

void QESession::prepare_temp(void) {
  if (state_rep == state_orig_file) {
    string temp = string(tmpnam(NULL)) + ".wav";

    struct stat stattemp1;
    struct stat stattemp2;
    stat(active_filename_rep.c_str(), &stattemp1);
    stat(temp.c_str(), &stattemp2);
    if (stattemp1.st_size != stattemp2.st_size) {
      QECopyEvent p (ectrl, active_filename_rep, temp, 0, 0);
      if (p.is_valid() == true) {
	p.start();
      }
    }
    if (stattemp1.st_size != stattemp2.st_size) 
      copy_file(active_filename_rep + ".ews", temp + ".ews");
    stat(temp.c_str(), &stattemp2);
    if (stattemp2.st_size == 0) {
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      mbox->information(this, "ecawave", QString("Error while creating temporary file ") + QString(temp.c_str()),0);
      state_rep = state_invalid;
    }
    else {
      active_filename_rep = temp;
      int old_height_hint = file->sizeHint().height();
      file->new_file(active_filename_rep);
      state_rep = state_edit_file;
      statusbar->toggle_editing(true);
      resize(width(), height() + file->sizeHint().height() - old_height_hint);
    }
  }
}

void QESession::play_event(void) { 
  stop_event();
  prepare_event();
  if (file->is_valid() == false) return;

  QEPlayEvent* p;
  p = new QEPlayEvent(ectrl, active_filename_rep, ecawaverc.resource("default-output"), start_pos, sel_length);

  if (p->is_valid() == true) {
    p->start();
    nb_event = p;
  }
  else 
    nb_event = 0;
}

void QESession::save_event(void) { 
  if (state_rep == state_orig_file) {
    QMessageBox* mbox = new QMessageBox(this, "mbox");
    mbox->information(this, "ecawave", "File not modified, save file cancelled.",0);
    return;
  }

  if (state_rep == state_new_file) save_as_event();
  else {
    stop_event();
    QESaveEvent p (ectrl, active_filename_rep, orig_filename_rep);
    if (p.is_valid() == true) p.start();
  }
}

void QESession::save_as_event(void) { 
  QESaveFileDialog* fdialog = new QESaveFileDialog();
  if (fdialog->exec() == QESaveFileDialog::Accepted) {
    stop_event();

    QESaveEvent p (ectrl, active_filename_rep, fdialog->result_filename());
    if (p.is_valid() == true) {
      p.start();
    }
  }
}

void QESession::copy_event(void) { 
  stop_event();
  prepare_event();
  if (file->is_valid() == false) return;

  QECopyEvent p (ectrl, active_filename_rep, ecawaverc.resource("clipboard-file"), start_pos, sel_length);
  
  if (p.is_valid() == true) {
    p.start();
  }
}

void QESession::paste_event(void) { 
  stop_event();
  prepare_event();
  prepare_temp();
  if (state_rep == state_invalid) return;
  assert(state_rep != state_orig_file);

  QEPasteEvent p (ectrl, 
		  ecawaverc.resource("clipboard-file"),
		  active_filename_rep, 
		  start_pos);
  if (p.is_valid() == true) {
    p.start();
    update_wave_data();
  }
}

void QESession::cut_event(void) { 
  stop_event();
  prepare_event();
  prepare_temp();
  if (state_rep == state_invalid) return;
  assert(state_rep != state_orig_file);

  QECutEvent p (ectrl, 
		active_filename_rep,
		ecawaverc.resource("clipboard-file"),
		start_pos,
		sel_length);
  if (p.is_valid() == true) {
    p.start();
    file->unmark();
    update_wave_data();
  }
}

void QESession::stop_event(void) { 
  if (nb_event != 0) {
    if (ectrl->is_running()) nb_event->stop();
    delete nb_event;
  }
  nb_event = 0;
}

void QESession::copy_file(const string& a, const string& b) {
  FILE *f1, *f2;
  f1 = fopen(a.c_str(), "r");
  f2 = fopen(b.c_str(), "w");
  char buffer[16384];

  if (!f1 || !f2) {
    QMessageBox* mbox = new QMessageBox(this, "mbox");
    mbox->information(this, "ecawave", QString("Error while creating temporary file ") + QString(b.c_str()),0);
    return;
  }

  fseek(f1, 0, SEEK_END);
  long int len = ftell(f1);
  fseek(f1, 0, SEEK_SET);

  QProgressDialog progress ("Creating temporary file for processing...", 0,
			    (int)(len / 1000), 0, 0, true);
  progress.setProgress(0);
  progress.show();

  while(!feof(f1) && !ferror(f2)) {
    fwrite(buffer, fread(buffer, 1, 16384, f1), 1, f2);
    progress.setProgress((int)(ftell(f1) / 1000));
  }
  fclose(f1);
  fclose(f2);
}
