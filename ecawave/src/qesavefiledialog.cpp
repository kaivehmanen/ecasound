// ------------------------------------------------------------------------
// qesavefiledialog.cpp: Dialog for selecting audio file for saving
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

#include <ecasound/eca-audio-objects.h>

#include "qesavefiledialog.h"

QESaveFileDialog::QESaveFileDialog (QWidget *parent, const char *name) 
  : QDialog(parent, name, true) {

  QBoxLayout* top = new QVBoxLayout(this);

  fname = new QEFilenameInput(QEFilenameInput::browse_any, this);
  top->addWidget(fname);

  top->addSpacing(5);

  okcancel = new QEAcceptInput(this);
  QObject::connect(okcancel, SIGNAL(ok()), this, SLOT(accept()));
  QObject::connect(okcancel, SIGNAL(cancel()), this, SLOT(reject()));
  top->addWidget(okcancel);
}
