// ------------------------------------------------------------------------
// eca-qtinte.cpp: Main user interface widget for qtecasound
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

#include <fstream>
#include <vector>
#include <unistd.h>
#include <pthread.h>

#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qaccel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qtabwidget.h>
#include <qtimer.h>

#include <kvutils.h>
#include "qebuttonrow.h"

#include "eca-version.h"
#include "eca-control.h"
#include "eca-qtchainsetup.h"
#include "eca-qtmain.h"
#include "eca-qtrtposition.h"
#include "eca-qtdebug.h"
#include "eca-qtdebugtab.h"
#include "eca-qtinte.h"

QEInterface::QEInterface(ECA_CONTROL* control, QWidget *parent, const char *name)
  : QWidget(parent, name), ctrl_repp(control)
{
  setCaption("qtecasound - control panel");
  qApp->setMainWidget(this);
  init_layout();
}

void QEInterface::get_focus(void) {
  raise();
  setActiveWindow();
  setFocus();
}

void QEInterface::init_layout(void) {
  QBoxLayout* topLayout = new QVBoxLayout( this );
  QBoxLayout* qtbuttons = new QHBoxLayout();
  QBoxLayout* textinput = new QHBoxLayout();
  QBoxLayout* tabwidget = new QHBoxLayout();
  QBoxLayout* bottomrow = new QHBoxLayout();

  init_buttons();
  topLayout->addWidget(buttonrow_repp);
  topLayout->addLayout(qtbuttons,0);
  topLayout->addLayout(textinput,0);
  topLayout->addLayout(tabwidget,1);
  topLayout->addLayout(bottomrow,0);

  init_tabwidget(tabwidget);
  init_runtimebar(qtbuttons);
  init_textinput(textinput);
  init_bottomrow(bottomrow);
}

void QEInterface::init_bottomrow(QBoxLayout* bottomrow) {
  init_statusbar();
  bottomrow->addWidget(statusbar_repp, 2, 0);
}

void QEInterface::init_statusbar(void) {
  statusbar_repp = new QStatusBar(this, "qsbar");
  statusbar_repp->message(string("qtecasound v" + ecasound_library_version + " ready.").c_str()); 

  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(update_statusbar()));
  timer->start( 500, false);
}

void QEInterface::update_statusbar(void) {
  static string last_epstatus;
  string new_epstatus = ctrl_repp->engine_status();

  if (new_epstatus == last_epstatus) return;
  else last_epstatus = new_epstatus;

  statusbar_repp->message(string("qtecasound v" + ecasound_library_version
			    + " . (C) 1997-2000 Kai Vehmanen . engine status: [" +
			    last_epstatus + "]").c_str()); 
}

void QEInterface::init_tabwidget(QBoxLayout* layout) {
  QTabWidget* qtab = new QTabWidget(this, "qtab");

  QEDebugTab* debugtab = new QEDebugTab(this, this, "debugtab");
  qtab->addTab(debugtab, "Engine o&utput");

  session_repp = new QEChainsetup(ctrl_repp, this, "qechainsetup");
  qtab->addTab(session_repp, "Session setu&p");
  layout->addWidget(qtab,1,0);
}

void QEInterface::init_runtimebar(QBoxLayout* buttons) {
  rpos_repp = new QERuntimePosition(ctrl_repp->length_in_seconds_exact(),
			       this, "rpos");
  buttons->addWidget( rpos_repp, 10, 0 );
  connect( rpos_repp, SIGNAL(position_changed_from_widget(double)), this, SLOT(emsg_setpos(double)));

  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(update_runtimebar()));
  timer->start( 1000, false);
}

void QEInterface::update_runtimebar(void) {
  if (rpos_repp->does_widget_have_control() == false) {
    rpos_repp->position_in_seconds(ctrl_repp->position_in_seconds_exact());
    rpos_repp->length_in_seconds(ctrl_repp->length_in_seconds_exact());
  }
}

