// ------------------------------------------------------------------------
// qstringdialog.cpp: Qt dialog widget that asks user for a string.
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

#include <ctype.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qaccel.h>

#include "qestringdialog.h"

QEStringDialog::QEStringDialog (const QString& prompt, QWidget *parent, const char *name) 
  : QDialog(parent, name, true) {

  setCaption("Input - " + prompt);

  QBoxLayout* topLayout = new QVBoxLayout( this );
  QBoxLayout* textinput = new QHBoxLayout();

  QLabel* tekstiinfo = new QLabel(prompt, this, "tekstiinfo");
  textinput->add( tekstiinfo);
  
  tekstirivi = new QLineEdit(this, "tekstirivi");
  tekstirivi->setMinimumSize(300,0);
 textinput->addWidget( tekstirivi);
  
  QObject::connect(tekstirivi, SIGNAL(returnPressed ()), this, SLOT(inputGiven()) );

  QBoxLayout* buttons = new QHBoxLayout();

  QPushButton *ok, *cancel;
  ok = new QPushButton( "(O)K", this );
  buttons->addWidget(ok);

  cancel = new QPushButton( "(C)ancel", this );
  buttons->addWidget(cancel);

  QObject::connect( ok, SIGNAL(clicked()), SLOT(inputGiven()) );
  QObject::connect( cancel, SIGNAL(clicked()), SLOT(reject()) );    

  topLayout->addLayout(textinput, 0);
  topLayout->addLayout(buttons, 0);

  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(CTRL+Key_C), this,
		 SLOT(reject()));
  a->connectItem(a->insertItem(CTRL+Key_O), this,
		 SLOT(inputGiven()));
}

void QEStringDialog::inputGiven(void) {
  input_text = tekstirivi->text();
  accept();
}

void QEStringDialog::keyPressEvent(QKeyEvent* kevent) {
  switch(tolower(kevent->ascii())) {
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
  }
  kevent->ignore();
}
