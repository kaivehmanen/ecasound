// ------------------------------------------------------------------------
// qeokcanceltinput.cpp: 'ok' - 'cancel' input buttons
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
#include <qpushbutton.h>

#include "qeokcancelinput.h"

QEOkCancelInput::QEOkCancelInput (QWidget *parent, const char *name) 
  : QEInput(parent, name) {

  buttons = new QHBoxLayout(this);

  QPushButton* ok_button_rep = new QPushButton( "(O)K", this);
  buttons->addWidget(ok_button_rep);
  
  QPushButton* cancel_button_rep = new QPushButton( "(C)ancel", this);
  buttons->addWidget(cancel_button_rep);

  QAccel *a = new QAccel(this);
  QObject::connect(ok_button_rep, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(cancel_button_rep, SIGNAL(clicked()), this, SLOT(reject()));

  a->connectItem(a->insertItem(ALT+Key_C), this, SLOT(reject()));
  a->connectItem(a->insertItem(ALT+Key_O), this, SLOT(accept()));
}

QSize QEOkCancelInput::sizeHint(void) const { 
  if (buttons != 0) 
    return(buttons->sizeHint());
  return(QSize(300,100));
}


void QEOkCancelInput::accept(void) { emit ok(); }
void QEOkCancelInput::reject(void) { emit cancel(); }
