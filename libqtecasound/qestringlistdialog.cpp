// ------------------------------------------------------------------------
// qestringlistdialog.cpp: Dialog widget that asks user to select a
//                         set of strings from a list of alternatives
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
#include <qpushbutton.h>
#include <qaccel.h>
#include <qlistbox.h>

#include "qestringlistdialog.h"

QEStringListDialog::QEStringListDialog (const QString& prompt, 
					const vector<string> items, 
					QWidget *parent	= 0, 
					const char *name = 0) 
  : QDialog(parent, name, true),
    alternatives_rep(items)
{
  setCaption(prompt);

  init_layout();
}

void QEStringListDialog::update_results(void) {
  items_rep.clear();
  for(int n = 0; n < item_input_repp->count(); n++) {
    if (item_input_repp->isSelected(item_input_repp->item(n))) {
      items_rep.push_back(item_input_repp->item(n)->text().latin1());
      cerr << "Added: " << items_rep.back() << endl;
    }
  }
  
  accept();
}

void QEStringListDialog::init_layout(void) { 
  QBoxLayout* top_layout = new QVBoxLayout(this);
  item_input_repp = new QListBox(this, "itemlist");
  item_input_repp->setSelectionMode(QListBox::Multi);

  vector<string>::const_iterator p = alternatives_rep.begin();
  while(p != alternatives_rep.end()) {
    item_input_repp->insertItem(p->c_str());
    ++p;
  }
  top_layout->addWidget(item_input_repp);
}
