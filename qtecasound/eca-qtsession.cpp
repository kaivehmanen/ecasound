// ------------------------------------------------------------------------
// eca-qtsession.cpp: Qt widget representing a ECA_SESSION object and 
//                    its state.
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

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

#include <qwidget.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qfont.h>
#include <qlistview.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qaccel.h>

#include "qstringdialog.h"

#include "eca-qtchainsetup.h"

#include "eca-session.h"
#include "eca-chainsetup.h"
#include "eca-controller.h"
#include "eca-qtinte.h"
#include "eca-qtsession.h"
#include "eca-error.h"

QESession::QESession (ECA_CONTROLLER* econtrol, const ECA_SESSION*
		      esession, QWidget *parent, const char *name)

  : QWidget( parent, name ), ctrl(econtrol), ecasession(esession)
{
  setMinimumSize( 600, 0 );
  //  setMaximumSize( 1024, 768 );

  timer_id = startTimer(100);
  current_dir = "";

  setCaption("qtecasound - session setup window");

  QBoxLayout* topLayout = new QVBoxLayout( this );
  QBoxLayout* buttons = new QHBoxLayout();

  init_chainsetuplist();
  init_buttons(buttons);

  init_shortcuts();

  topLayout->addLayout(buttons, 1);
  topLayout->addWidget(chainsetupview, 0, 0);

  setFocusPolicy(QWidget::ClickFocus);

  QObject::connect( chainsetupview,
		    SIGNAL(doubleClicked(QListViewItem*)), 
		    this, SLOT(button_chainsetup_clicked(QListViewItem*)));
  QObject::connect( chainsetupview,
		    SIGNAL(returnPressed(QListViewItem*)), 
		    this, SLOT(button_chainsetup_clicked(QListViewItem*)));

}

void QESession::closeEvent( QCloseEvent *e )
{
  emit session_closed();
  e->accept();      // hides the widget
}

void QESession::timerEvent( QTimerEvent * ) {
  static bool jep = false;
  if (jep == false) {
    killTimer(timer_id);
    startTimer(1000);
  }
  else {
    jep = true;
  }

  update_chainsetuplist();
}

