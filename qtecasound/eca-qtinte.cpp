// ------------------------------------------------------------------------
// eca-qtinte.cpp: Main user interface widget for qtecasound
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
#include "eca-qtwaveform.h"
#include "eca-qtsignallevel.h"
#include "eca-qtrtposition.h"
#include "eca-qtdebug.h"
#include "eca-qtinte.h"

QEInterface::QEInterface(ECA_CONTROLLER* control, const ECA_SESSION* session, QWidget *parent, const char *name )
        : QWidget( parent, name ), ctrl(control), ecaparams(session)
{
  //  if (ctrl->is_engine_ready()) ctrl->start_engine(true);
  
  //  setMinimumSize( 200, 300 );
  //  setMaximumSize( 800, 600 );

  sessionsetup_opened = false;
  //  startTimer(50);

  //  string caption = "qtecasound - control panel \t" + ecasound_version + " - (C) 1997-99 Kai Vehmanen";
  setCaption("qtecasound - control panel (*)");
  qApp->setMainWidget(this);

  QBoxLayout* topLayout = new QVBoxLayout( this );

  QBoxLayout* buttons = new QHBoxLayout();
  QBoxLayout* qtbuttons = new QHBoxLayout();
  QBoxLayout* textinput = new QHBoxLayout();
  QBoxLayout* debugout = new QHBoxLayout();
  QBoxLayout* bottomrow = new QHBoxLayout();

  //  QMenuBar* menubar = new QMenuBar(this, "qmbar");
  //  topLayout->addWidget(menubar, 2);

  topLayout->addLayout(buttons,0);
  topLayout->addLayout(qtbuttons,0);
  topLayout->addLayout(textinput,0);
  topLayout->addLayout(debugout,1);
  topLayout->addLayout(bottomrow,0);

  init_qtbuttons(qtbuttons);
  init_buttons(buttons);
  init_textinput(textinput);
  init_debugout(debugout);
  init_bottomrow(bottomrow);
  init_shortcuts();

  //  connect( qApp, SIGNAL( aboutToQuit() ), this, SLOT( emsg_quit() ) );
}

void QEInterface::focusInEvent ( QFocusEvent * ) {
  repaint( visibleRect() );
  if ( testWState(WState_AutoMask) )
    updateMask();
  //  setMicroFocusHint(width()/2, 0, 1, height(), FALSE);
  setCaption("qtecasound - control panel (*)");
  //  setEnabled(true);
}

void QEInterface::focusOutEvent ( QFocusEvent * ) {
  setCaption("qtecasound - control panel");
  //  show();
  //  repaint( visibleRect() );
  //  if ( testWState(WState_AutoMask) ) updateMask();
}

void QEInterface::get_focus(void) {
  raise();
  setActiveWindow();
  setFocus();
  //  show();
  setCaption("qtecasound - control panel (*)");
  //  QApplication::sendEvent(reinterpret_cast<QObject*>(this), 
  //		  new QEvent(QEvent::MouseButtonPress));
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
			    + " . (C) 1997-99 Kai Vehmanen . engine status: [" +
			    last_epstatus + "]").c_str()); 
}

void QEInterface::init_sessionsetup(void) {
  if (sessionsetup_opened) {
    ssetup->raise();
    ssetup->setActiveWindow();
    //    QMessageBox* mbox = new QMessageBox(this, "mbox");
    //    mbox->information(this, "qtecasound", "Session setup window already open!",0);
  }
  else {
    ssetup = new QESession(ctrl, ecaparams);
    ssetup->show();
  }

  setCaption("qtecasound - control panel");
  sessionsetup_opened = true;  
  connect(ssetup, SIGNAL(session_closed()), this,
	  SLOT(sessionsetup_closed()));
}

void QEInterface::sessionsetup_closed(void) {
  sessionsetup_opened = false;
}

void QEInterface::init_signallevel(void) {
  //  QESignalLevel* qsignallevel = new QESignalLevel(&(ecaparams->outslots));
  //  connect(this, SIGNAL(update_signallevel(int)), qsignallevel, SLOT(update(int)));
  //  connect(this, SIGNAL(mute_signallevels()), qsignallevel, SLOT(mute()));
  //  qsignallevel->show();
}

void QEInterface::init_debugout(QBoxLayout* debugout) {
  QEDebug* qdebug = new QEDebug(this, "qdebug");
  debugout->addWidget( qdebug, 1, 0);
}

