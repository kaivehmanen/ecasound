// ------------------------------------------------------------------------
// eca-qtiodevdialog.cpp: Qt dialog widget that asks user for the data 
//                        necessary to add a new audio input/output object.
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

// #include <qwidget.h>
#include <qaccel.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>
#include <qdir.h>

#include "qeokcancelinput.h"

#include "eca-chain.h"
#include "eca-chainsetup.h"
#include "eca-qtiodevdialog.h"
#include "eca-debug.h"

QEIodevDialog::QEIodevDialog (const ECA_CHAINSETUP* csetup, QWidget *parent, const char *name) 
  : QDialog(parent, name, true), chainsetup(csetup) {

  current_dir_rep = "";

  setCaption("qtecasound - new audio input/output");
  QBoxLayout* topLayout = new QVBoxLayout( this );

  init_inout();
  init_chains();

  // file name input
  fnameinput = new QEFilenameInput(QEFilenameInput::browse_existing, this, "fnameinput");

  // audio format
  aformat = new QEAudioFormatInput(this, "aformat");

  // ok-cancel buttons:
  QEOkCancelInput* okcancel = new QEOkCancelInput(this, "okcancel");
  QObject::connect(okcancel, SIGNAL(ok()), this, SLOT(inputGiven()) );
  QObject::connect(okcancel, SIGNAL(cancel()), this, SLOT(reject()) );  

  // layouts
  topLayout->addWidget(fnameinput, 0);
  topLayout->addLayout(inout, 0);
  topLayout->addWidget(aformat, 0);
  topLayout->addLayout(chains, 0);
  topLayout->addWidget(okcancel, 0);

  init_shortcuts();
}

void QEIodevDialog::init_shortcuts(void) {
  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(ALT+Key_C), this,
  		 SLOT(reject()));

  a->connectItem(a->insertItem(ALT+Key_N), this,
  		 SLOT(set_input_mode()));

  a->connectItem(a->insertItem(ALT+Key_O), this,
  		 SLOT(inputGiven()));

  a->connectItem(a->insertItem(ALT+Key_T), chaininput,
  		 SLOT(setFocus()));

  a->connectItem(a->insertItem(ALT+Key_U), this,
  		 SLOT(set_output_mode()));
}

void QEIodevDialog::init_inout(void) {
  b_dir = new QVButtonGroup("Direction", this, "dirgroup");
  QRadioButton* dir_in = new QRadioButton("I(n)put", b_dir, "input");
  QRadioButton* dir_out = new QRadioButton("O(u)tput", b_dir, "output");
  b_dir->setButton(0);

  b_override = new QVButtonGroup("Format parameters", this, "oridegroup");
  QCheckBox* obox = new QCheckBox("Override input/output format parameters",b_override, "oride");

  inout = new QHBoxLayout();
  inout->addWidget(b_dir);
  inout->addWidget(b_override);
}

void QEIodevDialog::init_chains(void) {
  chaininput = new QListBox(this, "chains");
  chaininput->setSelectionMode(QListBox::Multi);

  vector<CHAIN*>::const_iterator p = chainsetup->chains.begin();
  while(p != chainsetup->chains.end()) {
    chaininput->insertItem((*p)->name().c_str());
    ++p;
  }
  chaininput->setSelected(0, true);

  chains = new QHBoxLayout();
  chains->addWidget(new QLabel("A(t)tach to chains:", this, 0));
  chains->addWidget(chaininput);
}

void QEIodevDialog::inputGiven(void) {
  r_filename = fnameinput->result_filename();
  ecadebug->msg(4, "(eca-qtiodevdialog) filename " + r_filename);

  if (b_dir->selected()->text() == "I(n)put") {
    r_dir = input;
  }
  else {
    r_dir = output;
  }

  if (b_override->selected() == 0) {
    ecadebug->msg(4, "(eca-qtiodevdialog) explicit format disabled");
    r_explicit = false;
  }
  else {
    ecadebug->msg(4, "(eca-qtiodevdialog) explicit format enabled");
    r_explicit = true;
  }

  aformat->update_results();
  r_aformat = aformat->audio_format();

  for(int n = 0; n < chaininput->count(); n++) {
    ecadebug->msg(4, "(eca-qtiodevdialog) checking chaininput " + kvu_numtostr(n));
    if (chaininput->isSelected(chaininput->item(n))) {
      r_chains.push_back(chainsetup->chains[n]->name());
      ecadebug->msg(4, "(eca-qtiodevdialog) adding chain " + r_chains.back());
    }
  }
  
  accept();
}