void QEInterface::init_buttons(void) {
  buttonrow_repp = new QEButtonRow(this, "buttonrow");
  buttonrow_repp->add_button(new QPushButton("(B)egin",buttonrow_repp), 
			ALT+Key_B,
			this, SLOT(emsg_rw_begin()));

  buttonrow_repp->add_button(new QPushButton("Rew (<)",buttonrow_repp), 
			Key_Less,
			this, SLOT(emsg_rewind()));

  buttonrow_repp->add_button(new QPushButton("S(t)art",buttonrow_repp), 
			ALT+Key_T,
			this, SLOT(emsg_start()));

  buttonrow_repp->add_button(new QPushButton("(S)top",buttonrow_repp), 
			ALT+Key_S,
			this, SLOT(emsg_stop()));

  buttonrow_repp->add_button(new QPushButton("Fw (>)",buttonrow_repp), 
			SHIFT+Key_Greater,
			this, SLOT(emsg_forward()));

  buttonrow_repp->add_button(new QPushButton("(Q)uit",buttonrow_repp), 
			ALT+Key_Q,
			qApp, SLOT(closeAllWindows()));


}

void QEInterface::init_textinput(QBoxLayout* textinput) {
  QLabel* tekstiinfo = new QLabel("qtecasound CL(I) ('h' for help): ", this, "tekstiinfo");
  textinput->addWidget( tekstiinfo, 5);
  
  tekstirivi_repp = new QLineEdit(this, "tekstirivi");
  textinput->addWidget( tekstirivi_repp, 10);

  QAccel* accel = new QAccel(this);
  accel->connectItem(accel->insertItem(ALT+Key_I), tekstirivi_repp, SLOT(setFocus()));
  
  connect(tekstirivi_repp, SIGNAL(returnPressed ()), this, SLOT(emsg_general()) );
  connect(this, SIGNAL(clear_textinput()), tekstirivi_repp, SLOT(clear()) );
}

void QEInterface::emsg_general(void) {
  string s (tekstirivi_repp->text());
  if (s == "q" || s == "quit") {
    qApp->closeAllWindows();
  }
  else {
    try {
      ctrl_repp->command(s);
    }
    catch(ECA_ERROR* e) {
      if (e->error_action() != ECA_ERROR::stop) {
	QMessageBox* mbox = new QMessageBox(this, "mbox");
	QString errormsg = "";
	errormsg += "Ecasound engine error: \"";
	errormsg += e->error_section().c_str();
	errormsg += "\"; ";
	errormsg += e->error_msg().c_str();
	mbox->information(this, "qtecasound", errormsg,0);
      }
      else throw;
    }
  }
  
  emit clear_textinput();
}

void QEInterface::emsg_quit(void) {  }
void QEInterface::emsg_stop(void) { ctrl_repp->command("s"); }
void QEInterface::emsg_start(void) { ctrl_repp->command("t"); }
void QEInterface::emsg_rw_begin(void) { ctrl_repp->command("setpos 0"); }
void QEInterface::emsg_forward(void) { ctrl_repp->command("fw 5"); }
void QEInterface::emsg_rewind(void) { ctrl_repp->command("rw 5"); }
void QEInterface::emsg_status(void) { ctrl_repp->command("st"); }
void QEInterface::emsg_csstatus(void) { ctrl_repp->command("cs-status"); }
void QEInterface::emsg_ctrlstatus(void) { ctrl_repp->command("ctrl-status"); }
void QEInterface::emsg_estatus(void) { ctrl_repp->command("es"); }
void QEInterface::emsg_fstatus(void) { ctrl_repp->command("fs"); }
void QEInterface::emsg_cstatus(void) { ctrl_repp->command("cs"); }
void QEInterface::emsg_exec(void) { ctrl_repp->start(); }
void QEInterface::emsg_setpos(double pos_seconds) {
  ctrl_repp->command("setpos " + kvu_numtostr(pos_seconds));
  rpos_repp->control_back_to_parent();
}

void QEInterface::not_implemented(void) {
  QMessageBox* mbox = new QMessageBox(this, "mbox");
  mbox->information(this, "qtecasound", "This feature is not implemented...",0);
}
