// ------------------------------------------------------------------------
// eca-qtdebugtab.cpp: Wrapper class for QEDebug
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
#include <qpushbutton.h>

#include "qebuttonrow.h"
#include "eca-qtinte.h"
#include "eca-qtdebug.h"
#include "eca-qtdebugtab.h"

QEDebugTab::QEDebugTab(QEInterface *interface = 0, QWidget* parent = 0, const char *name)
        : QWidget(parent, name) {
  QBoxLayout* vbox = new QVBoxLayout(this);

  QEButtonRow* buttonrow = new QEButtonRow(this, "buttonrow");
  buttonrow->add_button(new QPushButton("Stat(u)s",buttonrow), 
			ALT+Key_U,
			interface, SLOT(emsg_status()));
  buttonrow->add_button(new QPushButton("C(h)ainsetups",buttonrow), 
			ALT+Key_H,
			interface, SLOT(emsg_csstatus()));
  buttonrow->add_button(new QPushButton("F(X)",buttonrow), 
			ALT+Key_X,
			interface, SLOT(emsg_estatus()));
  buttonrow->add_button(new QPushButton("(C)trls",buttonrow), 
			ALT+Key_C,
			interface, SLOT(emsg_ctrlstatus()));
  buttonrow->add_button(new QPushButton("Ch(a)ins",buttonrow), 
			ALT+Key_A,
			interface, SLOT(emsg_cstatus()));
  buttonrow->add_button(new QPushButton("Fi(l)es",buttonrow), 
			ALT+Key_L,
			interface, SLOT(emsg_fstatus()));
  vbox->addWidget(buttonrow);

  QEDebug* qdebug = new QEDebug(this, "qdebug");
  vbox->addWidget(qdebug, 1);
}
