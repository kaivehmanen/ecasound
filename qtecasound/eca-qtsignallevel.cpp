// ------------------------------------------------------------------------
// eca-qtsignallevel.cpp: Qt-base class for signallevel monitors.
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

#include <string>
#include <vector>

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kvutils.h>

#include "samplebuffer.h"
#include "eca-chain.h"

#include "eca-qtlevelmeter.h"
#include "eca-qtsignallevel.h"

QESignalLevel::QESignalLevel(vector<SAMPLE_BUFFER>* c, QWidget *parent, const char *name) 
        : QWidget( parent, name )
{
  string caption = "qtEcasound - output level monitoring";
  setCaption(caption.c_str());

  setMaximumSize(300, 80 * c->size());
  
  inputs = c;
  QBoxLayout* topLayout = new QVBoxLayout( this, 5 );

  for (int q = 0; q < static_cast<int>(inputs->size()); q++) {
    topLayout->addWidget(new QLabel("Left", this, "left"), 5);
    levelmeters.push_back(new QELevelMeter((double)SAMPLE_SPECS::max_amplitude, this,"level_left"));
    topLayout->addWidget(levelmeters.back(), 2);

    topLayout->addWidget(new QLabel("Right", this, "right"),5);
    levelmeters.push_back(new QELevelMeter((double)SAMPLE_SPECS::max_amplitude, this,"level_right"));
    topLayout->addWidget(levelmeters.back(), 2);
  }
}


void QESignalLevel::update(int p) {
  //  double temp = (*inputs)[p].average_volume();
  //  static double temp;
  //  temp = fabs((*inputs)[p].get(SAMPLE_BUFFER::ch_left));
  //  temp += fabs((*inputs)[p].get(SAMPLE_BUFFER::ch_right));
  //  temp /= 2;

  //  levelmeters[p]->set_value(fabs((*inputs)[p].get(SAMPLE_BUFFER::ch_left)));
  //  levelmeters[p+1]->set_value(fabs((*inputs)[p].get(SAMPLE_BUFFER::ch_right)));

  //  cerr << "A";
  //  levelmeters[p]->set_value(old_values[p]);
  //  levelmeters[p+1]->set_value(old_values[p + 1]);

  //  cerr << "B";
  //  old_values[p] = new_values[p];
  //  old_values[p + 1]  = new_values[p + 1];

  //  cerr << "C";
  //  new_values[p] = fabs((*inputs)[p].get(SAMPLE_BUFFER::ch_left));
  //  new_values[p + 1] = fabs((*inputs)[p].get(SAMPLE_BUFFER::ch_right));

  //  levelmeters[p]->set_value(temp);
  //  levelmeters[p + 1]->set_value(temp);

  levelmeters[p]->set_value((*inputs)[p].average_RMS_volume(SAMPLE_SPECS::ch_left,64));
  levelmeters[p+1]->set_value((*inputs)[p].average_RMS_volume(SAMPLE_SPECS::ch_right,64));
}

void QESignalLevel::mute(void) {
  for (int p = 0; p < static_cast<int>(levelmeters.size()); p++) {
    levelmeters[p]->set_value(0);
  }
}







