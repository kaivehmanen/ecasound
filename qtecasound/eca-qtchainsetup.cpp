// ------------------------------------------------------------------------
// eca-qtchainsetup: Qt widget representing a ECA_CHAINSETUP object and 
//                   its state.
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

#include <cmath>
#include <iostream>
#include <string>

#include <qapplication.h>
#include <qaccel.h>
#include <qwidget.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include <kvutils.h>

#include "qstringdialog.h"

#include "eca-chain.h"
#include "eca-controller.h"
#include "eca-chainsetup.h"

#include "eca-qtinte.h"
#include "eca-qtchain.h"
#include "eca-qtwaveform.h"
// #include "qlistview-dumb.h"
#include "eca-qtchainsetup.h"
#include "eca-qtiodevdialog.h"
#include "eca-qtchainselectdialog.h"

QEChainsetup::QEChainsetup (ECA_CONTROLLER* econtrol, 
			    const ECA_CHAINSETUP* csetup, 
			    QWidget *parent,
			    const char *name) 
  : ctrl(econtrol), chainsetup(csetup)
{
  setMinimumSize( 600, 0 );
  //  setMaximumSize( 1024, 768 );

  startTimer(1000);

  current_dir = "";

  string caption = "qtecasound - chainsetup: " + csetup->name();
  setCaption(caption.c_str());

  topLayout = new QVBoxLayout( this );
  gen_buttons = new QHBoxLayout();
  file_buttons = new QHBoxLayout();
  chain_buttons = new QHBoxLayout();

  init_filesetuplist();
  init_chainsetuplist();

  init_shortcuts();

  init_gen_buttons();
  init_file_buttons();
  init_chain_buttons();

  update_layout();

  QObject::connect( chainsetupview,
		    SIGNAL(doubleClicked(QListViewItem*)), 
		    this, SLOT(init_chainview(QListViewItem*)));
  QObject::connect( chainsetupview,
		    SIGNAL(returnPressed(QListViewItem*)), 
		    this, SLOT(init_chainview(QListViewItem*)));
}

void QEChainsetup::close_session(void) {
  emit close_waveforms();
  //  cerr << "Here!";
  //  close();
}

void QEChainsetup::not_implemented(void) {
  QMessageBox* mbox = new QMessageBox(this, "mbox");
  mbox->information(this, "qtecasound", "This feature is not implemented...",0);
}

