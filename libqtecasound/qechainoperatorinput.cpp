// ------------------------------------------------------------------------
// qechainoperatorinput.cpp: Chain operator input widget
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

#include <qlayout.h>
#include <qlabel.h>
#include <qtabwidget.h>

#include <string>
#include <map>

#include "eca-static-object-maps.h"

#include "qeobjectmap.h"
#include "qechainoperatorinput.h"
// #include "qeoperatorconfiguration.h"

QEChainOperatorInput::QEChainOperatorInput (QWidget *parent, const char *name) 
  : QEInput(parent, name) {

  init_layout();
}

void QEChainOperatorInput::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);
  maptab_rep = new QTabWidget(this, "maptab");
  omap_inputs.push_back(new QEObjectMap(&eca_chain_operator_map, this));
  maptab_rep->addTab(omap_inputs.back(), "&Chain operators");

  omap_inputs.push_back(new QEObjectMap(&eca_preset_map, this));
  maptab_rep->addTab(omap_inputs.back(), "&Effect presets");

  omap_inputs.push_back(new QEObjectMap(&eca_ladspa_plugin_map, this));
  maptab_rep->addTab(omap_inputs.back(), "&Ladspa plugins");

  top->addWidget(maptab_rep);

//    QBoxLayout* d = new QHBoxLayout();
//    QGroupBox* descgroup = new QHGroupBox(this, "descgroup");
//    QLabel* desc = new QLabel("Description: ", descgroup, "desc1");
//    cop_desc = new QLabel(" - ", descgroup, "desc2");
//    d->addWidget(descgroup);
//    top->addLayout(d);
//    top->addSpacing(10);
//    QGroupBox* paramgroup = new QVGroupBox(this, "paramgroup");
//    QGrid* igrid = new QGrid(2, paramgroup);
//    for(int n = 0; n < max_params; n++) {
//      paramlist[n] = new QLabel(QString::number(n + 1).leftJustify(12),igrid);
//      inputlist[n] = new QLineEdit(igrid);
//    }
//    top->addWidget(paramgroup);
}

void QEChainOperatorInput::update_results(void) { 
  QEObjectMap* omap = dynamic_cast<QEObjectMap*>(maptab_rep->currentPage());
  chainop_rep = dynamic_cast<CHAIN_OPERATOR*>(omap->result());
}
