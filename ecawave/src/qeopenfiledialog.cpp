// ------------------------------------------------------------------------
// qeopenfiledialog.cpp: Dialog for selecting audio file, format and 
//                       various related options.
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
#include <qcheckbox.h>

#include <ecasound/audioio.h>
#include <ecasound/eca-audio-objects.h>
#include <ecasound/eca-static-object-maps.h>

#include "qeopenfiledialog.h"

QEOpenFileDialog::QEOpenFileDialog (QWidget *parent, const char *name) 
  : QDialog(parent, name, true) {

  QBoxLayout* top = new QVBoxLayout(this);

  fname = new QEFilenameInput(QEFilenameInput::browse_existing, this);
  QObject::connect(fname, SIGNAL(file_selected()), this, SLOT(format_test()));
  top->addWidget(fname);

  top->addSpacing(5);

  aformat = new QEAudioFormatInput(this);
  top->addWidget(aformat);

  top->addSpacing(5);

  top->addLayout(init_toggles());

  top->addSpacing(5);

  okcancel = new QEAcceptInput(this);
  QObject::connect(okcancel, SIGNAL(ok()), this, SLOT(accept()));
  QObject::connect(okcancel, SIGNAL(cancel()), this, SLOT(reject()));
  top->addWidget(okcancel);
}

QBoxLayout* QEOpenFileDialog::init_toggles(void) {
  QBoxLayout* toggles = new QVBoxLayout();

  wcache_toggle = new QCheckBox("Use (w)aveform cache: ", this, 0);
  refresh_toggle = new QCheckBox("Force cache (r)efresh: ", this, 0);
  direct_toggle = new QCheckBox("D(i)rect mode: ", this, 0);

  wcache_toggle->toggle();

  toggles->addWidget(wcache_toggle);
  toggles->addWidget(refresh_toggle);
  toggles->addWidget(direct_toggle);

  QObject::connect(wcache_toggle, SIGNAL(toggled(bool)), this, SLOT(update_wcache_toggle(bool)));
  QObject::connect(refresh_toggle, SIGNAL(toggled(bool)), this, SLOT(update_refresh_toggle(bool)));

  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(ALT+Key_W), wcache_toggle, SLOT(toggle()));
  a->connectItem(a->insertItem(ALT+Key_R), refresh_toggle, SLOT(toggle()));
  a->connectItem(a->insertItem(ALT+Key_I), direct_toggle, SLOT(toggle()));
  return(toggles);
}

void QEOpenFileDialog::update_wcache_toggle(bool v) {
  if (v == false) refresh_toggle->setChecked(false);
}

void QEOpenFileDialog::update_refresh_toggle(bool v) {
  if (v == true) wcache_toggle->setChecked(true);
}

void QEOpenFileDialog::format_test(void) {
  AUDIO_IO* p = dynamic_cast<AUDIO_IO*>(eca_audio_object_map.object(result_filename()));
  if (p != 0) {
    if (p->locked_audio_format() == true) {
      aformat->disable_format();
    }
    else {
      aformat->enable_format();
    }
  
    if ((p->supported_io_modes() & AUDIO_IO::io_readwrite) ==
	AUDIO_IO::io_readwrite) {
      direct_toggle->setEnabled(true);
    }
    else {
      direct_toggle->setEnabled(false);
    }
  }
}
