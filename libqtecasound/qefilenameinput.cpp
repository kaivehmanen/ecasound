// ------------------------------------------------------------------------
// qefilenameinput.cpp: Filename input with browsing
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
#include <qlabel.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include "qefilenameinput.h"

QEFilenameInput::QEFilenameInput (int opmode, 
				  QWidget *parent,
				  const char *name) 
  : QEInput(parent, name),
    mode_rep(opmode) {
 
  filename = 0;
  accel = new QAccel(this);
  init_layout();
}

QSize QEFilenameInput::sizeHint(void) const { 
  if (filename != 0) return(filename->sizeHint());
  return(QSize(300,100));
}

void QEFilenameInput::init_layout(void) {
  filename = new QHBoxLayout(this);

  filename->add(new QLabel("(F)ile name: ", this, 0));
  filenameinput = new QLineEdit(this, "filenameinput");
  filenameinput->setMinimumSize(300,0);
  filename->addWidget(filenameinput);

  QPushButton* filenamebrowse = new QPushButton("(B)rowse", this, 0);
  filename->addWidget(filenamebrowse);

  QObject::connect(filenamebrowse, SIGNAL(clicked()), this, SLOT(button_browse()));

  accel->connectItem(accel->insertItem(CTRL+Key_B), this, SLOT(button_browse()));
  accel->connectItem(accel->insertItem(CTRL+Key_F), filenameinput, SLOT(setFocus()));
}

void QEFilenameInput::button_browse(void) { 
  QFileDialog* fdialog = new QFileDialog(0, 0, true);
  if (mode_rep == QEFilenameInput::browse_existing) {
    fdialog->setMode(QFileDialog::ExistingFile);    
    fdialog->setCaption("Opening file...");
  }
  else {
    fdialog->setMode(QFileDialog::AnyFile);
    fdialog->setCaption("Saving as...");
  }

  fdialog->setDir(current_dir_rep.c_str());
  fdialog->exec();

  QString fname = fdialog->selectedFile();
  if (!fname.isEmpty()) {
    current_dir_rep = string(fdialog->dirPath().latin1());
    filenameinput->setText(fname);
  }

  emit file_selected();
}

const string& QEFilenameInput::result_filename(void) const { 
  filename_rep = string(filenameinput->text().latin1());
  return(filename_rep); 
}
