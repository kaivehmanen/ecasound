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

#include <ecasound/eca-session.h>
#include <ecasound/eca-controller.h>
#include <ecasound/qebuttonrow.h>

#include "qefile.h"
#include "qeopenfiledialog.h"
#include "qesavefiledialog.h"
#include "qesession.h"

#include "qestatusbar.h"
#include "qeevent.h"
#include "qeplayevent.h"
#include "qesaveevent.h"
#include "qechainopevent.h"

#include "version.h"

QESession::QESession (const string& filename, 
		      const ECA_AUDIO_FORMAT& frm, 
		      bool use_wave_cache, 
		      bool force_refresh, 
		      bool direct_mode, 
		      QWidget *parent, 
		      const char *name)
        : QWidget( parent, name ),
	  filename_rep(filename),
	  refresh_toggle_rep(force_refresh),
	  wcache_toggle_rep(use_wave_cache),
          direct_mode_rep(direct_mode) {
  event = 0;
  editing_rep = false;

  if (direct_mode_rep == true) 
    tempfile_rep = filename_rep;
  else
    tempfile_rep = "";

  ecawaverc.load();

  esession = new ECA_SESSION();
  ectrl = new ECA_CONTROLLER(esession);

  auto_ptr<ECA_SESSION> p (esession);
  auto_esession = p;

  auto_ptr<ECA_CONTROLLER> q (ectrl);
  auto_ectrl = q;

  vlayout = new QVBoxLayout(this);
  file = 0;
  init_layout();
  assert(file != 0);
  file->set_audio_format(frm);
   
  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(position_update()));
  timer->start( 500, false);

  // --------
  ENSURE(file != 0);
  // --------
}

QESession::~QESession(void) {
  if (event != 0) {
    if (event->is_triggered() &&
  	ectrl->is_running()) event->stop();
    delete event;
  }

  if (tempfile_rep.empty() == false) {
    remove(tempfile_rep.c_str());
    remove((tempfile_rep + ".ews").c_str());
  }

  while(child_sessions.size() > 0) {
    delete child_sessions.back();
    child_sessions.pop_back();
  }

  ecawaverc.save();
}

void QESession::position_update(void) {
  static bool toggle = false;
  if (ectrl->is_running()) {
    toggle = true;
    file->current_position(start_pos + ectrl->position_in_samples() - ectrl->get_chainsetup()->buffersize());
  }
  else {
    if (toggle == true) {
      file->current_position(0);
      toggle = false;
    }
    else 
      toggle = false;
  }
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
  buttonrow->add_button(new QPushButton("(C)lose",buttonrow), 
		       ALT+Key_C, this, SLOT(close_file()));
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
  vlayout->addWidget(buttonrow2);

  if (filename_rep.empty() == false) file = new QEFile(filename_rep,
						       wcache_toggle_rep,
						       refresh_toggle_rep, 
						       this, 
						       "sessionfile");
  else file = new QEFile(this, "sessionfile");
  QObject::connect(file, SIGNAL(selection_changed()), this, SLOT(selection_update()));

  vlayout->addWidget(file,1);

  statusbar = new QEStatusBar(ectrl, filename_rep, this);
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
}

void QESession::new_session(void) {
  child_sessions.push_back(new QESession());
  child_sessions.back()->show();
}

void QESession::new_file(void) {
  stop_event();
  child_sessions.push_back(new QESession("", ECA_AUDIO_FORMAT()));
  child_sessions.back()->show();
  child_sessions.back()->setGeometry(x(), y(), width(), height());
  qApp->setMainWidget(child_sessions.back());
  close(false);
}

void QESession::close_file(void) {
  stop_event();
  child_sessions.push_back(new QESession());
  child_sessions.back()->show();
  qApp->setMainWidget(child_sessions.back());
  close(false);
}

void QESession::open_file(void) {
  QEOpenFileDialog* fdialog = new QEOpenFileDialog();
  if (fdialog->exec() == QEOpenFileDialog::Accepted) {
    stop_event();

    ECA_AUDIO_FORMAT frm (fdialog->result_channels(), 
			  (long int)fdialog->result_srate(), 
			  ECA_AUDIO_FORMAT::sfmt_s16);
    if (fdialog->result_bits() == 8)
      frm.set_sample_format(ECA_AUDIO_FORMAT::sfmt_u8);

    child_sessions.push_back(new QESession(fdialog->result_filename(), 
					   frm,
					   fdialog->result_wave_cache_toggle(),
					   fdialog->result_cache_refresh_toggle(),
					   fdialog->result_direct_mode_toggle()));
    child_sessions.back()->setGeometry(x(), y(), width(), height());
    child_sessions.back()->show();
    qApp->setMainWidget(child_sessions.back());
    close(false);
  }
}