void QEInterface::init_qtbuttons(QBoxLayout* buttons) {
  QFont butfont ("Helvetica", 12, QFont::Normal);

//    QPushButton* omonitor = new QPushButton( "Output monitor", this, "omonitor" );
//    omonitor->setFont(butfont);
//    buttons->addWidget( omonitor, 1, 0 );
//    connect(omonitor, SIGNAL(clicked()), this, SLOT(init_signallevel())
//  	  );
  QPushButton* session = new QPushButton( "Session setu(p)", this, "session" );
  session->setFont(butfont);
  buttons->addWidget( session, 5, 0 );

  connect(session, SIGNAL(clicked()), this, SLOT(init_sessionsetup()) );

  //  connect(omonitor, SIGNAL(clicked()), this, SLOT(init_signallevel()) );

  //  QPushButton* waveform = new QPushButton( "Waveform View", this, "waveform" );
  //  waveform->setFont(butfont);
  //  buttons->addWidget( waveform, 2, 0 );
  //  connect(waveform, SIGNAL(clicked()), this, SLOT(init_waveform())
  //  );
  rpos = new QERuntimePosition(ctrl->length_in_seconds_exact(),
			       this, "rpos");
  buttons->addWidget( rpos, 10, 0 );
  connect( rpos, SIGNAL(position_changed_from_widget(double)), this, SLOT(emsg_setpos(double)));

  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(update_qtbuttons()));
  timer->start( 1000, false);

  //  connect(waveform, SIGNAL(clicked()), this, SLOT(init_waveform())

}

void QEInterface::update_qtbuttons(void) {
  if (rpos->does_widget_have_control() == false) {
    rpos->position_in_seconds(ctrl->position_in_seconds_exact());
    rpos->length_in_seconds(ctrl->length_in_seconds_exact());
  }
}

void QEInterface::init_buttons(QBoxLayout* buttons) {
  QFont butfont ("Helvetica", 12, QFont::Normal);

  QPushButton* rw_begin = new QPushButton( "(B)egin", this, "rw_begin" );
  rw_begin->setFont(butfont);
  buttons->addWidget( rw_begin, 1, 1 );

  QPushButton* rewind = new QPushButton( "(R)ew", this, "rewind" );
  rewind->setFont(butfont);
  buttons->addWidget( rewind, 1, 1 );

  QPushButton* start = new QPushButton( "S(t)art", this, "start" );
  start->setFont(butfont);
  buttons->addWidget( start, 3, 1 );

  QPushButton* stop = new QPushButton( "(S)top", this, "stop" );
  stop->setFont(butfont);
  buttons->addWidget( stop, 3, 1 );

  QPushButton* forward = new QPushButton( "(F)w", this, "forward" );
  forward->setFont(butfont);
  buttons->addWidget( forward, 1, 1 );

  QPushButton* status = new QPushButton( "Stat(u)s", this, "status" );
  status->setFont(butfont);
  buttons->addWidget( status, 2, 1 );

  QPushButton* estatus = new QPushButton( "F(X)", this, "estatus" );
  estatus->setFont(butfont);
  buttons->addWidget( estatus, 2, 1 );

  QPushButton* cstatus = new QPushButton( "Ch(a)ins", this, "cstatus" );
  cstatus->setFont(butfont);
  buttons->addWidget( cstatus, 2, 1 );

  QPushButton* fstatus = new QPushButton( "Fi(l)es", this, "fstatus" );
  fstatus->setFont(butfont);
  buttons->addWidget( fstatus, 2, 1 );

  QPushButton* quit = new QPushButton( "(Q)uit", this, "quit" );
  quit->setFont(butfont);
  buttons->addWidget( quit, 3, 1);

  connect(rewind, SIGNAL(clicked()), this, SLOT(emsg_rewind()) );
  connect(rw_begin, SIGNAL(clicked()), this, SLOT(emsg_rw_begin()) );
  connect(start, SIGNAL(clicked()), this, SLOT(emsg_start()) );
  connect(stop, SIGNAL(clicked()), this, SLOT(emsg_stop()) );
  connect(forward, SIGNAL(clicked()), this, SLOT(emsg_forward()) );
  connect(status, SIGNAL(clicked()), this, SLOT(emsg_status()) );
  connect(estatus, SIGNAL(clicked()), this, SLOT(emsg_estatus()) );
  connect(fstatus, SIGNAL(clicked()), this, SLOT(emsg_fstatus()) );
  connect(cstatus, SIGNAL(clicked()), this, SLOT(emsg_cstatus()) );
  //  connect(quit, SIGNAL(clicked()), this, SLOT(emsg_quit()) );
  connect(quit, SIGNAL(clicked()), qApp, SLOT(closeAllWindows()) );

}

void QEInterface::init_textinput(QBoxLayout* textinput) {
  QLabel* tekstiinfo = new QLabel("qtecasound CL(I) ('h' for help): ", this, "tekstiinfo");
  textinput->addWidget( tekstiinfo, 5);
  
  tekstirivi = new QLineEdit(this, "tekstirivi");
  textinput->addWidget( tekstirivi, 10);
  
  connect(tekstirivi, SIGNAL(returnPressed ()), this, SLOT(emsg_general()) );
  connect(this, SIGNAL(clear_textinput()), tekstirivi, SLOT(clear()) );
}