void QEChainsetup::init_shortcuts(void) {
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
		 SLOT(button_add_file()));
  a->connectItem(a->insertItem(SHIFT+Key_A), this,
		 SLOT(button_add_file()));
  a->connectItem(a->insertItem(CTRL+Key_A), this,
		 SLOT(button_add_file()));

  a->connectItem(a->insertItem(Key_B), this,
		 SLOT(button_chain_bypass()));
  a->connectItem(a->insertItem(SHIFT+Key_B), this,
		 SLOT(button_chain_bypass()));
  a->connectItem(a->insertItem(CTRL+Key_B), this,
		 SLOT(button_chain_bypass()));

  a->connectItem(a->insertItem(Key_C), chainsetupview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(SHIFT+Key_C), chainsetupview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(CTRL+Key_C), chainsetupview,
		 SLOT(setFocus()));

  a->connectItem(a->insertItem(Key_D), this,
		 SLOT(button_remove_chain()));
  a->connectItem(a->insertItem(SHIFT+Key_D), this,
		 SLOT(button_remove_chain()));
  a->connectItem(a->insertItem(CTRL+Key_D), this,
		 SLOT(button_remove_chain()));

  a->connectItem(a->insertItem(Key_F), filesetupview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(SHIFT+Key_F), filesetupview,
		 SLOT(setFocus()));
  a->connectItem(a->insertItem(CTRL+Key_F), filesetupview,
		 SLOT(setFocus()));

  a->connectItem(a->insertItem(Key_N), this,
		 SLOT(button_add_chain()));
  a->connectItem(a->insertItem(SHIFT+Key_N), this,
		 SLOT(button_add_chain()));
  a->connectItem(a->insertItem(CTRL+Key_N), this,
		 SLOT(button_add_chain()));

  a->connectItem(a->insertItem(Key_M), this,
		 SLOT(button_chain_muting()));
  a->connectItem(a->insertItem(SHIFT+Key_M), this,
		 SLOT(button_chain_muting()));
  a->connectItem(a->insertItem(CTRL+Key_M), this,
		 SLOT(button_chain_muting()));

  a->connectItem(a->insertItem(Key_O), this,
		 SLOT(init_chainview()));
  a->connectItem(a->insertItem(SHIFT+Key_O), this,
		 SLOT(init_chainview()));
  a->connectItem(a->insertItem(CTRL+Key_O), this,
		 SLOT(init_chainview()));

  a->connectItem(a->insertItem(Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(SHIFT+Key_Q), this,
		 SLOT(close()));
  a->connectItem(a->insertItem(CTRL+Key_Q), this,
		 SLOT(close()));

  a->connectItem(a->insertItem(Key_R), this,
		 SLOT(button_remove_file()));
  a->connectItem(a->insertItem(SHIFT+Key_R), this,
		 SLOT(button_remove_file()));
  a->connectItem(a->insertItem(CTRL+Key_R), this,
		 SLOT(button_remove_file()));

  a->connectItem(a->insertItem(Key_S), this,
		 SLOT(button_chainselect()));
  a->connectItem(a->insertItem(SHIFT+Key_S), this,
		 SLOT(button_chainselect()));
  a->connectItem(a->insertItem(CTRL+Key_S), this,
		 SLOT(button_chainselect()));

  a->connectItem(a->insertItem(Key_V), this,
		 SLOT(init_waveform()));
  a->connectItem(a->insertItem(SHIFT+Key_V), this,
		 SLOT(init_waveform()));
  a->connectItem(a->insertItem(CTRL+Key_V), this,
		 SLOT(init_waveform()));

  a->connectItem(a->insertItem(Key_W), this,
		 SLOT(init_waveedit()));
  a->connectItem(a->insertItem(SHIFT+Key_W), this,
		 SLOT(init_waveedit()));
  a->connectItem(a->insertItem(CTRL+Key_W), this,
		 SLOT(init_waveedit()));
}

void QEChainsetup::init_chainview(void) {
  QListViewItem* item = chainsetupview->selectedItem();
  if (item == 0) item = chainsetupview->currentItem();
  init_chainview(chainsetupview->selectedItem());
}

