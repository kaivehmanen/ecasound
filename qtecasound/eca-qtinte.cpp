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
#include <qtimer.h>

#include <kvutils.h>

#include "eca-version.h"
// #include "eca-main.h"
#include "eca-session.h"
#include "eca-controller.h"
#include "eca-qtsession.h"
#include "eca-qtmain.h"
#include "eca-qtrtposition.h"
#include "eca-qtdebug.h"
#include "eca-qtinte.h"
#include "qebuttonrow.h"

QEInterface::QEInterface(ECA_CONTROLLER* control, const ECA_SESSION* session, QWidget *parent, const char *name )
        : QWidget( parent, name ), ctrl(control), ecaparams(session)
{
  setCaption("qtecasound - control panel (*)");
  qApp->setMainWidget(this);

  child_setup = 0;

  QBoxLayout* topLayout = new QVBoxLayout( this );
  QBoxLayout* qtbuttons = new QHBoxLayout();
  QBoxLayout* textinput = new QHBoxLayout();
  QBoxLayout* debugout = new QHBoxLayout();
  QBoxLayout* bottomrow = new QHBoxLayout();

  init_buttons();
  topLayout->addWidget(buttonrow);
  topLayout->addLayout(qtbuttons,0);
  topLayout->addLayout(textinput,0);
  topLayout->addLayout(debugout,1);
  topLayout->addLayout(bottomrow,0);

  init_runtimebar(qtbuttons);
  init_textinput(textinput);
  init_debugout(debugout);
  init_bottomrow(bottomrow);
}

void QEInterface::focusInEvent ( QFocusEvent * ) {
  repaint( visibleRect() );
  if ( testWState(WState_AutoMask) )
    updateMask();
  setCaption("qtecasound - control panel (*)");
}

void QEInterface::focusOutEvent ( QFocusEvent * ) {
  setCaption("qtecasound - control panel");
}

void QEInterface::get_focus(void) {
  raise();
  setActiveWindow();
  setFocus();
  setCaption("qtecasound - control panel (*)");
}

void QEInterface::init_bottomrow(QBoxLayout* bottomrow) {
  init_statusbar();
  bottomrow->addWidget(statusbar, 2, 0);
}

void QEInterface::init_statusbar(void) {
  statusbar = new QStatusBar(this, "qsbar");

  statusbar->message(string("qtecasound " + ecasound_version + " ready.").c_str()); 

  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(update_statusbar()));
  timer->start( 500, false);
}

void QEInterface::update_statusbar(void) {
  static string last_epstatus;
  string new_epstatus = ctrl->engine_status();

  if (new_epstatus == last_epstatus) return;
  else last_epstatus = new_epstatus;

  statusbar->message(string("qtecasound " + ecasound_version
			    + " . (C) 1997-2000 Kai Vehmanen . engine status: [" +
			    last_epstatus + "]").c_str()); 
}

void QEInterface::init_sessionsetup(void) {
  if (child_setup != 0) {
    child_setup->close(true);
  }
  child_setup = new QESession(ctrl, ecaparams);
  child_setup->show();

  setCaption("qtecasound - control panel");
  connect(child_setup, SIGNAL(session_closed()), this,
	  SLOT(sessionsetup_closed()));
}

void QEInterface::sessionsetup_closed(void) {
  child_setup = 0;
}

void QEInterface::init_debugout(QBoxLayout* debugout) {
  QEDebug* qdebug = new QEDebug(this, "qdebug");
  debugout->addWidget( qdebug, 1, 0);
}

void QEInterface::init_runtimebar(QBoxLayout* buttons) {
  QEButtonRow* buttonrow2 = new QEButtonRow(this, "buttonrow2");
  buttonrow2->add_button(new QPushButton("Session setu(p)",buttonrow2), 
			 ALT+Key_P,
			 this, SLOT(init_sessionsetup()));
  buttons->addWidget( buttonrow2, 5, 0 );

  rpos = new QERuntimePosition(ctrl->length_in_seconds_exact(),
			       this, "rpos");
  buttons->addWidget( rpos, 10, 0 );
  connect( rpos, SIGNAL(position_changed_from_widget(double)), this, SLOT(emsg_setpos(double)));

  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(update_runtimebar()));
  timer->start( 1000, false);
}

void QEInterface::update_runtimebar(void) {
  if (rpos->does_widget_have_control() == false) {
    rpos->position_in_seconds(ctrl->position_in_seconds_exact());
    rpos->length_in_seconds(ctrl->length_in_seconds_exact());
  }
}

