// ------------------------------------------------------------------------
// qeaudioformatinput.cpp: Dialog for selecting audio format
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
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kvutils/kvu_numtostr.h>

#include "qeaudioformatinput.h"

QEAudioFormatInput::QEAudioFormatInput (QWidget *parent, const char *name) 
  : QEInput(parent, name) {

  init_layout();
}

void QEAudioFormatInput::init_layout(void) {
  QVGroupBox* b_bits = new QVGroupBox("B(i)ts", this, 0);
  bits_input = new QSpinBox(8, 64, 8, b_bits, "bits");
  bits_input->setValue(bits());

  QVGroupBox* b_channels = new QVGroupBox("Ch(a)nnels", this, 0);
  channel_input = new QSpinBox(0, 1024, 1, b_channels, "channels");
  channel_input->setValue(channels());

  QVGroupBox* sratebox = new QVGroupBox("Sample (r)ate", this, 0);
  srate_input = new QSpinBox(4000, 192000, 10, sratebox, "srate");
  srate_input->setValue(samples_per_second());

  QBoxLayout* format = new QHBoxLayout(this);
  format->addWidget(b_bits);
  format->addWidget(b_channels);
  format->addWidget(sratebox);

  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(CTRL+Key_A), channel_input, SLOT(setFocus()));
  a->connectItem(a->insertItem(CTRL+Key_I), bits_input, SLOT(setFocus()));
  a->connectItem(a->insertItem(CTRL+Key_R), srate_input, SLOT(setFocus()));
}

void QEAudioFormatInput::enable(void) {
  channel_input->setEnabled(true);
  srate_input->setEnabled(true);
  bits_input->setEnabled(true);
}

void QEAudioFormatInput::disable(void) {
  channel_input->setEnabled(false);
  srate_input->setEnabled(false);
  bits_input->setEnabled(false);
}

void QEAudioFormatInput::update_results(void) {
  set_sample_format(kvu_numtostr(bits_input->value()));
  set_channels(channel_input->value());
  set_samples_per_second(srate_input->value());
}
