// ------------------------------------------------------------------------
// qeobjectmap.cpp: Object map input widget
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
#include <qgrid.h>

#include <string>

#include "eca-object.h"
#include "eca-object-map.h"

#include "qeobjectmap.h"

QEObjectMap::QEObjectMap (ECA_OBJECT_MAP* omap, QWidget *parent, const char *name) 
  : QEInput(parent, name),
    omap_rep(omap) {
  init_layout();
}

void QEObjectMap::init_layout(void) {
  QBoxLayout* top = new QVBoxLayout(this);

  QGroupBox* objgroup = new QVGroupBox(this, "objgroup");
  QListBox* objlist = new QListBox(objgroup, "objlist");

  const map<string,string>& omap = omap_rep->registered_objects();
  map<string,string>::const_iterator p = omap.begin();
  while(p != omap.end()) {
    objlist->insertItem(p->first.c_str());
    ++p;
  }
  objlist->setSelected(0, true);
  top->addWidget(objgroup);

  QObject::connect(objlist, SIGNAL(highlighted(int)), this, SLOT(update_object(int)));
}

void QEObjectMap::update_results(void) {}
void QEObjectMap::update_object(int index) {
  // --------
  REQUIRE(index >= 0);
  // --------

  object_rep = 0;
  const map<string,string>& omap = omap_rep->registered_objects();
  int counter = 0;
  map<string,string>::const_iterator p = omap.begin();
  while(p != omap.end()) {
    if (counter == index) {
      object_rep = dynamic_cast<ECA_OBJECT*>(omap_rep->object(p->second));
      selected_index = counter;
      emit changed();
    }
    ++p;
    ++counter;
  }

  // --------
  ENSURE(object_rep != 0);
  // --------
}
