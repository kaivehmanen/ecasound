// ------------------------------------------------------------------------
// eca-qtchainsetup: Qt widget representing an ecasound chainsetup.
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

#include <cmath>
#include <iostream>
#include <string>

#include <qwidget.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlistview.h>

#include <kvutils.h>
#include "qestringdialog.h"
#include "qebuttonrow.h"
#include "eca-chain.h"
#include "audioio-types.h"
#include "eca-control.h"
#include "eca-chainsetup.h"

#include "eca-qtinte.h"
#include "eca-qtchainsetup.h"
//  #include "eca-qtchain.h"
//  #include "eca-qtiodevdialog.h"
//  #include "eca-qtchainselectdialog.h"

QEChainsetup::QEChainsetup (ECA_CONTROL* econtrol,
			    QWidget *parent,
			    const char *name) 
  : QWidget(parent, name),
    ctrl_repp(econtrol)
{
  startTimer(1000);
  user_input_lock_rep = false;

  top_layout_repp = new QVBoxLayout( this );
  init_buttons();
  top_layout_repp->addWidget(buttons_repp);
  init_chain_list();
  top_layout_repp->addWidget(chain_list_repp);
//    QObject::connect(chain_list_repp,
//  		   SIGNAL(doubleClicked(QListViewItem*)), 
//  		   this, 
//  		   SLOT(init_chainview(QListViewItem*)));
//    QObject::connect(chain_list_repp,
//  		   SIGNAL(returnPressed(QListViewItem*)), 
//  		   this, 
//  		   SLOT(init_chainview(QListViewItem*)));
}

void QEChainsetup::closeEvent(QCloseEvent *e) {
  emit widget_closed();
  e->accept();
}

void QEChainsetup::not_implemented(void) {
  QMessageBox* mbox = new QMessageBox(this, "mbox");
  mbox->information(this, "qtecasound", "This feature is not implemented...",0);
}

void QEChainsetup::init_chain_list(void) {
  chain_list_repp = new QListView(this, "chain_listview");

  chain_list_repp->addColumn("Chain");
  chain_list_repp->addColumn("Input");
  chain_list_repp->addColumn("Output");
  chain_list_repp->addColumn("Status");
  chain_list_repp->addColumn("Operators");

  chain_list_repp->setAllColumnsShowFocus(true); 
  chain_list_repp->setSorting(0);
  chain_list_repp->setMultiSelection(true);
  for(int n = 0; n < 5; n++) { // n < chain_list_repp->columns()
    chain_list_repp->setColumnWidthMode(n, QListView::Manual);
  }

  QObject::connect(chain_list_repp,
  		   SIGNAL(selectionChanged()), 
		   this,
  		   SLOT(select_chains()));

  update_chain_list();
}

void QEChainsetup::update_chain_list(void) { 
  QString chain_name, input_name, output_name, status_string, chain_string;
  vector<string> saved = ctrl_repp->selected_chains();
  vector<string> chain_names = ctrl_repp->chain_names();
  vector<string>::const_iterator p = chain_names.begin();
  QListViewItem* selected = chain_list_repp->firstChild();
  while (p != chain_names.end()) {
    ctrl_repp->select_chain(*p);
    CHAIN* current_chain = ctrl_repp->get_chain();
    if (current_chain == 0) continue;
    chain_name = current_chain->name().c_str();
    if (current_chain->connected_input() != 0) 
      input_name = current_chain->connected_input()->label().c_str();
    if (current_chain->connected_output() != 0) 
      output_name = current_chain->connected_output()->label().c_str();
    chain_string = current_chain->to_string().c_str();
    if (current_chain->is_processing() != true)
      status_string += "bypassed";
    if (current_chain->is_muted() == true) {
      if (status_string.isEmpty() != true) status_string += ",";
      status_string += "muted";
    }
    if (selected != 0 && 
	chain_name == selected->text(0)) {
      selected->setText(1, input_name);
      selected->setText(2, output_name);
      selected->setText(3, status_string);
      selected->setText(4, chain_string);
      selected->repaint();
    }
    else {
      QListViewItem* newitem = new QListViewItem(chain_list_repp,
						 chain_name,
						 input_name,
						 output_name,
						 status_string,
						 chain_string);
    }

    if (selected != 0) selected = selected->nextSibling();
    ++p;
  }

  if (selected != 0) {
    QListViewItem* next = 0;
    while(selected != 0) {
      next = selected->nextSibling();
      chain_list_repp->takeItem(selected);
      selected = next;
    }
  }
  select_chains(saved);
}

void QEChainsetup::select_chains(const vector<string>& chains) {
  ctrl_repp->select_chains(chains);
  if (user_input_lock_rep != true) {
    user_input_lock_rep = true;
    chain_list_repp->clearSelection();
    vector<string>::const_iterator p = chains.begin();
    while (p != chains.end()) {
      QListViewItem* selected = chain_list_repp->firstChild();
      while (selected != 0) {
	if (QString(p->c_str()) == selected->text(0)) 
	  selected->setSelected(true);

	selected = selected->nextSibling();
      }
      ++p;
    }
    user_input_lock_rep = false;
  }
}

