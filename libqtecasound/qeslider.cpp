// ------------------------------------------------------------------------
// qeslider.cpp: Graphical slider for controlling effect parameters
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

#include <string>
#include <vector>

#include <qwidget.h>
#include <qlayout.h>

#include "qeslider.h"

QESlider::QESlider(QWidget *parent, const char *name) 
  : QEController(parent, name) {

  init_layout();
}

CONTROLLER_SOURCE::parameter_type QESlider::value(void) {
  return(value_rep);
}

void QESlider::init_layout(void) {
  QBoxLayout* layout = new QVBoxLayout(this);

  slider_rep = new QSlider(0, // min
			   100, // max
			   1, // step
			   0, // initial value
			   QSlider::Horizontal, 
			   this,
			   0);
  QObject::connect(slider_rep, SIGNAL(valueChanged(int)), this, SLOT(update_value(int)));

  layout->addWidget(slider_rep);
}

void QESlider::update_value(int v) {
  double d (v);
  d /= 100;
  value_rep = d;
}
