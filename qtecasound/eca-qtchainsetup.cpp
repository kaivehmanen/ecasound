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

#include <qapplication.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qpushbutton.h>

#include <kvutils.h>

#include "qestringdialog.h"
#include "qebuttonrow.h"

#include "eca-chain.h"
#include "audioio-types.h"
#include "eca-controller.h"
#include "eca-chainsetup.h"

#include "eca-qtinte.h"
#include "eca-qtchain.h"
#include "eca-qtchainsetup.h"
#include "eca-qtiodevdialog.h"
#include "eca-qtchainselectdialog.h"

QEChainsetup::QEChainsetup (ECA_CONTROLLER* econtrol,
			    QWidget *parent,
			    const char *name) 
  : ctrl_repp(econtrol)
{
  startTimer(1000);

  top_layout_repp = new QVBoxLayout( this );
  init_chain_list();
  init_buttons();
  top_layout_repp->addWidget(buttons_repp);

  QObject::connect(chain_list_repp,
		   SIGNAL(doubleClicked(QListViewItem*)), 
		   this, 
		   SLOT(init_chainview(QListViewItem*)));
  QObject::connect(chain_list_repp,
		   SIGNAL(returnPressed(QListViewItem*)), 
		   this, 
		   SLOT(init_chainview(QListViewItem*)));
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
  QListViewItem* item = chain_list_repp->selectedItem();
  if (item == 0) item = chain_list_repp->currentItem();

  // temporarily removed
  not_implemented();
  return;
}

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

  buttons_repp->add_button(new QPushButton("(O)pen",buttons_repp), 
		      ALT+Key_O,
		      this,
		      SLOT(init_chain_list()));

  buttons_repp->add_button(new QPushButton("(A)dd",buttons_repp), 
		      ALT+Key_A,
		      this,
		      SLOT(button_add_file()));

  buttons_repp->add_button(new QPushButton("(R)emove from setup",buttons_repp), 
		      ALT+Key_R,
		      this,
		      SLOT(button_remove_file()));

  buttons_repp->add_button(new QPushButton("Attach to chain(s)",buttons_repp), 
		      ALT+Key_S,
		      this,
		      SLOT(button_select_chain()));

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
    ctrl_repp->select_chainsetup("command-line-setup");
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    ctrl_repp->add_chain(sdialog->result_string().latin1());
    update_chain_list();
    update_chain_list_clean();
  }
}