void QEChainsetup::select_chains(void) {
  if (user_input_lock_rep != true) {
    user_input_lock_rep = true;
    vector<string> selected_chains;
    
    QListViewItem* selected = chain_list_repp->firstChild();
    while (selected != 0) {
      if (selected->isSelected()) 
	selected_chains.push_back(selected->text(0).latin1());
      
      selected = selected->nextSibling();
    }
    
    ctrl_repp->select_chains(selected_chains);
    user_input_lock_rep = false;
  }
}

void QEChainsetup::update_chain_list_clean (void) { }

void QEChainsetup::init_buttons(void) { 
  buttons_repp = new QEButtonRow(this, "chain_buttonrow");

  buttons_repp->add_button(new QPushButton("(N)ew chain",buttons_repp), 
		      ALT+Key_N,
		      this,
		      SLOT(button_add_chain()));

  buttons_repp->add_button(new QPushButton("Remo(v)e chain",buttons_repp), 
		      ALT+Key_V,
		      this,
		      SLOT(button_remove_chain()));

  buttons_repp->add_button(new QPushButton("(M)uting",buttons_repp), 
		      ALT+Key_M,
		      this,
		      SLOT(button_chain_muting()));

  buttons_repp->add_button(new QPushButton("(B)ypass",buttons_repp), 
		      ALT+Key_B,
		      this,
		      SLOT(button_chain_bypass()));

//    buttons_repp->add_button(new QPushButton("(O)pen",buttons_repp), 
//  		      ALT+Key_O,
//  		      this,
//  		      SLOT(init_chain_list()));

  buttons_repp->add_button(new QPushButton("(A)dd",buttons_repp), 
		      ALT+Key_A,
		      this,
		      SLOT(button_add_file()));

  buttons_repp->add_button(new QPushButton("(R)emove from setup",buttons_repp), 
		      ALT+Key_R,
		      this,
		      SLOT(button_remove_file()));

//    buttons_repp->add_button(new QPushButton("Attach to chain(s)",buttons_repp), 
//  		      ALT+Key_S,
//  		      this,
//  		      SLOT(button_select_chain()));

//    buttons_repp->add_button(new QPushButton("(W)ave form edit",buttons_repp), 
//  		      ALT+Key_W,
//  		      this,
//  		      SLOT(init_wave_edit()));
}
 
void QEChainsetup::button_add_chain(void) { 
  QEStringDialog* sdialog = new QEStringDialog("Chain name: ", this);
  if (sdialog->exec() == QEStringDialog::Accepted) {
    // !!!
    // FIXME
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    ctrl_repp->add_chain(sdialog->result_string().latin1());
    update_chain_list();
  }
}

void QEChainsetup::button_remove_chain(void) { 
  if (ctrl_repp->is_selected() &&
      ctrl_repp->selected_chains().size() > 0) {
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    ctrl_repp->remove_chains();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::button_chain_muting(void) {
  if (ctrl_repp->is_selected() &&
      ctrl_repp->selected_chains().size() > 0) {
    ctrl_repp->toggle_chain_muting();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::button_chain_bypass(void) {
  if (ctrl_repp->is_selected() &&
      ctrl_repp->selected_chains().size() > 0) {
    ctrl_repp->toggle_chain_bypass();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::button_add_file(void) { 
//    QEIodevDialog* fdialog = new QEIodevDialog(chainsetup, this, "addfile");
//    if (fdialog->exec() == QEIodevDialog::Accepted) {
//      if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
//      ctrl_repp->set_default_audio_format(fdialog->result_audio_format());
//      ctrl_repp->select_chains(fdialog->result_chains());

//      if (fdialog->result_direction() == QEIodevDialog::input)
//        ctrl_repp->add_audio_input(fdialog->result_filename());
//      else
//        ctrl_repp->add_audio_output(fdialog->result_filename());

//      update_filesetuplist(true);
//      update_chainsetuplist_clean();
//    }
}

void QEChainsetup::button_remove_file(void) { 
//    if (is_filesetup_highlighted()) {
//      select_highlighted_filesetup();
//      if (ctrl_repp->is_connected()) {
//        ctrl_repp->disconnect_chainsetup();
//      }
//      if (ctrl_repp->get_audio_object() != 0) {
//        ctrl_repp->remove_audio_object();
//      }
//    }
//    else
//      QMessageBox::information(this, "qtecasound", "No file selected!",0);
}

void QEChainsetup::timerEvent(QTimerEvent *) { update_chain_list(); }
void QEChainsetup::resizeEvent(QResizeEvent *) {
  chain_list_repp->setColumnWidth(0, width() * 0.10f);
  chain_list_repp->setColumnWidth(1, width() * 0.25f);
  chain_list_repp->setColumnWidth(2, width() * 0.25f);
  chain_list_repp->setColumnWidth(3, width() * 0.15f);
  chain_list_repp->setColumnWidth(4, width() * 0.25f);
}

void QEChainsetup::mousePressEvent(QMouseEvent *) { 
  user_input_lock_rep = true;
}

void QEChainsetup::mouseReleaseEvent(QMouseEvent *) {
  user_input_lock_rep = false;
}