void QESession::prepare_event(void) { 
  // --------
  REQUIRE(file != 0);
  // --------

  if (file->is_valid() == false) {
    QMessageBox* mbox = new QMessageBox(this, "mbox");
    mbox->information(this, "ecawave", "Invalid file; unable to continue.",0);
    return;
  }

  start_pos = file->current_position();
  sel_length = file->selection_length();

  //  cerr << "Range: " << start_pos << " len " << sel_length << ".\n";
  if (start_pos > file->length() ||
      start_pos < 0) {
    start_pos = 0;
  }
  if (sel_length == 0 ||
      start_pos + sel_length > file->length()) {
    sel_length = file->length() - start_pos;
  }
  //  cerr << "Range2: " << start_pos << " len " << sel_length << ".\n";

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

  QEChainopEvent* p;
  if (tempfile_rep.empty() == true) {
    tempfile_rep = string(tmpnam(NULL)) + ".wav";
    p = new QEChainopEvent(ectrl, filename_rep, tempfile_rep, start_pos, sel_length);
    edit_start = start_pos;
    edit_length = sel_length;
  }
  else {
    p = new QEChainopEvent(ectrl, tempfile_rep, tempfile_rep,
			   start_pos, sel_length);
    if (start_pos < edit_start) edit_start = start_pos;
    if (sel_length > edit_length) edit_length = sel_length;
  }
  QObject::connect(p, SIGNAL(finished()), this, SLOT(update_wave_data()));
  p->show();
  event = p;
}

void QESession::update_wave_data(void) {
  if (file == 0) return;

  if (editing_rep == false && direct_mode_rep == false) {
    file->open(tempfile_rep);
  }
  else {
    file->update_wave_form_data();
  }

  if (editing_rep == false) {
    editing_rep = true;
    statusbar->toggle_editing(true);
  }
}

bool QESession::temp_file_created(void) {
  struct stat stattemp1;
  struct stat stattemp2;
  stat(filename_rep.c_str(), &stattemp1);
  stat(tempfile_rep.c_str(), &stattemp2);
  if (stattemp1.st_size != stattemp2.st_size) return(false);
  return(true);
}

void QESession::play_event(void) { 
  stop_event();
  prepare_event();
  if (file->is_valid() == false) return;

  QEPlayEvent* p;
  if (temp_file_created() == true)
    p = new QEPlayEvent(ectrl, tempfile_rep, ecawaverc.resource("default-output"), start_pos, sel_length);
  else
    p = new QEPlayEvent(ectrl, filename_rep, ecawaverc.resource("default-output"), start_pos, sel_length);

  if (p->is_valid() == true) {
    p->start();
    event = p;
  }
  else 
    event = 0;
}

void QESession::save_event(void) { 
  if (temp_file_created() == false) {
    QMessageBox* mbox = new QMessageBox(this, "mbox");
    mbox->information(this, "ecawave", "File not modified, save file cancelled.",0);
    return;
  }

  stop_event();

  QESaveEvent* p = new QESaveEvent(ectrl, tempfile_rep, filename_rep, edit_start, edit_length);

  if (p->is_valid() == true) {
    p->start(true);
    event = p;
  }
  else 
    event = 0;
}

void QESession::save_as_event(void) { 
  QESaveFileDialog* fdialog = new QESaveFileDialog();
  if (fdialog->exec() == QESaveFileDialog::Accepted) {
    stop_event();

    QESaveEvent* p;
    if (temp_file_created() == true) 
      p = new QESaveEvent(ectrl, tempfile_rep, fdialog->result_filename(), 0, file->length());
    else
      p = new QESaveEvent(ectrl, filename_rep, fdialog->result_filename(), 0, file->length());

    if (p->is_valid() == true) {
      p->start(true);
      event = p;
    }
    else 
      event = 0;
  }
}

void QESession::stop_event(void) { 
  if (event != 0) {
    if (ectrl->is_running()) event->stop();
    delete event;
  }
  event = 0;
}

void QESession::selection_update(void) {
//    if (event != 0) {
//      prepare_event();
//      event->restart(start_pos, sel_length);
//    }
}

