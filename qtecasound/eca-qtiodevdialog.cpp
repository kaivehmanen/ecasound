// ------------------------------------------------------------------------
// eca-qtiodevdialog.cpp: Qt dialog widget that asks user for the data 
//                        necessary to add a new audio input/output object.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include <qvbuttongroup.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>
#include <qdir.h>

#include "eca-chain.h"
#include "eca-chainsetup.h"
#include "eca-qtiodevdialog.h"

#include "eca-debug.h"

QEIodevDialog::QEIodevDialog (const ECA_CHAINSETUP* csetup, QWidget *parent, const char *name) 
  : QDialog(parent, name, true), chainsetup(csetup) {

  setMinimumSize(0, 300);

  current_dir_rep = "";

  setCaption("qtecasound - new audio input/output");

  QBoxLayout* topLayout = new QVBoxLayout( this );

  init_filename();
  init_inout();
  init_format();
  init_chains();
  // ok-cancel buttons:
  QBoxLayout* buttons = new QHBoxLayout();
  QPushButton *ok, *cancel;
  ok = new QPushButton( "(O)K", this );
  buttons->addWidget(ok);

  cancel = new QPushButton( "(C)ancel", this );
  buttons->addWidget(cancel);

  QObject::connect( ok, SIGNAL(clicked()), SLOT(inputGiven()) );
  QObject::connect( cancel, SIGNAL(clicked()), SLOT(reject()) );    

  // ok-cancel buttons:
  topLayout->addLayout(filename, 0);
  topLayout->addLayout(inout, 0);
  topLayout->addLayout(format, 0);
  topLayout->addLayout(chains, 0);
  topLayout->addLayout(buttons, 0);

  init_shortcuts();
}

void QEIodevDialog::init_shortcuts(void) {
  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(CTRL+Key_8), this,
  		 SLOT(handle_key_8()));
  
  a->connectItem(a->insertItem(CTRL+Key_6), this,
  		 SLOT(handle_key_6()));

  a->connectItem(a->insertItem(CTRL+Key_A), chaininput,
  		 SLOT(setFocus()));

  a->connectItem(a->insertItem(CTRL+Key_B), this,
  		 SLOT(button_browse()));

  a->connectItem(a->insertItem(CTRL+Key_C), this,
  		 SLOT(reject()));

  a->connectItem(a->insertItem(CTRL+Key_I), this,
  		 SLOT(handle_key_i()));

  a->connectItem(a->insertItem(CTRL+Key_O), this,
  		 SLOT(inputGiven()));

  a->connectItem(a->insertItem(CTRL+Key_U), this,
  		 SLOT(handle_key_u()));

  a->connectItem(a->insertItem(CTRL+Key_R), srateinput,
		 SLOT(setFocus()));
}

void QEIodevDialog::init_filename(void) {
  filename = new QHBoxLayout();

  filename->add(new QLabel("(F)ile name: ", this, 0));

  filenameinput = new QLineEdit(this, "filenameinput");
  filenameinput->setMinimumSize(300,0);
  filename->addWidget(filenameinput);

  filenamebrowse = new QPushButton("(B)rowse", this, 0);
  filename->addWidget(filenamebrowse);

  QObject::connect(filenamebrowse, SIGNAL(clicked()), this, SLOT(button_browse()));
}

void QEIodevDialog::init_inout(void) {
  b_dir = new QVButtonGroup("Direction", this, "dirgroup");
  QRadioButton* dir_in = new QRadioButton("(I)nput", b_dir, "input");
  QRadioButton* dir_out = new QRadioButton("O(u)tput", b_dir, "output");
  b_dir->setButton(0);

  b_override = new QVButtonGroup("Format parameters", this, "oridegroup");
  QCheckBox* oride = new QCheckBox("Override input/output format parameters",b_override, "oride");

  inout = new QHBoxLayout();
  inout->addWidget(b_dir);
  inout->addWidget(b_override);
}

void QEIodevDialog::init_format(void) {
  b_bits = new QVButtonGroup("Bits", this, "bits");
  QRadioButton* bit8 = new QRadioButton("(8)bit", b_bits, "8bit");
  QRadioButton* bit16 = new QRadioButton("1(6)bit", b_bits, "16bit");
  b_bits->setButton(1);

  b_channels = new QVButtonGroup("Channels", this, "channels");
  QRadioButton* mono = new QRadioButton("(M)ono", b_channels, "mono");
  QRadioButton* stereo = new QRadioButton("(S)tereo", b_channels, "stereo");
  b_channels->setButton(1);


  QVGroupBox* sratebox = new QVGroupBox("Sample (r)ate", this, 0);
  //  QLabel* sratelabel = new QLabel("Sample (r)ate: ", sratebox, "srlabel");
  srateinput = new QSpinBox(4000, 48000, 10, sratebox, "srate");
  srateinput->setValue(44100);

  //  QBoxLayout* srate = new QVBoxLayout();
  //  srate->addWidget(sratelabel);
  //  srate->addWidget(srateinput);

  format = new QHBoxLayout();
  format->addWidget(b_bits);
  format->addWidget(b_channels);
  format->addWidget(sratebox);
  //  format->addLayout(srate);
}

