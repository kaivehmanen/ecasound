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

QEChainopEvent::QEChainopEvent (ECA_CONTROL* ctrl, 
				const string& input,
				const string& output,
				long int start_pos, 
				long int length,
				QWidget *parent, 
				const char *name) 
  : QDialog(parent, name, false),
    QENonblockingEvent(ctrl),
    ectrl(ctrl),
    input_rep(input),
    output_rep(output),
    start_pos_rep(start_pos),
    length_rep(length) {
  init_layout();
}

void QEChainopEvent::restart(long int start_pos, long int length) { 
  start_pos_rep = start_pos;
  length_rep = length;
  if (mode == process_mode) process();
  else preview();
}

long int QEChainopEvent::position_in_samples(void) const {
  if (ectrl->is_running() == true)
    return(start_pos_rep + ectrl->position_in_samples() - ectrl->get_chainsetup()->buffersize());
  return(start_pos_rep);
}

void QEChainopEvent::preview(void) {
  mode = preview_mode;
  init("chainopevent-preview", "default");
  set_input(input_rep);
  set_input_position(start_pos_rep);
  set_length(length_rep);
  set_default_audio_format(input_rep);
  ectrl->add_default_output();
  if (copinput != 0) {
    copinput->update_results();
    OPERATOR* c = dynamic_cast<OPERATOR*>(copinput->result());
    c = c->clone();
    ectrl->add_chain_operator(dynamic_cast<CHAIN_OPERATOR*>(c));
    // parametrien asetus!!!
  }
  start();
}

void QEChainopEvent::process(void) {
  mode = process_mode;
  init("chainopevent-process", "default");
  set_input(input_rep);
  set_input_position(start_pos_rep);
  set_length(length_rep);
  set_default_audio_format(input_rep);
  set_output(output_rep);
  set_output_position(start_pos_rep);
  
  if (copinput != 0) {
      copinput->update_results();
      OPERATOR* c = dynamic_cast<OPERATOR*>(copinput->result());
      c = c->clone();
      ectrl->add_chain_operator(dynamic_cast<CHAIN_OPERATOR*>(c));
    // parametrien asetus!!!
  }
  blocking_start();
  emit finished();
  accept();
}


void QEChainopEvent::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);

  copinput = new QEChainOperatorInput(this, "qechainop"); 
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