void QEInterface::emsg_general(void) {
  string s (tekstirivi->text());
  if (s == "q" || s == "quit") {
    //    emsg_quit();
    qApp->closeAllWindows();
  }
  else 
    ctrl->command(s);

  emit clear_textinput();
}

void QEInterface::emsg_quit(void) { 
  //  qApp->closeAllWindows();
  //  emit is_finished();
  //  ctrl->quit();

}

void QEInterface::emsg_stop(void) { ctrl->command("s"); }
void QEInterface::emsg_start(void) { ctrl->command("t"); }
void QEInterface::emsg_rw_begin(void) { ctrl->command("setpos 0"); }
void QEInterface::emsg_forward(void) { ctrl->command("fw 5"); }
void QEInterface::emsg_rewind(void) { ctrl->command("rw 5"); }
void QEInterface::emsg_status(void) { ctrl->command("st"); }
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

void QEInterface::init_shortcuts(void) {
  QAccel *a = new QAccel(this);

  a->connectItem(a->insertItem(CTRL+Key_A), this,
		 SLOT(emsg_cstatus()));

  a->connectItem(a->insertItem(CTRL+Key_B), this,
		 SLOT(emsg_rw_begin()));

  a->connectItem(a->insertItem(CTRL+Key_F), this,
		 SLOT(emsg_forward()));

  a->connectItem(a->insertItem(CTRL+Key_I), tekstirivi,
		 SLOT(setFocus()));

  a->connectItem(a->insertItem(CTRL+Key_L), this,
		 SLOT(emsg_fstatus()));

  a->connectItem(a->insertItem(CTRL+Key_P), this,
		 SLOT(init_sessionsetup()));

  a->connectItem(a->insertItem(CTRL+Key_Q), qApp,
		 SLOT(closeAllWindows()));

  a->connectItem(a->insertItem(CTRL+Key_R), this,
		 SLOT(emsg_rewind()));

  a->connectItem(a->insertItem(CTRL+Key_U), this,
		 SLOT(emsg_status()));

  a->connectItem(a->insertItem(CTRL+Key_X), this,
		 SLOT(emsg_estatus()));
}

void QEInterface::keyPressEvent(QKeyEvent* kevent) {
  switch(tolower(kevent->ascii())) {
  case 'q': {
    //    close();
    qApp->closeAllWindows();
    break;
  }
  case 'b': {    
    emsg_rw_begin();
    break;
  }
  case 'f': {
    emsg_forward();
    break;
  }
  case 'r': {    
    emsg_rewind();
    break;
  }
  case 'u': {    
    emsg_status();
    break;
  }
  case 'x': {    
    emsg_estatus();
    break;
  }
  case 'a': {    
    emsg_cstatus();
    break;
  }
  case 'l': {    
    emsg_fstatus();
    break;
  }
  case 'i': {    
    tekstirivi->setFocus();
    break;
  }
  case 'p': {    
    init_sessionsetup();
    break;
  }
  case 't': { } // start
  case 's': {   // stop
    MESSAGE_ITEM m;
    m << (char)kevent->ascii();
    ctrl->command(m.to_string());
    break;
  }
  }
  kevent->ignore();
}

void QEInterface::timerEvent( QTimerEvent * ) {
  //  static long loop_counter = 0;
  //  static int n;
  //  static bool here = false; 
  //  static unsigned long int usleep_count;


  //  if (here == true) return;
  //  else here = true;
  //  if (ecaparams->status() != ep_status_running) {
  //    emit mute_signallevels();
  //    return;
  //  }

 //  cerr << "a";
  //  if (loop_counter == ecaparams->loop_counter) return;
  //  loop_counter = ecaparams->loop_counter;

  //  for(n = 0; n < ecaparams->outputs->size(); n++) {
    //    if (pthread_mutex_trylock(ecaparams->out_locks[n]) != 0) {
    //      --loop_counter;
    //      break;
    //    }
  //    pthread_mutex_lock(ecaparams->out_locks[n]);
  //    if (ecaparams->status() == ep_status_running) 
  //      emit update_signallevel(n);
  //    else 
  //      emit mute_signallevels();
    //    cerr << "b";
  //    pthread_mutex_unlock(ecaparams->out_locks[n]);
    // cerr << "c";
    //    emit update_signallevel(n);
	  //    pthread_mutex_unlock(ecaparams->out_locks[n]);
    // cerr << "d";
  //  }
  //  here = false;
}