void QEInterface::init_buttons(void) {
  buttonrow = new QEButtonRow(this, "buttonrow");
  buttonrow->add_button(new QPushButton("(B)egin",buttonrow), 
			ALT+Key_B,
			this, SLOT(emsg_rw_begin()));

  buttonrow->add_button(new QPushButton("(R)ew",buttonrow), 
			ALT+Key_R,
			this, SLOT(emsg_rewind()));

  buttonrow->add_button(new QPushButton("S(t)art",buttonrow), 
			ALT+Key_T,
			this, SLOT(emsg_start()));

  buttonrow->add_button(new QPushButton("(S)top",buttonrow), 
			ALT+Key_S,
			this, SLOT(emsg_stop()));

  buttonrow->add_button(new QPushButton("(F)w",buttonrow), 
			ALT+Key_F,
			this, SLOT(emsg_forward()));

  buttonrow->add_button(new QPushButton("Stat(u)s",buttonrow), 
			ALT+Key_U,
			this, SLOT(emsg_status()));

  buttonrow->add_button(new QPushButton("C(h)ainsetups",buttonrow), 
			ALT+Key_H,
			this, SLOT(emsg_csstatus()));

  buttonrow->add_button(new QPushButton("F(X)",buttonrow), 
			ALT+Key_X,
			this, SLOT(emsg_estatus()));

  buttonrow->add_button(new QPushButton("(C)trls",buttonrow), 
			ALT+Key_C,
			this, SLOT(emsg_ctrlstatus()));

  buttonrow->add_button(new QPushButton("Ch(a)ins",buttonrow), 
			ALT+Key_A,
			this, SLOT(emsg_cstatus()));

  buttonrow->add_button(new QPushButton("Fi(l)es",buttonrow), 
			ALT+Key_L,
			this, SLOT(emsg_fstatus()));

  buttonrow->add_button(new QPushButton("(Q)uit",buttonrow), 
			ALT+Key_Q,
			qApp, SLOT(closeAllWindows()));


}

void QEInterface::init_textinput(QBoxLayout* textinput) {
  QLabel* tekstiinfo = new QLabel("qtecasound CL(I) ('h' for help): ", this, "tekstiinfo");
  textinput->addWidget( tekstiinfo, 5);
  
  tekstirivi = new QLineEdit(this, "tekstirivi");
  textinput->addWidget( tekstirivi, 10);

  QAccel* accel = new QAccel(this);
  accel->connectItem(accel->insertItem(ALT+Key_I), tekstirivi, SLOT(setFocus()));
  
  connect(tekstirivi, SIGNAL(returnPressed ()), this, SLOT(emsg_general()) );
  connect(this, SIGNAL(clear_textinput()), tekstirivi, SLOT(clear()) );
}

void QEInterface::emsg_general(void) {
  string s (tekstirivi->text());
  if (s == "q" || s == "quit") {
    qApp->closeAllWindows();
  }
  else 
    ctrl->command(s);

  emit clear_textinput();
}

void QEInterface::emsg_quit(void) {  }
void QEInterface::emsg_stop(void) { ctrl->command("s"); }
void QEInterface::emsg_start(void) { ctrl->command("t"); }
void QEInterface::emsg_rw_begin(void) { ctrl->command("setpos 0"); }
void QEInterface::emsg_forward(void) { ctrl->command("fw 5"); }
void QEInterface::emsg_rewind(void) { ctrl->command("rw 5"); }
void QEInterface::emsg_status(void) { ctrl->command("st"); }
void QEInterface::emsg_csstatus(void) { ctrl->command("cs-status"); }
void QEInterface::emsg_ctrlstatus(void) { ctrl->command("ctrl-status"); }
void QEInterface::emsg_estatus(void) { ctrl->command("es"); }
void QEInterface::emsg_fstatus(void) { ctrl->command("fs"); }
void QEInterface::emsg_cstatus(void) { ctrl->command("cs"); }
void QEInterface::emsg_exec(void) { ctrl->start(); }

void QEInterface::emsg_setpos(double pos_seconds) {
  ctrl->command("setpos " + kvu_numtostr(pos_seconds));
  rpos->control_back_to_parent();
}

void QEInterface::not_implemented(void) {
  QMessageBox* mbox = new QMessageBox(this, "mbox");
  mbox->information(this, "qtecasound", "This feature is not implemented...",0);
}
