// ------------------------------------------------------------------------
// qechainopinput.cpp: Chain operator input widget
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

#include "eca-static-object-maps.h"

#include "qeobjectinput.h"
#include "qechainopinput.h"
#include "qeoperatorinput.h"

QEChainopInput::QEChainopInput (QWidget *parent, const char *name) 
  : QEInput(parent, name) {

  QBoxLayout* top = new QVBoxLayout(this);
  QEObjectInput* obinput = new QEObjectInput(&eca_chain_operator_map, this);
  //  QEOperatorInput* opinput = new QEObjectInput(0, this);
  top->addWidget(obinput);
  //  top->addWidget(opinput);  

  init_layout();
}

void QEChainopInput::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);

  QGroupBox* copgroup = new QVGroupBox(this, "copgroup");
  QListBox* coplist = new QListBox(copgroup, "coplist");

  int max_params = 0;

  register_default_objects();
  const map<string,string>& omap = eca_chain_operator_map.registered_objects();

  map<string,string>::const_iterator p = omap.begin();
  while(p != omap.end()) {
    coplist->insertItem(p->first.c_str());
    int params = dynamic_cast<CHAIN_OPERATOR*>(eca_chain_operator_map.object(p->second))->number_of_params();
    if (params > max_params) max_params = params;
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
    paramlist[n] = new QLabel(QString::number(n + 1).leftJustify(12),igrid);
    inputlist[n] = new QLineEdit(igrid);
  }
  top->addWidget(paramgroup);
  QObject::connect(coplist, SIGNAL(highlighted(int)), this, SLOT(update_chainop(int)));
}

void QEChainopInput::update_results(void) {
  if (chainop_rep == 0) return;
  
  for(int n = 0; n < chainop_rep->number_of_params(); n++) {
    chainop_rep->set_parameter(n + 1, inputlist[n]->text().toDouble());
  }
}

void QEChainopInput::update_chainop(int index) {
  chainop_rep = 0;

  const map<string,string>& omap = eca_chain_operator_map.registered_objects();
  int counter = 0;
  map<string,string>::const_iterator p = omap.begin();
  while(p != omap.end()) {
    if (counter == index) {
      chainop_rep = dynamic_cast<CHAIN_OPERATOR*>(eca_chain_operator_map.object(p->second));
      selected_index = counter;
    }   
    ++p;
    ++counter;
  }

  if (chainop_rep != 0) {
    cop_desc->setText(chainop_rep->name().c_str());

    for(int n = 0; n < chainop_rep->number_of_params(); n++) {
      if (n >= static_cast<int>(paramlist.size())) break;
      paramlist[n]->setText(QString(QString::number(n + 1) + 
				    QString(". ") + 
				    QString(chainop_rep->get_parameter_name(n + 1).c_str())).leftJustify(12));
    }

    for(int n = chainop_rep->number_of_params(); n < static_cast<int>(paramlist.size()); n++) {
      paramlist[n]->setText(QString(" - ").leftJustify(12));
    }
  }
}
