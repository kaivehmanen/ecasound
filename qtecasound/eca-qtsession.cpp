// ------------------------------------------------------------------------
// eca-qtsession.cpp: Qt widget representing a ecasound session object and 
//                    its state.
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

#include "qebuttonrow.h"
#include "qestringdialog.h"

#include "eca-qtchainsetup.h"
#include "eca-session.h"
#include "eca-chainsetup.h"
#include "eca-controller.h"
#include "eca-qtinte.h"
#include "eca-qtsession.h"
#include "eca-error.h"

QESession::QESession (ECA_CONTROLLER* econtrol, const ECA_SESSION*
		      esession, QWidget *parent, const char *name)
  : QWidget(parent, name), ctrl(econtrol), ecasession(esession)
{
  timer_id = startTimer(100);
  current_dir = "";
  child_csetup = 0;

  setCaption("qtecasound - session setup window");
  QBoxLayout* topLayout = new QVBoxLayout(this);

  init_chainsetuplist();
  init_buttons();

  topLayout->addWidget(buttons);
  topLayout->addWidget(chainsetupview, 0, 0);

  setFocusPolicy(QWidget::ClickFocus);
  QObject::connect( chainsetupview,
		    SIGNAL(doubleClicked(QListViewItem*)), 
		    this, SLOT(button_chainsetup_clicked(QListViewItem*)));
  QObject::connect( chainsetupview,
		    SIGNAL(returnPressed(QListViewItem*)), 
		    this, SLOT(button_chainsetup_clicked(QListViewItem*)));
}

void QESession::closeEvent(QCloseEvent *e) {
  if (child_csetup != 0) {
    child_csetup->close(true);
  }
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

void QESession::init_buttons(void) {
  buttons = new QEButtonRow(this, "buttonrow");
  buttons->add_button(new QPushButton("(F)ocus to list",buttons), 
		      ALT+Key_F,
		      chainsetupview,
		      SLOT(setFocus()));

  buttons->add_button(new QPushButton("(N)ew",buttons), 
		      ALT+Key_N,
		      this,
		      SLOT(button_new()));

  buttons->add_button(new QPushButton("(L)oad",buttons), 
		      ALT+Key_L,
		      this,
		      SLOT(button_load()));

  buttons->add_button(new QPushButton("(S)ave",buttons), 
		      ALT+Key_S,
		      this,
		      SLOT(button_save()));

  buttons->add_button(new QPushButton("(R)emove",buttons), 
		      ALT+Key_R,
		      this,
		      SLOT(button_del()));

  buttons->add_button(new QPushButton("(D)is/connect",buttons), 
		      ALT+Key_D,
		      this,
		      SLOT(button_toggle_connected()));

  buttons->add_button(new QPushButton("(O)pen",buttons), 
		      ALT+Key_O,
		      this,
		      SLOT(button_open_chainsetup()));

  buttons->add_button(new QPushButton("(E)dit",buttons), 
		      ALT+Key_E,
		      this,
		      SLOT(button_edit_chainsetup()));
}

void QESession::init_chainsetuplist (void) {
  chainsetupview = new QListView(this, "chainsetupview");

  chainsetupview->addColumn("Chainsetup");
  chainsetupview->addColumn("Inputs");
  chainsetupview->addColumn("Outputs");
  chainsetupview->addColumn("Chains");
  chainsetupview->addColumn("Status");

  chainsetupview->setAllColumnsShowFocus(true); 
  chainsetupview->setSorting(0);

  update_chainsetuplist();
  //  chainsetupview->setGeometry(0, 0, width() - 4, chainsetupview->height() - 15);

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
    QEStringDialog* sdialog = new QEStringDialog("Chainsetup name: ", this);
    if (sdialog->exec() == QEStringDialog::Accepted) {
      ctrl->add_chainsetup(sdialog->result_string().latin1());
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
    if (child_csetup != 0) {
      child_csetup->close(true);
    }
    child_csetup = new QEChainsetup(ctrl, ctrl->get_chainsetup());
    child_csetup->show();
    connect(child_csetup, SIGNAL(widget_closed()), this, SLOT(child_setup_closed()));
  }
}

void QESession::child_setup_closed(void) { child_csetup = 0; }

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
