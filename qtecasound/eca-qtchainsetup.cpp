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
  : ctrl_repp(econtrol)
{
  startTimer(1000);

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

  chain_list_repp->setAllColumnsShowFocus(true); 
  chain_list_repp->setSorting(0);

  update_chain_list();
}

void QEChainsetup::update_chain_list(void) { 
  // HEI!
  // Tähän systeemi joka käy läpi listviewin (tyhjentämättä sitä) 
  // ja päivittää vain tarvittavat. 
  // --
  // selected chainit merkataan myös widgetissä valituiksi!
  // --
  
  QListViewItem* selected = chain_list_repp->selectedItem();
  QString selname = ""; 
  if (selected != 0) selname = selected->text(0);
  chain_list_repp->clear();

  vector<string> chain_names = ctrl_repp->chain_names();
  vector<string>::const_iterator p = chain_names.begin();
  while(p != chain_names.end()) {
    ctrl_repp->select_chain(*p);
    CHAIN* current_chain = ctrl_repp->get_chain();
    QString chain_name = "";
    QString input_name = "none";
    QString output_name = "none";
    QString status_string = "";
    if (current_chain != 0) {
      chain_name = current_chain->name().c_str();
      if (current_chain->connected_input() != 0) 
	input_name = current_chain->connected_input()->label().c_str();
      if (current_chain->connected_output() != 0) 
	output_name = current_chain->connected_output()->label().c_str();
      if (current_chain->is_processing() != true)
	status_string += "bypassed";
      if (current_chain->is_muted() == true) {
	if (status_string.isEmpty() != true) status_string += ",";
	status_string += "muted";
      }
    }
    QListViewItem* newitem = new QListViewItem(chain_list_repp,
					       chain_name,
					       input_name,
					       output_name,
					       status_string);
    if (newitem->text(0) == selname) chain_list_repp->setSelected(newitem, true);
    ++p;
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

void QEChainsetup::timerEvent( QTimerEvent * ) {
  update_chain_list();
}
