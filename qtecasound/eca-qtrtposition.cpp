// ------------------------------------------------------------------------
// eca-qtrtposition.cpp: A horizontal slider with two-way data
// exchange.
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

#include <cmath>
#include <iostream>

#include <qwidget.h>
#include <qslider.h>
#include <qwindowsstyle.h>

#include "eca-qtrtposition.h"

QERuntimePosition::QERuntimePosition (double length, QWidget *parent, const char
				      *name) :
QSlider(QSlider::Horizontal, parent, name)
{
  length_in_seconds(length);
  widget_control = false;
  //  setBackgroundColor("white");
  setTickmarks(QSlider::Both);
  setRange(0, 1000);
  setTickInterval(500);
  setStyle(new QWindowsStyle());
  connect(this, SIGNAL(sliderReleased()), this, SLOT(change_position_from_widget()));
  connect(this, SIGNAL(sliderPressed()), this, SLOT(mouse_active()));
}

void QERuntimePosition::length_in_seconds(double seconds) {
  totallen = seconds;
  //  setRange(1,(int)ceil(totallen));
}

void QERuntimePosition::position_in_seconds(double seconds) {
  if (widget_control) {
    //    cerr << "Widget still has control, can't change position...\n";
    return;
  }
  if (seconds < totallen) 
    position = seconds;

  last_normally_changed = (int)floor(position * 1000.0 / totallen);
  setValue(last_normally_changed);

  //  repaint();
}

bool QERuntimePosition::does_widget_have_control(void) {
  return(widget_control);
}

void QERuntimePosition::control_back_to_parent(void) {
  //  cerr << "Control back to parent.\n";
  widget_control = false;
}

void QERuntimePosition::change_position_from_widget(void) {
  if (value() == last_normally_changed) {
    widget_control = false;
    return;
  }
  //  cerr << "User touching my goddam slider! :)\n";
  emit position_changed_from_widget(value() / 1000.0 * totallen);
}

void QERuntimePosition::mouse_active(void) { widget_control = true;  }







