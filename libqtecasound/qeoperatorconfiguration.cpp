// ------------------------------------------------------------------------
// qeoperatorconfiguration.cpp: Input widget for configuring ecasound 
//                              operators
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

#include "qeoperatorconfiguration.h"

QEOperatorConfiguration::QEOperatorConfiguration (OPERATOR* op, QWidget *parent, const char *name) 
  : QEInput(parent, name),
    operator_rep(op) {
  init_layout();
}

void QEOperatorConfiguration::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);

  QGroupBox* descgroup = new QHGroupBox(this, "descgroup");
  obj_name = new QLabel("Name: ", descgroup, "name1");
  obj_name = new QLabel(operator_rep->name().c_str(), descgroup, "name2");
  top->addWidget(descgroup);

  QGroupBox* descgroup2 = new QHGroupBox(this, "descgroup2");
  obj_desc = new QLabel("Description: ", descgroup2, "desc1");
  obj_desc = new QLabel(operator_rep->description().c_str(), descgroup2, "desc2");
  top->addWidget(descgroup2);

  operator_rep->map_parameters();
  paramlist.resize(operator_rep->number_of_params());
  inputlist.resize(operator_rep->number_of_params());
  paramgrid = new QGrid(2, this);
  for(int n = 0; n < operator_rep->number_of_params(); n++) {
    paramlist[n] = new QLabel(QString::number(n + 1).leftJustify(12),paramgrid);
    inputlist[n] = new QLineEdit(paramgrid);
  }
  top->addWidget(paramgrid, 1);

  for(int n = 0; n < operator_rep->number_of_params(); n++) {
    paramlist[n]->setText(QString(QString::number(n + 1) + 
				  QString(". ") + 
				  QString(operator_rep->get_parameter_name(n + 1).c_str())).leftJustify(12));
  }
  
  for(int n = operator_rep->number_of_params(); n < static_cast<int>(paramlist.size()); n++) {
    paramlist[n]->setText(QString(" - ").leftJustify(12));
  }
}

void QEOperatorConfiguration::update_results(void) {
  for(int n = 0; n < operator_rep->number_of_params(); n++) {
    operator_rep->set_parameter(n + 1, inputlist[n]->text().toFloat());
  }
}

void QEOperatorConfiguration::change_operator(OPERATOR* op) {
  assert(op != 0);
  operator_rep = op;
  obj_name->setText(operator_rep->name().c_str());
  obj_desc->setText(operator_rep->description().c_str());

  operator_rep->map_parameters();
  int params_height = 0;
  for(int n = 0; n < operator_rep->number_of_params(); n++) {
    if (n == static_cast<int>(paramlist.size())) {
      paramlist.push_back(new QLabel("",paramgrid));
      inputlist.push_back(new QLineEdit(paramgrid));
    }
    paramlist[n]->setText(QString(QString::number(n + 1) + 
				  QString(". ") + 
				  QString(operator_rep->get_parameter_name(n + 1).c_str())).leftJustify(12));
    inputlist[n]->setText(QString::number(operator_rep->get_parameter(n + 1)));
    params_height += paramlist.back()->height();

    inputlist[n]->show();
    paramlist[n]->show();
  }

  for(int n = operator_rep->number_of_params(); n <
	static_cast<int>(paramlist.size()); n++) {
    inputlist[n]->hide();
    paramlist[n]->hide();
  }
  paramgrid->resize(paramgrid->width(), params_height);
}