void QEChainsetup::button_remove_chain(void) { 
  if (is_chain_highlighted()) {
    select_highlighted_chain();
    ctrl_repp->select_chainsetup(chainsetup->name());
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    ctrl_repp->remove_chains();
    update_chainsetuplist_clean();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::button_chain_muting(void) {
  if (is_chain_highlighted()) {
    select_highlighted_chain();
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    ctrl_repp->toggle_chain_muting();
    update_chainsetuplist();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::button_chain_bypass(void) {
  if (is_chain_highlighted()) {
    select_highlighted_chain();
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    ctrl_repp->toggle_chain_bypass();
    update_chainsetuplist();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

bool QEChainsetup::is_filesetup_highlighted(void) const {
  QListViewItem* item = filesetupview->selectedItem();
  if (item == 0) item = filesetupview->currentItem();
  if (item != 0)  return(true);
  return(false);
}

void QEChainsetup::select_highlighted_filesetup(void) {
  QListViewItem* item = filesetupview->selectedItem();
  if (item == 0) item = filesetupview->currentItem();
  if (item != 0) {
    ctrl_repp->select_chainsetup(chainsetup->name());
    ctrl_repp->select_audio_object(item->text(0).latin1());
  }
}

bool QEChainsetup::is_chain_highlighted(void) const {
  QListViewItem* item = chainsetupview->selectedItem();
  if (item == 0) item = chainsetupview->currentItem();
  if (item != 0)  return(true);
  return(false);
}

void QEChainsetup::select_highlighted_chain(void) {
  QListViewItem* item = chainsetupview->selectedItem();
  if (item == 0) item = chainsetupview->currentItem();
  if (item != 0) {
    ctrl_repp->select_chainsetup(chainsetup->name());
    vector<string> t (1, string(item->text(0).latin1()));
    ctrl_repp->select_chains(t);
  }
}

void QEChainsetup::init_filesetuplist (void) {
  filesetupview = new QListView(this, "filesetupview");

  filesetupview->addColumn("File");
  filesetupview->addColumn("Mode");
  filesetupview->addColumn("Bits/Ch/Rate");
  filesetupview->addColumn("Position/Length");
  filesetupview->addColumn("Realtime");
  filesetupview->addColumn("Status");
  filesetupview->addColumn("Chains");

  filesetupview->setAllColumnsShowFocus(true); 
  filesetupview->setSorting(6);

  update_filesetuplist(true);
  //  filesetupview->show();
}

void QEChainsetup::update_filesetuplist (bool clean) {
  filesetupview->setColumnWidthMode(0, QListView::Maximum);

  if (clean) {
    filesetupview->clear();
    update_filesetup_clean(chainsetup->inputs);
    update_filesetup_clean(chainsetup->outputs);
  }
  else {
    if (ctrl_repp->is_connected() == true) {
      update_filesetup(chainsetup->inputs);    
      update_filesetup(chainsetup->outputs);
    }
  }
  filesetupview->triggerUpdate();
}

void QEChainsetup::update_filesetup_clean (const vector<AUDIO_IO*>& flist) {
  for(aiod_sizet = 0; aiod_sizet < flist.size(); aiod_sizet++) {
    cs_namestring = QString(flist[aiod_sizet]->label().c_str());
    cs_chainstring = ""; 
    if (flist[aiod_sizet]->io_mode() == AUDIO_IO::io_read) {
      cs_modestring = "input"; 
      cs_chainstring = ctrl_repp->attached_chains_input(flist[aiod_sizet]).c_str();
    }
    else {
      cs_modestring = "output"; 
      cs_chainstring = ctrl_repp->attached_chains_output(flist[aiod_sizet]).c_str();
    }

    if (ctrl_repp->selected_chainsetup() != chainsetup->name()) {
	cs_posstring = "- / - ";
	cs_statusstring = "not open";
    }
    else {
      if (flist[aiod_sizet]->is_open()) {
	cs_posstring.sprintf("%.2f/%.2f",
			     flist[aiod_sizet]->position_in_seconds_exact(),
			     flist[aiod_sizet]->length_in_seconds_exact());
	cs_statusstring = "open";
      }
      else {
	cs_posstring = "- / - ";
	cs_statusstring = "not open";
      }
    }

    AUDIO_IO_DEVICE* p = dynamic_cast<AUDIO_IO_DEVICE*>(flist[aiod_sizet]);
    if (p != 0)
      cs_rtstring = "yes";
    else
      cs_rtstring = "no";
  
    cs_format.sprintf("%d/%d/%ld", 
		   flist[aiod_sizet]->bits(),
		   flist[aiod_sizet]->channels(),
		   flist[aiod_sizet]->samples_per_second());

    newitem = new QListViewItem(filesetupview,
				cs_namestring,
				cs_modestring,
				cs_format,
				cs_posstring,
				cs_rtstring,
				cs_statusstring,
				cs_chainstring);

  }
}

void QEChainsetup::update_filesetup (const vector<AUDIO_IO*>& flist) {
  for(aiod_sizet = 0; aiod_sizet < flist.size(); aiod_sizet++) {
    newitem = filesetupview->firstChild();
    while(newitem != 0) {
      if (newitem->text(0) ==
	  QString(flist[aiod_sizet]->label().c_str()))
	break;
      newitem = newitem->nextSibling();
    }
    if (newitem == 0) continue;

    if (ctrl_repp->selected_chainsetup() != chainsetup->name()) {
	cs_posstring = "- / - ";
	cs_statusstring = "not open";
    }
    else {
      if (flist[aiod_sizet]->is_open()) {
	cs_posstring.sprintf("%.2f/%.2f",
			     flist[aiod_sizet]->position_in_seconds_exact(),
			     flist[aiod_sizet]->length_in_seconds_exact());
	cs_statusstring = "open";
      }
      else {
	cs_posstring = "- / - ";
	cs_statusstring = "not open";
      }
    }
   
    newitem->setText(3, cs_posstring);
    newitem->setText(5, cs_statusstring);
  }
}

void QEChainsetup::init_chainsetuplist (void) {
  chainsetupview = new QListView(this, "chainsetupview");

  chainsetupview->addColumn("Chain");
  chainsetupview->addColumn("Chain operators");  
  chainsetupview->addColumn("Controllers");  
  chainsetupview->addColumn("Status");  

  chainsetupview->setAllColumnsShowFocus(true); 
  chainsetupview->setSorting(0);

  update_chainsetuplist_clean();
  // chainsetupview->show();
}

void QEChainsetup::update_chainsetuplist_clean(void) {
  chainsetupview->setColumnWidthMode(0, QListView::Maximum);
  chainsetupview->clear();

  vector<CHAIN*>::const_iterator p = chainsetup->chains.begin();
  while(p != chainsetup->chains.end()) {
    QString astring;
    if ((*p)->is_processing() == false)
      astring = "bypassed";

    if ((*p)->is_muted())
      astring = "muted";

    newitem = new QListViewItem(chainsetupview,
				(*p)->name().c_str(),
				kvu_numtostr((*p)->number_of_chain_operators()).c_str(),
				0,
				kvu_numtostr((*p)->number_of_controllers()).c_str(),
				astring);
    ++p;
  }

  chainsetupview->triggerUpdate();
}

void QEChainsetup::update_chainsetuplist () {
  if (!ctrl_repp->is_connected()) return;

  vector<CHAIN*>::const_iterator p = chainsetup->chains.begin();
  while(p != chainsetup->chains.end()) {
    newitem = chainsetupview->firstChild();
    while(newitem != 0) {
      if (newitem->text(0) ==
	  QString((*p)->name().c_str()))
	break;
      newitem = newitem->nextSibling();
    }
    if (newitem == 0) {
      ++p;
      continue;
    }

    QString astring;
    if ((*p)->is_processing() == false)
      astring = "bypassed";

    if ((*p)->is_muted())
      astring = "muted";

    newitem->setText(3, astring);

    ++p;
  }

  chainsetupview->triggerUpdate();
}

void QEChainsetup::button_add_file(void) { 
  QEIodevDialog* fdialog = new QEIodevDialog(chainsetup, this, "addfile");
  if (fdialog->exec() == QEIodevDialog::Accepted) {
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    ctrl_repp->set_default_audio_format(fdialog->result_audio_format());
    ctrl_repp->select_chains(fdialog->result_chains());

    if (fdialog->result_direction() == QEIodevDialog::input)
      ctrl_repp->add_audio_input(fdialog->result_filename());
    else
      ctrl_repp->add_audio_output(fdialog->result_filename());

    update_filesetuplist(true);
    update_chainsetuplist_clean();
  }
}

void QEChainsetup::button_remove_file(void) { 
  if (is_filesetup_highlighted()) {
    select_highlighted_filesetup();
    if (ctrl_repp->is_connected()) {
      ctrl_repp->disconnect_chainsetup();
    }
    if (ctrl_repp->get_audio_object() != 0) {
      ctrl_repp->remove_audio_object();
    }
    update_filesetuplist(true);
  }
  else
    QMessageBox::information(this, "qtecasound", "No file selected!",0);
}

void QEChainsetup::button_chainselect(void) { 
  QListViewItem* item = filesetupview->selectedItem();
  if (item == 0) item = filesetupview->currentItem();
  if (item != 0) {
    string name = string(item->text(0).latin1());

    QEChainselectDialog* cdialog = new QEChainselectDialog(chainsetup, this, "chainselect");
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    cdialog->set_chains(ctrl_repp->attached_chains(name));
    if (cdialog->exec() == QEChainselectDialog::Accepted) {
      ctrl_repp->select_chains(cdialog->result_chains());
      ctrl_repp->select_audio_object(name);
      ctrl_repp->attach_audio_object();
      update_filesetuplist(true);
    }
  }
  else
    QMessageBox::information(this, "qtecasound", "No file selected!",0);
}

void QEChainsetup::init_waveedit(void) {
  if (is_filesetup_highlighted()) {
    if (ctrl_repp->is_connected()) ctrl_repp->disconnect_chainsetup();
    select_highlighted_filesetup();
    ctrl_repp->wave_edit_audio_object();
    update_filesetuplist(true);
  }
  else
    QMessageBox::information(this, "qtecasound", "No file selected!",0);
}

void QEChainsetup::timerEvent( QTimerEvent * ) {
  update_filesetuplist(false);
  update_chainsetuplist();
}
