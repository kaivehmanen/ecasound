// ------------------------------------------------------------------------
// qeoperatorinput.cpp: Input widget for dynamic objects with numeric 
//                      parameters
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
#include <qvgroupbox.h>
#include <qhbox.h>
#include <qlineedit.h>
#include <qgrid.h>

#include "qeoperatorinput.h"

QEOperatorInput::QEOperatorInput (OPERATOR* op, QWidget *parent, const char *name) 
  : QEInput(parent, name),
    operator_rep(op) {

  init_layout();
}

void QEOperatorInput::init_layout(void) {
  QGroupBox* paramgroup = new QVGroupBox(this, "paramgroup");
  paramlist.resize(operator_rep->number_of_params());
  inputlist.resize(operator_rep->number_of_params());
  QGrid* igrid = new QGrid(2, paramgroup);
  for(int n = 0; n < operator_rep->number_of_params(); n++) {
    paramlist[n] = new QLabel(QString::number(n + 1).leftJustify(12),igrid);
    inputlist[n] = new QLineEdit(igrid);
  }
  top->addWidget(paramgroup);

  if (operator_rep != 0) {
    cop_desc->setText(operator_rep->name().c_str());

    for(int n = 0; n < operator_rep->number_of_params(); n++) {
      if (n >= static_cast<int>(paramlist.size())) break;
      paramlist[n]->setText(QString(QString::number(n + 1) + 
				    QString(". ") + 
				    QString(operator_rep->get_parameter_name(n + 1).c_str())).leftJustify(12));
    }

    for(int n = operator_rep->number_of_params(); n < static_cast<int>(paramlist.size()); n++) {
      paramlist[n]->setText(QString(" - ").leftJustify(12));
    }
  }

  updateGeometry();
}

void QEOperatorInput::update_results(void) {
  if (operator_rep == 0) return;
  
  for(int n = 0; n < operator_rep->number_of_params(); n++) {
    operator_rep->set_parameter(n + 1, inputlist[n]->text().toDouble());
  }
}
