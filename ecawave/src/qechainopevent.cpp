// ------------------------------------------------------------------------
// qechainopevent.cpp: Process audio data with a chain operator 
//                     provided by libecasound
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

#include <sys/stat.h>
#include <unistd.h>

#include <qaccel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qpushbutton.h>
#include <qprogressdialog.h>
#include <qmessagebox.h>

#include <string>
#include <map>

#include "qechainopevent.h"

QEChainopEvent::QEChainopEvent (ECA_CONTROLLER* ctrl, 
				const string& input,
				const string& output,
				long int start_pos, 
				long int length,
				QWidget *parent, 
				const char *name) 
  : QDialog(parent, name, false),
    QEProcessEvent(ctrl),
    input_rep(input),
    output_rep(output),
    start_pos_rep(start_pos),
    length_rep(length) {

  init();
  init_layout();
}

void QEChainopEvent::restart(long int start_pos, long int length) { 
  start_pos_rep = start_pos;
  length_rep = length;
  if (mode == process_mode) process();
  else preview();
}

void QEChainopEvent::preview(void) {
  mode = preview_mode;
  init();
  toggle_valid_state(false);
  ectrl->add_chain("default");
  set_input(input_rep);
  set_input_position(start_pos_rep);
  set_length(length_rep);
  get_default_audio_format(input_rep);
  ectrl->add_default_output();
  toggle_valid_state(true);
  if (copinput != 0) {
    copinput->set_parameters();
    add_chain_operator(copinput->clone_result());
  }
  start(false);
}

void QEChainopEvent::process(void) {
  toggle_valid_state(false);
  create_output();
  if (mode != invalid) {
    mode = process_mode;
    init();
    ectrl->add_chain("default");
    set_input(input_rep);
    set_input_position(start_pos_rep);
    set_length(length_rep);
    get_default_audio_format(input_rep);
    set_output(output_rep);
    set_output_position(start_pos_rep);
    toggle_valid_state(true);
    if (copinput != 0) {
      copinput->set_parameters();
      add_chain_operator(copinput->clone_result());
    }
    start(true);
    emit finished();
    accept();
  }
}

void QEChainopEvent::create_output(void) {
  struct stat stattemp1;
  struct stat stattemp2;
  stat(input_rep.c_str(), &stattemp1);
  stat(output_rep.c_str(), &stattemp2);
  if (stattemp1.st_size != stattemp2.st_size) {
    copy_file(input_rep, output_rep);
    copy_file(input_rep + ".ews", output_rep + ".ews");
  }
  stat(input_rep.c_str(), &stattemp1);
  stat(output_rep.c_str(), &stattemp2);
  if (stattemp1.st_size != stattemp2.st_size) {
    QMessageBox* mbox = new QMessageBox(this, "mbox");
    mbox->information(this, "ecawave", QString("Error while creating temporary file ") + QString(output_rep.c_str()),0);
    mode = invalid;
  }
}

void QEChainopEvent::copy_file(const string& a, const string& b) {
  FILE *f1, *f2;
  f1 = fopen(a.c_str(), "r");
  f2 = fopen(b.c_str(), "w");
  char buffer[16384];

  if (!f1 || !f2) {
    QMessageBox* mbox = new QMessageBox(this, "mbox");
    mbox->information(this, "ecawave", QString("Error while creating temporary file ") + QString(output_rep.c_str()),0);
    mode = invalid;
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

void QEChainopEvent::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);

  copinput = new QEChainopInput(this); 
  top->addWidget(copinput);

  top->addSpacing(10);

  QBoxLayout* buttons = new QHBoxLayout();

  QPushButton* process_button = new QPushButton( "Pr(o)cess", this);
  buttons->addWidget(process_button);

  QPushButton* preview_button = new QPushButton( "(P)review", this);
  buttons->addWidget(preview_button);

  QPushButton* cancel_button = new QPushButton( "(C)ancel", this);
  buttons->addWidget(cancel_button);

  QObject::connect(process_button, SIGNAL(clicked()), this, SLOT(process()));
  QObject::connect(preview_button, SIGNAL(clicked()), this, SLOT(preview()));
  QObject::connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(ALT+Key_O), this, SLOT(process()));
  a->connectItem(a->insertItem(ALT+Key_P), this, SLOT(preview()));
  a->connectItem(a->insertItem(ALT+Key_C), this, SLOT(reject()));

  top->addLayout(buttons);
}