void QEChainsetup::init_chainview(QListViewItem* item) {
  if (item != 0) {
    const CHAIN* chain =
      chainsetup->get_chain_with_name(item->text(0).latin1());
    if (chain != 0) {
      QEChain* csetup = new QEChain(ctrl, chain);
      csetup->show();
    }
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::init_gen_buttons(void) {
  QFont butfont ("Helvetica", 12, QFont::Normal);

  QPushButton* cpanelbut = new QPushButton( "(!) Control panel", this, "cpanelbut" );
  cpanelbut->setFont(butfont);
  gen_buttons->addWidget( cpanelbut, 1, 0);

  QObject::connect( cpanelbut, SIGNAL(clicked()), 
		 reinterpret_cast<QEInterface*>(qApp->mainWidget()), 
		 SLOT(get_focus()));

  QPushButton* ffocus = new QPushButton( "Focus to (f)iles", this, "ffocus" );
  ffocus->setFont(butfont);
  gen_buttons->addWidget( ffocus, 1, 0);

  QPushButton* cfocus = new QPushButton( "Focus to (c)hains", this, "cfocus" );
  cfocus->setFont(butfont);
  gen_buttons->addWidget( cfocus, 1, 0);

  QPushButton* quit = new QPushButton( "(Q)uit", this, "quit" );
  quit->setFont(butfont);
  gen_buttons->addWidget( quit, 2, 0);

  QObject::connect( cfocus, SIGNAL(clicked()), chainsetupview,
		    SLOT(setFocus()));
  QObject::connect( ffocus, SIGNAL(clicked()), filesetupview,
		    SLOT(setFocus()));
  QObject::connect( quit, SIGNAL(clicked()), this, SLOT(close()));
  //  connect(quit, SIGNAL(clicked()), this, SLOT(emsg_quit()) );
}

void QEChainsetup::update_layout(void) {
  // if (topLayout != 0) delete topLayout;
  // topLayout = new QVBoxLayout( this );

  topLayout->addLayout(gen_buttons, 0);

  topLayout->addLayout(file_buttons, 0);
  topLayout->addWidget(filesetupview, 0, 0);

  topLayout->addLayout(chain_buttons, 0);
  topLayout->addWidget(chainsetupview, 0, 0);
}

void QEChainsetup::init_file_buttons(void) { 
  QFont butfont ("Helvetica", 12, QFont::Normal);

  QPushButton* add = new QPushButton( "(A)dd", this, "add" );
  add->setFont(butfont);
  file_buttons->addWidget( add, 1, 0);

  QPushButton* remove = new QPushButton( "(R)emove from setup", this, "remove" );
  remove->setFont(butfont);
  file_buttons->addWidget( remove, 1, 0);

  QPushButton* change = new QPushButton( "Attach to chain(s)", this, "change" );
  change->setFont(butfont);
  file_buttons->addWidget( change, 1, 0);

  QPushButton* wform = new QPushButton( "Wave form (v)iew", this, "wform" );
  wform->setFont(butfont);
  file_buttons->addWidget( wform, 2, 0);

  QPushButton* wedit = new QPushButton( "(W)ave form edit", this, "wedit" );
  wedit->setFont(butfont);
  file_buttons->addWidget( wedit, 2, 0);

  QObject::connect( add, SIGNAL(clicked()), this, SLOT(button_add_file()));
  QObject::connect( remove, SIGNAL(clicked()), this, SLOT(button_remove_file()));
  QObject::connect( change, SIGNAL(clicked()), this, SLOT(button_chainselect()));
  QObject::connect( wform, SIGNAL(clicked()), this, SLOT(init_waveform()));
  QObject::connect( wedit, SIGNAL(clicked()), this, SLOT(init_waveedit()));
}

void QEChainsetup::init_chain_buttons(void) { 
  QFont butfont ("Helvetica", 12, QFont::Normal);

  QPushButton* newb = new QPushButton( "(N)ew chain", this, "newb" );
  newb->setFont(butfont);
  chain_buttons->addWidget( newb, 1, 0);

  QPushButton* delchain = new QPushButton( "(D)elete chain", this, "delchain" );
  delchain->setFont(butfont);
  chain_buttons->addWidget( delchain, 1, 0);

  QPushButton* mute = new QPushButton( "(M)uting", this, "mute" );
  mute->setFont(butfont);
  chain_buttons->addWidget( mute, 1, 0);

  QPushButton* bp_on = new QPushButton( "(B)ypass", this, "bp_on" );
  bp_on->setFont(butfont);
  chain_buttons->addWidget( bp_on, 1, 0);

  QPushButton* open = new QPushButton( "(O)pen", this, "open" );
  open->setFont(butfont);
  chain_buttons->addWidget( open, 2, 0);

  QObject::connect( bp_on,  SIGNAL(clicked()), this, SLOT(button_chain_bypass()));
  QObject::connect( mute, SIGNAL(clicked()), this, SLOT(button_chain_muting()));
  QObject::connect( delchain, SIGNAL(clicked()), this, SLOT(button_remove_chain()));
  QObject::connect( open, SIGNAL(clicked()), this, SLOT(init_chainview()));
  QObject::connect( newb, SIGNAL(clicked()), this, SLOT(button_add_chain()));
}
 
void QEChainsetup::button_add_chain(void) { 
  QStringDialog* sdialog = new QStringDialog("Chain name: ", this);
  if (sdialog->exec() == QStringDialog::Accepted) {
    ctrl->select_chainsetup(chainsetup->name());
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    ctrl->add_chain(sdialog->resultString().latin1());
    update_chainsetuplist();
    update_chainsetuplist_clean();
  }
}

void QEChainsetup::button_remove_chain(void) { 
  if (is_chain_highlighted()) {
    select_highlighted_chain();
    ctrl->select_chainsetup(chainsetup->name());
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    ctrl->remove_chains();
    update_chainsetuplist_clean();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::button_chain_muting(void) {
  if (is_chain_highlighted()) {
    select_highlighted_chain();
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    ctrl->toggle_chain_muting();
    update_chainsetuplist();
  }
  else
    QMessageBox::information(this, "qtecasound", "No chain selected!",0);
}

void QEChainsetup::button_chain_bypass(void) {
  if (is_chain_highlighted()) {
    select_highlighted_chain();
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    ctrl->toggle_chain_bypass();
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
    ctrl->select_chainsetup(chainsetup->name());
    ctrl->select_audio_object(item->text(0).latin1());
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
    ctrl->select_chainsetup(chainsetup->name());
    vector<string> t (1, string(item->text(0).latin1()));
    ctrl->select_chains(t);
  }
}

void QEChainsetup::init_filesetuplist (void) {

  filesetupview = new QListView(this, "filesetupview");

  filesetupview->setMinimumSize( 600, 100 );

  filesetupview->addColumn("File");
  filesetupview->addColumn("Mode");
  filesetupview->addColumn("Bits/Ch/Rate");
  filesetupview->addColumn("Position/Length");
  filesetupview->addColumn("Realtime");
  filesetupview->addColumn("Status");
  filesetupview->addColumn("Chains");

  filesetupview->setAllColumnsShowFocus(true); 
  filesetupview->setSorting(6);

  update_filesetuplist();

  filesetupview->setGeometry(0, 0, width(), 100);

  filesetupview->show();
}

void QEChainsetup::update_filesetuplist (bool clean) {
  QListViewItem* selected = filesetupview->selectedItem();
  QString selname = ""; 

  int pixelsleft = width() - 4;
  for(int n = 1; n < 7; n++) {
    pixelsleft -= filesetupview->columnWidth(n);
  }  

  if (pixelsleft > 0) {
    filesetupview->setColumnWidthMode(0, QListView::Maximum);
    filesetupview->setColumnWidth(0, pixelsleft);
  }

  if (selected != 0) selname = selected->text(0);

  if (clean) {
    filesetupview->clear();
    update_filesetup_clean(chainsetup->inputs, selname);    
    update_filesetup_clean(chainsetup->outputs, selname);
  }
  else {
    if (!ctrl->is_connected()) return;
    update_filesetup(chainsetup->inputs, selname);    
    update_filesetup(chainsetup->outputs, selname);
  }
  filesetupview->triggerUpdate();
}

void QEChainsetup::update_filesetup_clean (const vector<AUDIO_IO*>&
					   flist, const QString& selname) {
  aiod_sizet = 0;

  while(aiod_sizet < flist.size()) {
    cs_namestring = QString(flist[aiod_sizet]->label().c_str());

    cs_chainstring = ""; 
    if (flist[aiod_sizet]->io_mode() == si_read) {
      cs_modestring = "input"; 
      cs_chainstring = ctrl->connected_chains_input(flist[aiod_sizet]).c_str();
    }
    else {
      cs_modestring = "output"; 
      cs_chainstring = ctrl->connected_chains_output(flist[aiod_sizet]).c_str();
    }

    if (ctrl->selected_chainsetup() != chainsetup->name()) {
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

    if (flist[aiod_sizet]->is_realtime()) 
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

    if (newitem->text(0) == selname) filesetupview->setSelected(newitem, true);
    ++aiod_sizet;
  }
}

void QEChainsetup::update_filesetup (const vector<AUDIO_IO*>&
  				     flist, const QString& selname) {

  for(aiod_sizet = 0; aiod_sizet < flist.size(); aiod_sizet++) {
    newitem = filesetupview->firstChild();
    while(newitem != 0) {
      if (newitem->text(0) ==
	  QString(flist[aiod_sizet]->label().c_str()))
	break;
      newitem = newitem->nextSibling();
    }
    if (newitem == 0) continue;

    if (ctrl->selected_chainsetup() != chainsetup->name()) {
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

  chainsetupview->setMinimumSize( 600, 100 );

  chainsetupview->addColumn("Chain");
  chainsetupview->addColumn("Chain operators");  
  chainsetupview->addColumn("Controllers");  
  chainsetupview->addColumn("Status");  

  chainsetupview->setAllColumnsShowFocus(true); 
  chainsetupview->setSorting(0);

  update_chainsetuplist_clean();

  chainsetupview->setGeometry(0, 0, width(), 100);
  chainsetupview->show();
}

void QEChainsetup::update_chainsetuplist_clean(void) {
  QListViewItem* selected = chainsetupview->selectedItem();
  QString selname = ""; 

  int pixelsleft = width() - 4;
  for(int n = 1; n < 4; n++) 
    pixelsleft -= chainsetupview->columnWidth(n);
  
  if (pixelsleft > 0) {
    chainsetupview->setColumnWidthMode(0, QListView::Maximum);
    chainsetupview->setColumnWidth(0, pixelsleft);
  }

  if (selected != 0) selname = selected->text(0);

  chainsetupview->clear();

  vector<CHAIN*>::const_iterator p = chainsetup->chains.begin();
  while(p != chainsetup->chains.end()) {
    //    cerr << "Adding a new one!\n";

    QString astring;
    if ((*p)->is_processing() == false)
      astring = "bypassed";

    if ((*p)->is_muted())
      astring = "muted";

    newitem = new QListViewItem(chainsetupview,
				(*p)->name().c_str(),
				kvu_numtostr((*p)->number_of_chain_operators()).c_str(),
				0,
				//				kvu_numtostr((*p)->gcontrollers.size()).c_str(),
				astring);
    // *** NOTICE!!! see above comment
    if (newitem->text(0) == selname) chainsetupview->setSelected(newitem, true);
    ++p;
  }

  chainsetupview->triggerUpdate();
}

void QEChainsetup::update_chainsetuplist () {
  if (!ctrl->is_connected()) return;

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
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    string t = "s16_le";
    if (fdialog->result_bits() == 8) t = "u8";
    ctrl->set_default_audio_format(t,
				   fdialog->result_channels(), 
				   (long int)fdialog->result_srate());
    ctrl->select_chains(fdialog->result_chains());

    if (fdialog->result_direction() == QEIodevDialog::input)
      ctrl->add_audio_input(fdialog->result_filename());
    else
      ctrl->add_audio_output(fdialog->result_filename());

    update_filesetuplist();
    update_chainsetuplist_clean();
  }
}

void QEChainsetup::button_remove_file(void) { 
  if (is_filesetup_highlighted()) {
    select_highlighted_filesetup();
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    ctrl->remove_audio_object();
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
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    cdialog->set_chains(ctrl->connected_chains(name));
    
    if (cdialog->exec() == QEChainselectDialog::Accepted) {
      ctrl->select_chains(cdialog->result_chains());
      ctrl->select_audio_object(name);
      ctrl->attach_audio_object();
      update_filesetuplist();
    }
  }
  else
    QMessageBox::information(this, "qtecasound", "No file selected!",0);
}

void QEChainsetup::init_waveform(void) {
  if (ctrl->is_engine_started()) ctrl->stop();
  QEWaveForm* wform = 0;
  string name = "";

  QListViewItem* item = filesetupview->selectedItem();
  if (item == 0) item = filesetupview->currentItem();
  if (item != 0) {
    name = string(item->text(0).latin1());
    AUDIO_IO* dev = 0;
    ctrl->select_chainsetup(chainsetup->name());
    ctrl->select_audio_object(name);
    dev = ctrl->get_audio_object();
    if (dev != 0) {
      wform = new QEWaveForm(static_cast<AUDIO_IO_FILE*>(dev));
      if (wform != 0) {
	QObject::connect(this, SIGNAL(close_waveforms()), 
			  wform, SLOT(close()));

	wform->updateWaveData();
	wform->show();  
      }
    }
  }
  else
    QMessageBox::information(this, "qtecasound", "No file selected!",0);
}

void QEChainsetup::init_waveedit(void) {
  if (is_filesetup_highlighted()) {
    if (ctrl->is_connected()) ctrl->disconnect_chainsetup();
    select_highlighted_filesetup();
    ctrl->wave_edit_audio_object();
    update_filesetuplist(true);
  }
  else
    QMessageBox::information(this, "qtecasound", "No file selected!",0);
}

void QEChainsetup::timerEvent( QTimerEvent * ) {
  update_filesetuplist(false);
  update_chainsetuplist();
}



