// ------------------------------------------------------------------------
// qeoperatorconfiguration.cpp: Input widget for configuring ecasound 
//                              audio objects
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

#include <vector>
#include <string>

#include <qaccel.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qhbox.h>
#include <qlineedit.h>
#include <qgrid.h>

#include "qeaudioobjectconfiguration.h"

QEAudioObjectConfiguration::QEAudioObjectConfiguration (AUDIO_IO* op, QWidget *parent, const char *name) 
  : QEInput(parent, name),
    object_rep(op) {
  init_layout();
}

void QEAudioObjectConfiguration::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);

  QGroupBox* descgroup = new QHGroupBox(this, "descgroup");
  obj_name = new QLabel("Name: ", descgroup, "name1");
  obj_name = new QLabel(object_rep->name().c_str(), descgroup, "name2");
  top->addWidget(descgroup);

  QGroupBox* descgroup2 = new QHGroupBox(this, "descgroup2");
  obj_desc = new QLabel("Description: ", descgroup2, "desc1");
  obj_desc = new QLabel(object_rep->description().c_str(), descgroup2, "desc2");
  top->addWidget(descgroup2);

  object_rep->map_parameters();
  paramlist.resize(object_rep->number_of_params());
  inputlist.resize(object_rep->number_of_params());
  paramgrid = new QGrid(2, this);
  for(int n = 0; n < object_rep->number_of_params(); n++) {
    paramlist[n] = new QLabel(QString::number(n + 1).leftJustify(12),paramgrid);
    inputlist[n] = new QLineEdit(paramgrid);
  }
  top->addWidget(paramgrid, 1);

  for(int n = 0; n < object_rep->number_of_params(); n++) {
    paramlist[n]->setText(QString(QString::number(n + 1) + 
				  QString(". ") + 
				  QString(object_rep->get_parameter_name(n + 1).c_str())).leftJustify(12));
  }
  
  for(int n = object_rep->number_of_params(); n < static_cast<int>(paramlist.size()); n++) {
    paramlist[n]->setText(QString(" - ").leftJustify(12));
  }
}

void QEAudioObjectConfiguration::update_results(void) {
  for(int n = 0; n < object_rep->number_of_params(); n++) {
    object_rep->set_parameter(n + 1, inputlist[n]->text().latin1());
  }
}

void QEAudioObjectConfiguration::change_object(AUDIO_IO* op) {
  assert(op != 0);
  object_rep = op;
  obj_name->setText(object_rep->name().c_str());
  obj_desc->setText(object_rep->description().c_str());

  object_rep->map_parameters();
  int params_height = 0;
  for(int n = 0; n < object_rep->number_of_params(); n++) {
    if (n == static_cast<int>(paramlist.size())) {
      paramlist.push_back(new QLabel("",paramgrid));
      inputlist.push_back(new QLineEdit(paramgrid));
    }
    paramlist[n]->setText(QString(QString::number(n + 1) + 
				  QString(". ") + 
				  QString(object_rep->get_parameter_name(n + 1).c_str())).leftJustify(12));
    inputlist[n]->setText(object_rep->get_parameter(n + 1).c_str());
    params_height += paramlist.back()->height();

    inputlist[n]->show();
    paramlist[n]->show();
  }

  for(int n = object_rep->number_of_params(); n <
	static_cast<int>(paramlist.size()); n++) {
    inputlist[n]->hide();
    paramlist[n]->hide();
  }
  paramgrid->resize(paramgrid->width(), params_height);
}