void QESession::init_shortcuts(void) {
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

  a->connectItem(a->insertItem(Key_C), this,
		 SLOT(button_toggle_connected()));
  a->connectItem(a->insertItem(SHIFT+Key_C), this,
		 SLOT(button_toggle_connected()));
  a->connectItem(a->insertItem(CTRL+Key_C), this,
		 SLOT(button_toggle_connected()));

  a->connectItem(a->insertItem(Key_D), this,
		 SLOT(button_del()));
  a->connectItem(a->insertItem(SHIFT+Key_D), this,
		 SLOT(button_del()));
  a->connectItem(a->insertItem(CTRL+Key_D), this,
		 SLOT(button_del()));

  a->connectItem(a->insertItem(Key_E), this,
		 SLOT(button_edit_chainsetup()));
  a->connectItem(a->insertItem(SHIFT+Key_E), this,
		 SLOT(button_edit_chainsetup()));
  a->connectItem(a->insertItem(CTRL+Key_E), this,
		 SLOT(button_edit_chainsetup()));

  a->connectItem(a->insertItem(Key_F), chainsetupview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(SHIFT+Key_F), chainsetupview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(CTRL+Key_F), chainsetupview,
		 SLOT(setFocus()));

  a->connectItem(a->insertItem(Key_L), this,
		 SLOT(button_load()));
  a->connectItem(a->insertItem(SHIFT+Key_L), this,
		 SLOT(button_load()));
  a->connectItem(a->insertItem(CTRL+Key_L), this,
		 SLOT(button_load()));

  a->connectItem(a->insertItem(Key_N), this,
		 SLOT(button_new()));
  a->connectItem(a->insertItem(SHIFT+Key_N), this,
		 SLOT(button_new()));
  a->connectItem(a->insertItem(CTRL+Key_N), this,
		 SLOT(button_new()));

  a->connectItem(a->insertItem(Key_O), this,
		 SLOT(button_open_chainsetup()));
  a->connectItem(a->insertItem(SHIFT+Key_O), this,
		 SLOT(button_open_chainsetup()));
  a->connectItem(a->insertItem(CTRL+Key_O), this,
		 SLOT(button_open_chainsetup()));

  a->connectItem(a->insertItem(Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(SHIFT+Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(CTRL+Key_Q), this,
		 SLOT(close()));

  a->connectItem(a->insertItem(Key_S), this,
		 SLOT(button_save()));
  a->connectItem(a->insertItem(SHIFT+Key_S), this,
		 SLOT(button_save()));
  a->connectItem(a->insertItem(CTRL+Key_S), this,
		 SLOT(button_save()));
}

void QESession::init_buttons(QBoxLayout* buttons) {
  QFont butfont ("Helvetica", 12, QFont::Normal);

  QPushButton* cpanelbut = new QPushButton( "(!) Control panel", this, "cpanelbut" );
  cpanelbut->setFont(butfont);
  buttons->addWidget( cpanelbut, 1, 0);

  QPushButton* ffocus = new QPushButton( "(F)ocus to list", this, "ffocus" );
  ffocus->setFont(butfont);
  buttons->addWidget( ffocus, 1, 0);

  QPushButton* newb = new QPushButton( "(N)ew", this, "newb" );
  newb->setFont(butfont);
  buttons->addWidget( newb, 1, 0 );

  QPushButton* load = new QPushButton( "(L)oad", this, "load" );
  load->setFont(butfont);
  buttons->addWidget( load, 1, 0 );

  QPushButton* save = new QPushButton( "(S)ave", this, "save" );
  save->setFont(butfont);
  buttons->addWidget( save, 1, 0 );

  QPushButton* del = new QPushButton( "(D)elete", this, "del" );
  del->setFont(butfont);
  buttons->addWidget( del, 1, 0 );

  QPushButton* activate = new QPushButton( "(C)onnect/disconnect", this, "activat" );
  activate->setFont(butfont);
  buttons->addWidget( activate, 1, 0 );

  QPushButton* open = new QPushButton( "(O)pen", this, "open" );
  open->setFont(butfont);
  buttons->addWidget( open, 2, 0 );

  QPushButton* edit = new QPushButton( "(E)dit", this, "edit" );
  edit->setFont(butfont);
  buttons->addWidget( edit, 2, 0 );

  QPushButton* quit = new QPushButton( "(Q)uit", this, "quit" );
  quit->setFont(butfont);
  buttons->addWidget( quit, 1, 0);

  QObject::connect( cpanelbut, SIGNAL(clicked()), 
		    reinterpret_cast<QEInterface*>(qApp->mainWidget()), 
		    SLOT(get_focus()));

  //		    reinterpret_cast<QObject*>(qApp->mainWidget()), 
  //		    SLOT(setFocus()));

  QObject::connect( load, SIGNAL(clicked()), this, SLOT(button_load()));
  QObject::connect( save, SIGNAL(clicked()), this, SLOT(button_save()));
  QObject::connect( newb, SIGNAL(clicked()), this, SLOT(button_new()));
  QObject::connect( del, SIGNAL(clicked()), this, SLOT(button_del()));
  QObject::connect( activate, SIGNAL(clicked()), this, SLOT(button_toggle_connected()));
  QObject::connect( open, SIGNAL(clicked()), this, SLOT(button_open_chainsetup()));
  QObject::connect( edit, SIGNAL(clicked()), this, SLOT(button_edit_chainsetup()));
  QObject::connect( ffocus, SIGNAL(clicked()), chainsetupview, SLOT(setFocus()));
  QObject::connect( quit, SIGNAL(clicked()), this, SLOT(close()));
  //  connect(rewind, SIGNAL(clicked()), this, SLOT(emsg_rewind()) );
  //  connect(quit, SIGNAL(clicked()), this, SLOT(emsg_quit()) );
}

void QESession::init_chainsetuplist (void) {

  //  chainsetupview = new QListView_dumb(this, "chainsetupview");
  chainsetupview = new QListView(this, "chainsetupview");

  //chainsetupview->setMinimumSize( 600, 100 );

  chainsetupview->addColumn("Chainsetup");
  chainsetupview->addColumn("Inputs");
  chainsetupview->addColumn("Outputs");
  chainsetupview->addColumn("Chains");
  chainsetupview->addColumn("Status");

//    chainsetupview->setColumnAlignment(0, AlignLeft);
//    chainsetupview->setColumnAlignment(1, AlignRight);
//    chainsetupview->setColumnAlignment(2, AlignRight);
//    chainsetupview->setColumnAlignment(3, AlignRight);
//    chainsetupview->setColumnAlignment(4, AlignRight);

  chainsetupview->setAllColumnsShowFocus(true); 
  chainsetupview->setSorting(0);

  update_chainsetuplist();
  chainsetupview->setGeometry(0, 0, width() - 4, chainsetupview->height()
			      - 15);

  chainsetupview->show();
}

void QESession::update_chainsetuplist (void) {
  QListViewItem* selected = chainsetupview->selectedItem();
  QString selname = ""; 
  if (selected != 0) selname = selected->text(0);
  chainsetupview->clear();

  int pixelsleft = width();
  for(int n = 1; n < 5; n++) {
    pixelsleft -= chainsetupview->columnWidth(n);
  }

  if (pixelsleft > 0) {
    chainsetupview->setColumnWidthMode(0, QListView::Maximum);
    chainsetupview->setColumnWidth(0, pixelsleft - 4);
  }

  vector<ECA_CHAINSETUP*>::const_iterator p = ecasession->get_chainsetups().begin();
  while(p != ecasession->get_chainsetups().end()) {
    //    cerr << "Adding a new one!\n";
    QString astring;
    if (ctrl->connected_chainsetup() == (*p)->name())
      astring = "connected";

    QListViewItem* newitem = new QListViewItem(chainsetupview,
					       (*p)->name().c_str(),
					       kvu_numtostr((*p)->inputs.size()).c_str(),
					       kvu_numtostr((*p)->outputs.size()).c_str(),
					       kvu_numtostr((*p)->chains.size()).c_str(),
					       astring);
    if (newitem->text(0) == selname) chainsetupview->setSelected(newitem, true);
    ++p;
  }
  chainsetupview->triggerUpdate();
}

void QESession::button_load(void) {
  QFileDialog* fdialog = new QFileDialog();
  if (current_dir != "") fdialog->setDir(current_dir);
  QString filename = fdialog->getOpenFileName();
  current_dir = fdialog->dirPath();
  if (!filename.isEmpty()) {
    try {
      ctrl->load_chainsetup(string(filename.latin1()));
    }
    catch(ECA_ERROR* e) {
      if (e->error_action() != ECA_ERROR::stop) {
	QMessageBox* mbox = new QMessageBox(this, "mbox");
	QString errormsg = "";
	errormsg += "Error while loading chainsetup: \"";
	errormsg += e->error_section().c_str();
	errormsg += "\"; ";
	errormsg += e->error_msg().c_str();
	mbox->information(this, "qtecasound", errormsg,0);
      }
      else throw;
    }

    update_chainsetuplist();
  }
}

bool QESession::is_chainsetup_highlighted(void) const {
  QListViewItem* item = chainsetupview->selectedItem();
  if (item == 0) item = chainsetupview->currentItem();
  if (item != 0)  return(true);
  return(false);
}

void QESession::select_highlighted_chainsetup(void) {
  QListViewItem* item = chainsetupview->selectedItem();
  if (item == 0) item = chainsetupview->currentItem();
  if (item != 0) {
      ctrl->select_chainsetup(item->text(0).latin1());
  }
}

void QESession::button_save(void) {
  QFileDialog* fdialog = new QFileDialog();
  try {
    if (is_chainsetup_highlighted()) {
      select_highlighted_chainsetup();

      QString fdefault (ctrl->chainsetup_filename().c_str());
      if (current_dir != "") fdialog->setDir(current_dir);
      QString filename = fdialog->getSaveFileName(fdefault);
      current_dir = fdialog->dirPath();
      if (!filename.isEmpty()) {
	ctrl->save_chainsetup(filename.latin1());
      }
    }
    else
      QMessageBox::information(this, "qtecasound", "No chainsetup selected!",0);
  }
  catch(ECA_ERROR* e) {
    if (e->error_action() != ECA_ERROR::stop) {
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      QString errormsg ("Error while saving chainsetup. ");
      errormsg += e->error_section().c_str();
      errormsg += ": ";
      errormsg += e->error_msg().c_str();
      mbox->information(this, "qtecasound", errormsg,0);
    }
    else throw;
  }
}

void QESession::button_new(void) {
  try {
    QStringDialog* sdialog = new QStringDialog("Chainsetup name: ", this);
    if (sdialog->exec() == QStringDialog::Accepted) {
      ctrl->add_chainsetup(sdialog->resultString().latin1());
      update_chainsetuplist();
    }
  }
  catch(ECA_ERROR* e) {
    if (e->error_action() != ECA_ERROR::stop) {
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      
      QString errormsg = "";
      errormsg += "Error while creating new chainsetup. ";
      errormsg += e->error_section().c_str();
      errormsg += ": ";
      errormsg += e->error_msg().c_str();
      mbox->information(this, "qtecasound", errormsg,0);
    }
    else throw;
  }
}

void QESession::button_del(void) {
  try {
    if (is_chainsetup_highlighted()) {
      select_highlighted_chainsetup();
      if (ctrl->selected_chainsetup() == ctrl->connected_chainsetup()) {
	ctrl->disconnect_chainsetup();
      }
      ctrl->remove_chainsetup();
      update_chainsetuplist();
    }
    else
      QMessageBox::information(this, "qtecasound", "No chainsetup selected!",0);
  }
  catch(ECA_ERROR* e) {
    if (e->error_action() != ECA_ERROR::stop) {
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      QString errormsg ("Error while deleting chainsetup. ");
      errormsg += e->error_section().c_str();
      errormsg += ": ";
      errormsg += e->error_msg().c_str();
      mbox->information(this, "qtecasound", errormsg,0);
    }
    else throw;
  }
}

void QESession::button_toggle_connected(void) {
  try {
    if (is_chainsetup_highlighted()) {
      select_highlighted_chainsetup();
      if (ctrl->is_valid() == false) {
	QMessageBox::information(this, "qtecasound", "Selected chainsetup not valid! Can't connect.",0);
	return;
      }
     
      if (ctrl->selected_chainsetup() != ctrl->connected_chainsetup()) {
	ctrl->connect_chainsetup();
      }
      else {
	ctrl->disconnect_chainsetup();
      }
      update_chainsetuplist();
    }
    else
      QMessageBox::information(this, "qtecasound", "No chainsetup selected!",0);
  }
  catch(ECA_ERROR* e) {
    if (e->error_action() != ECA_ERROR::stop) {
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      QString errormsg ("Error while connecting/disconnecting chainsetup. ");
      errormsg += e->error_section().c_str();
      errormsg += ": ";
      errormsg += e->error_msg().c_str();
      mbox->information(this, "qtecasound", errormsg,0);
    }
    else throw;
  }
}

void QESession::button_open_chainsetup(void) { 
  QListViewItem* item = chainsetupview->selectedItem();
  if (item == 0) item = chainsetupview->currentItem();
  if (item == 0) {
    QMessageBox::information(this, "qtecasound", "Can't open! No chainsetup selected.",0);
  }
  else {
    ctrl->select_chainsetup(item->text(0).latin1());
    if (ctrl->is_selected() == false) return;
    QEChainsetup* active_csetup = new QEChainsetup(ctrl,
				     ctrl->get_chainsetup());

    active_csetup->show();
  }
}

void QESession::button_edit_chainsetup(void) { 
  try {
    if (is_chainsetup_highlighted()) {
      select_highlighted_chainsetup();
      if (ctrl->selected_chainsetup() != ctrl->connected_chainsetup()) {
	ctrl->edit_chainsetup();
	update_chainsetuplist();
      }
      else {
	QMessageBox::information(this, "qtecasound", "Can't edit connected chainsetup. Disconnect and try again.",0);
      }
    }
    else
      QMessageBox::information(this, "qtecasound", "Can't edit! No chainsetup selected.",0);
  }
  catch(ECA_ERROR* e) {
    if (e->error_action() != ECA_ERROR::stop) {
      QMessageBox* mbox = new QMessageBox(this, "mbox");
      QString errormsg ("Error while editing chainsetup. ");
      errormsg += e->error_section().c_str();
      errormsg += ": ";
      errormsg += e->error_msg().c_str();
      mbox->information(this, "qtecasound", errormsg,0);
    }
    else throw;
  }
}

void QESession::button_chainsetup_clicked(QListViewItem* i) { 
  if (i != 0) button_open_chainsetup();
}