void QEIodevDialog::init_chains(void) {
  chaininput = new QListBox(this, "chains");
  chaininput->setSelectionMode(QListBox::Multi);
  chaininput->setFrameStyle(b_channels->frameStyle());
  chaininput->setMargin(b_channels->margin());

  vector<CHAIN*>::const_iterator p = chainsetup->chains.begin();
  while(p != chainsetup->chains.end()) {
    chaininput->insertItem((*p)->name().c_str());
    ++p;
  }
  chains = new QHBoxLayout();
  chains->addWidget(new QLabel("(A)ttach to chains:", this, 0));
  chains->addWidget(chaininput);
}

void QEIodevDialog::inputGiven(void) {
  r_filename = filenameinput->text();
  ecadebug->msg(4, "(eca-qtiodevdialog) filename " + r_filename);

  if (b_dir->selected()->text() == "(I)nput") {
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

  if (b_bits->selected()->text() == "(8)bit") r_bits = 8;
  else if (b_bits->selected()->text() == "1(6)bit") r_bits = 16;
  ecadebug->msg(4, "(eca-qtiodevdialog) bits " + kvu_numtostr(r_bits));

  if (b_channels->selected()->text() == "(M)ono") r_channels = 1;
  else if (b_channels->selected()->text() == "(S)tereo") r_channels = 2;
  ecadebug->msg(4, "(eca-qtiodevdialog) channels " + kvu_numtostr(r_channels));

  r_srate = srateinput->value();
  ecadebug->msg(4, "(eca-qtiodevdialog) srate " + kvu_numtostr(r_srate));

  //  chaininput->setSelected(0, true);
  //  cerr << ":" << chaininput->item(0)->text() << "\n";
  for(int n = 0; n < chaininput->count(); n++) {
    ecadebug->msg(4, "(eca-qtiodevdialog) checking chaininput " + kvu_numtostr(n));
    if (chaininput->isSelected(chaininput->item(n))) {
      r_chains.push_back(chainsetup->chains[n]->name());
      ecadebug->msg(4, "(eca-qtiodevdialog) adding chain " + r_chains.back());
    }
  }
  
  accept();
}

void QEIodevDialog::button_browse(void) { 
  QFileDialog* fdialog = new QFileDialog(0, 0, true);
  fdialog->setMode(QFileDialog::ExistingFile);
  fdialog->setCaption("Select file or device");
  fdialog->setDir(current_dir_rep.c_str());
  fdialog->exec();
  //  if (current_dir.exists()) fdialog->setDir(current_dir);
  //  QString fname = fdialog->getSaveFileName();
  //  QString fname = QFileDialog::getSaveFileName(0, 0, 0, "Select file or device");
  QString fname = fdialog->selectedFile();
  if (!fname.isEmpty()) {
    current_dir_rep = string(fdialog->dirPath().latin1());
    filenameinput->setText(fname);
  }
}

void QEIodevDialog::keyPressEvent(QKeyEvent* kevent) {
  handle_key(kevent->ascii());
  kevent->ignore();
}

// void QEIodevDialog::handle_key(char c) {
//   handle_key((int)c);
// }

void QEIodevDialog::handle_key(int c) {
  switch(tolower(c)) {
  case '8': 
    {
      b_bits->setButton(0);
      break;
    }
  case '6': 
    {
      b_bits->setButton(1);
      break;
    }
  case 'a': 
    {
      chaininput->setFocus();
      break;
    }
  case 'b': 
    {
      button_browse();
      break;
    }
  case 'c': 
    {
      reject();
      break;
    }
  case 'i': 
    {
      b_dir->setButton(0);
      break;
    }
  case 'm': 
    {
      b_channels->setButton(0);
      break;
    }
  case 'o': 
    {
      inputGiven();
      break;
    }
 
  case 'r': 
    {
      srateinput->setFocus();
      break;
    }
  case 's': 
    {
      b_channels->setButton(1);
      break;
    }
  case 'u': 
    {
      b_dir->setButton(1);
      break;
    }
  default: { }
  }
}
