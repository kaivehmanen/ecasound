// ------------------------------------------------------------------------
// qechainopinput.cpp: Input widget selecting an libecasound chain operator
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
#include <qlistbox.h>
#include <qlabel.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qhbox.h>
#include <qlineedit.h>
#include <qgrid.h>

#include <string>
#include <map>

#include "qechainopinput.h"

QEChainopInput::QEChainopInput (QWidget *parent, const char *name) 
  : QWidget(parent, name, false) {

  init_layout();
}

void QEChainopInput::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);

  QGroupBox* copgroup = new QVGroupBox(this, "copgroup");
  QListBox* coplist = new QListBox(copgroup, "coplist");
  QObject::connect(coplist, SIGNAL(highlighted(int)), this, SLOT(update_chainop(int)));

  int max_params = 0;
  ECA_CHAIN_OPERATOR_MAP::register_default_objects();
  map<string, DYNAMIC_OBJECT*>::const_iterator p = ECA_CHAIN_OPERATOR_MAP::object_map.begin();
  while(p != ECA_CHAIN_OPERATOR_MAP::object_map.end()) {
    coplist->insertItem(p->second->name().c_str());
    if (p->second->number_of_params() > max_params) max_params = p->second->number_of_params();
    ++p;
  }
  coplist->setSelected(0, true);
  top->addWidget(copgroup);

  top->addSpacing(10);

  QBoxLayout* d = new QHBoxLayout();
  QGroupBox* descgroup = new QHGroupBox(this, "descgroup");
  QLabel* desc = new QLabel("Description: ", descgroup, "desc1");
  cop_desc = new QLabel(" - ", descgroup, "desc2");
  d->addWidget(descgroup);
  top->addLayout(d);

  top->addSpacing(10);
  
  QGroupBox* paramgroup = new QVGroupBox(this, "paramgroup");
  paramlist.resize(max_params);
  inputlist.resize(max_params);
  QGrid* igrid = new QGrid(2, paramgroup);
  for(int n = 0; n < max_params; n++) {
    //    QHBox* box = new QHBox(paramgroup);
    paramlist[n] = new QLabel(QString::number(n + 1).leftJustify(12),igrid);
    inputlist[n] = new QLineEdit(igrid);
  }
  top->addWidget(paramgroup);
}

CHAIN_OPERATOR* QEChainopInput::clone_result(void) {
  return(chainop_rep->clone());
}

void QEChainopInput::set_parameters(void) {
  if (chainop_rep == 0) return;
  
  for(int n = 0; n < chainop_rep->number_of_params(); n++) {
    chainop_rep->set_parameter(n + 1, inputlist[n]->text().toDouble());
  }
}

void QEChainopInput::update_chainop(int index) {
  DYNAMIC_OBJECT* dobj = 0;

  map<string, DYNAMIC_OBJECT*>::const_iterator p = ECA_CHAIN_OPERATOR_MAP::object_map.begin();
  for(int n = 0; n < ECA_CHAIN_OPERATOR_MAP::object_map.size(); n++) {
    assert(p != ECA_CHAIN_OPERATOR_MAP::object_map.end());
    if (n == index) {
      dobj = p->second;
      chainop_rep = reinterpret_cast<CHAIN_OPERATOR*>(dobj);
      selected_index = n;
    }   
    ++p;
  }
  if (dobj == 0) return;

  cop_desc->setText(dobj->name().c_str());

  for(int n = 0; n < dobj->number_of_params(); n++) {
    if (n >= paramlist.size()) break;
    paramlist[n]->setText(QString(QString::number(n + 1) + 
			  QString(". ") + 
			  QString(dobj->get_parameter_name(n + 1).c_str())).leftJustify(12));
  }

  for(int n = dobj->number_of_params(); n < paramlist.size(); n++) {
    paramlist[n]->setText(QString(" - ").leftJustify(12));
  }
}
