// ------------------------------------------------------------------------
// QEAudioFormatInput.cpp: Dialog for selecting audio format
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

#include "qeaudioformatinput.h"

QEAudioFormatInput::QEAudioFormatInput (QWidget *parent, const char *name) 
  : QWidget(parent, name) {

  init_layout();
}

void QEAudioFormatInput::init_layout(void) {
  QVGroupBox* b_bits = new QVGroupBox("B(i)ts", this, 0);
  bits_input = new QSpinBox(8, 32, 8, b_bits, "bits");
  bits_input->setValue(16);

  QVGroupBox* b_channels = new QVGroupBox("Ch(a)nnels", this, 0);
  channel_input = new QSpinBox(0, 1024, 1, b_channels, "channels");
  channel_input->setValue(2);

  QVGroupBox* sratebox = new QVGroupBox("Sample (r)ate", this, 0);
  srate_input = new QSpinBox(4000, 48000, 10, sratebox, "srate");
  srate_input->setValue(44100);

  QBoxLayout* format = new QHBoxLayout(this);
  format->addWidget(b_bits);
  format->addWidget(b_channels);
  format->addWidget(sratebox);

  QAccel *a = new QAccel(this);
  a->connectItem(a->insertItem(ALT+Key_A), channel_input, SLOT(setFocus()));
  a->connectItem(a->insertItem(ALT+Key_I), bits_input, SLOT(setFocus()));
  a->connectItem(a->insertItem(ALT+Key_R), srate_input, SLOT(setFocus()));
}

void QEAudioFormatInput::enable_format(void) {
  channel_input->setEnabled(true);
  srate_input->setEnabled(true);
  bits_input->setEnabled(true);
}

void QEAudioFormatInput::disable_format(void) {
  channel_input->setEnabled(false);
  srate_input->setEnabled(false);
  bits_input->setEnabled(false);
}

int QEAudioFormatInput::result_bits(void) const { return(bits_input->value()); }
int QEAudioFormatInput::result_channels(void) const { return(channel_input->value()); }
int QEAudioFormatInput::result_srate(void) const { return(srate_input->value()); }

void QEAudioFormatInput::set_bits(int value) { bits_input->setValue(value); }
void QEAudioFormatInput::set_channels(int value) { channel_input->setValue(value); }
void QEAudioFormatInput::set_srate(int value) { srate_input->setValue(value); }
