// ------------------------------------------------------------------------
// eca-qtchainselectdialog.cpp: Qt dialog widget for selecting which chains
//                              are attached to a audio input/output.
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

#include <qaccel.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>

#include "eca-chain.h"
#include "eca-qtchainselectdialog.h"

#include "eca-debug.h"

QEChainselectDialog::QEChainselectDialog (const ECA_CHAINSETUP* csetup, QWidget *parent=0, const char *name=0) 
  : QDialog(parent, name, true), chainsetup(csetup) {

  setMinimumSize(300, 300);

  setCaption("qtecasound - select chains");

  QBoxLayout* topLayout = new QVBoxLayout( this );

  init_chains();

  QBoxLayout* buttons = new QHBoxLayout();
  QPushButton *ok, *cancel;
  ok = new QPushButton( "(O)K", this );
  buttons->addWidget(ok);

  cancel = new QPushButton( "(C)ancel", this );
  buttons->addWidget(cancel);

  QObject::connect( ok, SIGNAL(clicked()), SLOT(inputGiven()) );
  QObject::connect( cancel, SIGNAL(clicked()), SLOT(reject()) );    

  // ok-cancel buttons:
  topLayout->addLayout(chains, 0);
  topLayout->addLayout(buttons, 0);

  init_shortcuts();
}

void QEChainselectDialog::init_shortcuts(void) {
  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(CTRL+Key_A), chaininput,
  		 SLOT(setFocus()));

  a->connectItem(a->insertItem(CTRL+Key_C), this,
  		 SLOT(reject()));

  a->connectItem(a->insertItem(CTRL+Key_O), this,
  		 SLOT(inputGiven()));
}

void QEChainselectDialog::init_chains(void) {
  QVGroupBox* chainbox = new QVGroupBox("(A)ttach file to chains", this, 0);
  chaininput = new QListBox(chainbox, "chains");
  chaininput->setSelectionMode(QListBox::Multi);

  vector<CHAIN*>::const_iterator p = chainsetup->chains.begin();
  while(p != chainsetup->chains.end()) {
    chaininput->insertItem((*p)->name().c_str());
    ++p;
  }
  chains = new QHBoxLayout();
  chains->addWidget(chainbox);
}

void QEChainselectDialog::inputGiven(void) {
  for(int n = 0; n < chaininput->count(); n++) {
    ecadebug->msg(4, "(eca-qtiodevdialog) checking chaininput " + kvu_numtostr(n));
    if (chaininput->isSelected(chaininput->item(n))) {
      r_chains.push_back(chainsetup->chains[n]->name());
      ecadebug->msg(4, "(eca-qtiodevdialog) adding chain " + r_chains.back());
    }
  }
  
  accept();
}

void QEChainselectDialog::keyPressEvent(QKeyEvent* kevent) {
  handle_key(kevent->ascii());
  kevent->ignore();
}

void QEChainselectDialog::handle_key(int c) {
  switch(tolower(c)) {
  case 'a': 
    {
      chaininput->setFocus();
      break;
    }
  case 'c': 
    {
      reject();
      break;
    }
  case 'o': 
    {
      inputGiven();
      break;
    }

  default: { }
  }
}
