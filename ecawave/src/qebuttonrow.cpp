// ------------------------------------------------------------------------
// qebuttonrow.cpp: User-interface widget for representing a button-row
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

#include <qbutton.h>
#include <qobject.h>

#include "qebuttonrow.h"

void QEButtonRow::set_default_font(const QFont& v) { default_font = v; }
QFont QEButtonRow::default_font = QFont::defaultFont();

QEButtonRow::QEButtonRow (QWidget *parent = 0, const char *name = 0) 
  : QWidget(parent,name)
{
  font_rep = QEButtonRow::default_font;
  box = new QHBoxLayout(this);
  accel = new QAccel(this);
}

void QEButtonRow::set_font(const QFont& v) { 
  font_rep = v;
  vector<QButton*>::const_iterator p = buttons.begin();
  while(p != buttons.end()) {
    (*p)->setFont(font_rep);
    ++p;
  }
}

void QEButtonRow::add_button(QButton* v, int key) {
  v->setFont(font_rep);
  accel->connectItem(accel->insertItem(key), v, SLOT(animateClick()));
  box->addWidget(v);
  buttons.push_back(v);
}

void QEButtonRow::add_button(QButton* v, int key, const QObject *
			    receiver, const char * member) {
  v->setFont(font_rep);
  accel->connectItem(accel->insertItem(key), v, SLOT(animateClick()));
  QObject::connect(v, SIGNAL(clicked()), receiver, member);
  box->addWidget(v);
  buttons.push_back(v);
}
