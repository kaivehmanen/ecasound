// ------------------------------------------------------------------------
// eca-qtchain: Qt widget representing a CHAIN object and its state.
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <qapplication.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

#include <kvutils.h>

#include "qebuttonrow.h"

#include "eca-chain.h"
#include "eca-controller.h"

#include "eca-qtinte.h"
#include "eca-qtchain.h"

QEChain::QEChain(ECA_CONTROLLER* control, const CHAIN* ch, QWidget *parent, const char *name )
        : QWidget( parent, name ), ctrl(control), chain(ch)
{
  startTimer(500);
  
  QString caption = "qtecasound - chain: " + QString(ch->name().c_str());
  setCaption(caption);

  QBoxLayout* topLayout = new QVBoxLayout( this );

  init_chainlist();
  init_buttons();

  topLayout->addWidget(buttons);
  topLayout->addWidget(chainview);
}

void QEChain::closeEvent(QCloseEvent *e) {
  emit widget_closed();
  e->accept();
}

void QEChain::not_implemented(void) {
  QMessageBox* mbox = new QMessageBox(this, "mbox");
  mbox->information(this, "qtecasound", "This feature is not implemented...",0);
}

void QEChain::init_buttons(void) {
  buttons = new QEButtonRow(this, "buttonrow");
  buttons->add_button(new QPushButton("Control (p)anel",buttons),
		      ALT+Key_P,
		      reinterpret_cast<QEInterface*>(qApp->mainWidget()), 
		      SLOT(get_focus()));

  buttons->add_button(new QPushButton("(F)ocus to list",buttons), 
		      ALT+Key_F,
		      chainview,
		      SLOT(setFocus()));

  buttons->add_button(new QPushButton("(A)dd",buttons), 
		      ALT+Key_A,
		      this,
		      SLOT(not_implemented()));

  buttons->add_button(new QPushButton("(R)emove",buttons), 
		      ALT+Key_R,
		      this,
		      SLOT(not_implemented()));

  buttons->add_button(new QPushButton("(P)arameters",buttons), 
		      ALT+Key_P,
		      this,
		      SLOT(not_implemented()));

  buttons->add_button(new QPushButton("(P)arameters",buttons), 
		      ALT+Key_P,
		      this,
		      SLOT(not_implemented()));

  buttons->add_button(new QPushButton("(C)lose",buttons), 
		      ALT+Key_C,
		      this,
		      SLOT(close()));
}

void QEChain::init_chainlist (void) {
  chainview = new QListView(this, "chainview");

  chainview->addColumn("Operator");
  chainview->addColumn("Params");

  chainview->setAllColumnsShowFocus(true); 
  chainview->setSorting(-1);

  update_chainlist_clean();

  chainview->setGeometry(0, 0, width(), 100);
}

void QEChain::update_chainlist_clean (void) {
  QListViewItem* selected = chainview->selectedItem();
  QString selname = ""; 

  int pixelsleft = width() - 4;
  for(int n = 0; n < 1; n++) 
    pixelsleft -= chainview->columnWidth(n);
  
  if (pixelsleft > 0) {
    chainview->setColumnWidthMode(1, QListView::Maximum);
    chainview->setColumnWidth(1, pixelsleft);
  }

  if (selected != 0) selname = selected->text(0);

  chainview->clear();

  // NOTICE! temporary comment
//    string params = ""; 
//    vector<CHAIN_OPERATOR*>::const_iterator p = chain->chainops.begin();
//    while(p != chain->chainops.end()) {
//      params = ""; 
//      for(int n = 0; n < (*p)->number_of_params(); n++) {
//        params += (*p)->get_parameter_name(n + 1);
//        params += " ";
//        params += kvu_numtostr((*p)->get_parameter(n + 1));
//        if (n + 1 < (*p)->number_of_params()) params += ", ";
//      }
//      newitem = new QListViewItem(chainview,
//  				(*p)->name().c_str(),
//  				params.c_str());
//      if (newitem->text(0) == selname) chainview->setSelected(newitem, true);
//      ++p;
//    }


  chainview->triggerUpdate();
}

void QEChain::update_chainlist (void) {
  string params = ""; 
  // temporary comment
//    vector<CHAIN_OPERATOR*>::const_iterator p = chain->chainops.begin();
//    while(p != chain->chainops.end()) {
//      newitem = chainview->firstChild();
//      while(newitem != 0) {
//        if (newitem->text(0) ==
//  	  QString((*p)->name().c_str()))
//  	break;
//        newitem = newitem->nextSibling();
//      }
//      if (newitem == 0) {
//        ++p;
//        continue;
//      }

//      params = ""; 
//      for(int n = 0; n < (*p)->number_of_params(); n++) {
//        params += (*p)->get_parameter_name(n + 1);
//        params += " ";
//        params += kvu_numtostr((*p)->get_parameter(n + 1));
//        if (n + 1 < (*p)->number_of_params()) params += ", ";
//      }

//      newitem->setText(1, params.c_str());
//      ++p;
//    }

  chainview->triggerUpdate();
}

void QEChain::timerEvent( QTimerEvent * ) {
  update_chainlist();
}


