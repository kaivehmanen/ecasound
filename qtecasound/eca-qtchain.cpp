// ------------------------------------------------------------------------
// eca-qtchain: Qt widget representing a CHAIN object and its state.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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
#include <qaccel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include <kvutils.h>

#include "eca-chain.h"
#include "eca-controller.h"

// #include "qlistview-dumb.h"
#include "eca-qtinte.h"
#include "eca-qtchain.h"

QEChain::QEChain(ECA_CONTROLLER* control, const CHAIN* ch, QWidget *parent, const char *name )
        : QWidget( parent, name ), ctrl(control), chain(ch)
{
  setMinimumSize( 600, 0 );

  startTimer(500);
  
  QString caption = "qtecasound - chain: " + QString(ch->name().c_str());
  setCaption(caption);

  QBoxLayout* topLayout = new QVBoxLayout( this );
  buttons = new QHBoxLayout();

  init_chainlist();

  init_buttons();
  init_shortcuts();

  topLayout->addLayout(buttons, 1);
  topLayout->addWidget(chainview, 0, 0);
}

void QEChain::not_implemented(void) {
  QMessageBox* mbox = new QMessageBox(this, "mbox");
  mbox->information(this, "qtecasound", "This feature is not implemented...",0);
}

void QEChain::init_shortcuts(void) {
  QAccel *a = new QAccel(this);

  a->connectItem(a->insertItem(Key_Exclam),
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()),
		 SLOT(get_focus()));

  a->connectItem(a->insertItem(Key_Exclam),
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()),
		 SLOT(get_focus()));

  a->connectItem(a->insertItem(SHIFT+Key_Exclam),
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()),
		 SLOT(get_focus()));

  a->connectItem(a->insertItem(Key_A), this,
		 SLOT(not_implemented()));
  a->connectItem(a->insertItem(SHIFT+Key_A), this,
		 SLOT(not_implemented()));
  a->connectItem(a->insertItem(CTRL+Key_A), this,
		 SLOT(not_implemented()));

  a->connectItem(a->insertItem(Key_F), chainview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(SHIFT+Key_F), chainview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(CTRL+Key_F), chainview,
		 SLOT(setFocus()));

  a->connectItem(a->insertItem(Key_P), this,
		 SLOT(not_implemented()));
  a->connectItem(a->insertItem(SHIFT+Key_P), this,
		 SLOT(not_implemented()));
  a->connectItem(a->insertItem(CTRL+Key_P), this,
		 SLOT(not_implemented()));

  a->connectItem(a->insertItem(Key_R), this,
		 SLOT(not_implemented()));
  a->connectItem(a->insertItem(SHIFT+Key_R), this,
		 SLOT(not_implemented()));
  a->connectItem(a->insertItem(CTRL+Key_R), this,
		 SLOT(not_implemented()));

  a->connectItem(a->insertItem(Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(SHIFT+Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(CTRL+Key_Q), this,
		 SLOT(close()));
}

void QEChain::init_buttons(void) {
  QFont butfont ("Helvetica", 12, QFont::Normal);

  QPushButton* cpanelbut = new QPushButton( "(!) Control panel", this, "cpanelbut" );
  cpanelbut->setFont(butfont);
  buttons->addWidget( cpanelbut, 1, 0);

  QObject::connect( cpanelbut, SIGNAL(clicked()), 
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()), 
		 SLOT(get_focus()));

  QPushButton* ffocus = new QPushButton( "(F)ocus to list", this, "ffocus" );
  ffocus->setFont(butfont);
  buttons->addWidget( ffocus, 1, 0);

  QPushButton* add = new QPushButton( "(A)dd", this, "add" );
  add->setFont(butfont);
  buttons->addWidget( add, 1, 0);

  QPushButton* remove = new QPushButton( "(R)emove", this, "remove" );
  remove->setFont(butfont);
  buttons->addWidget( remove, 1, 0);

  QPushButton* param = new QPushButton( "(P)arameters", this, "param" );
  param->setFont(butfont);
  buttons->addWidget( param, 1, 0);

  QPushButton* quit = new QPushButton( "(Q)uit", this, "quit" );
  quit->setFont(butfont);
  buttons->addWidget( quit, 1, 0);

  QObject::connect( add, SIGNAL(clicked()), this, SLOT(not_implemented()));
  QObject::connect( remove, SIGNAL(clicked()), this, SLOT(not_implemented()));
  QObject::connect( param, SIGNAL(clicked()), this, SLOT(not_implemented()));
  QObject::connect( ffocus, SIGNAL(clicked()), chainview,
		    SLOT(setFocus()));
  QObject::connect( quit, SIGNAL(clicked()), this, SLOT(close()));
  //  connect(quit, SIGNAL(clicked()), this, SLOT(emsg_quit()) );
}

void QEChain::init_chainlist (void) {

  //  chainview = new QListView_dumb(this, "chainview");
  chainview = new QListView(this, "chainview");

  //  chainsetupview->setMaximumSize( width() / 2, height() / 2);
  chainview->setMinimumSize( 600, 100 );
       
  chainview->addColumn("Operator");
  chainview->addColumn("Params");

  chainview->setAllColumnsShowFocus(true); 
  chainview->setSorting(-1);

  update_chainlist_clean();

  chainview->setGeometry(0, 0, width(), 100);

  chainview->show();
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


