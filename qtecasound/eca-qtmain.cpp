// ------------------------------------------------------------------------
// eca-qtmain.cpp: GUI routines for qtecasound (based on QT-libraries).
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

#include <qapplication.h>
#include <qpushbutton.h>
#include <signal.h>

#include <kvutils.h>

#include "eca-debug.h"
#include "eca-error.h"
#include "eca-main.h"
#include "eca-session.h"
#include "eca-controller.h"

#include "eca-qtinte.h"
#include "eca-qtmain.h"
#include "qtdebug_if.h"

ECA_SESSION* global_pointer_to_ecaparams = 0; 
bool global_session_deleted = false;
QTDEBUG_IF qtdebug_if;

int main( int argc, char **argv )
{
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGQUIT, signal_handler);

  ECA_SESSION* param = 0;

  try {
    QApplication a (argc, argv);
    attach_debug_object(&qtdebug_if);  
    ecadebug->set_debug_level(0);

    COMMAND_LINE cline = COMMAND_LINE (argc, argv);

    if (cline.has("-o:stdout") ||
	cline.has("stdout") || 
	cline.has('q'))
      ecadebug->disable();

    cline.push_back("-c");

    param = new ECA_SESSION (cline);
    global_pointer_to_ecaparams = param;  // used only for signal handling! 

    ECA_CONTROLLER* ctrl = new ECA_CONTROLLER (param);

    //    ctrl->enable_mthreaded_use();

    QEInterface w (ctrl, param);

    //    start_normal_thread(param);

    QObject::connect( &w, SIGNAL(is_finished()), &a, SLOT(quit()));
    QObject::connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    a.setMainWidget(&w);
    w.show();
    a.exec();
    //    a.~QApplication();
  }
  catch(int n) {
    if (n == ECA_QUIT) 
      ecadebug->msg("(eca-qtmain) Exiting...");
  }
  catch(ECA_ERROR* e) {
    cerr << "\n---\nERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }    
  catch(...) {
    cerr << "\n---\nCaught an unknown exception!\n";
  }

  try {
    if (global_session_deleted == false) {
      global_session_deleted = true;
      if (param != 0) delete param;
    }
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\nCaught an unknown exception!\n";
  }
}

void signal_handler(int signum) {
  cerr << "Unexpected interrupt... cleaning up.\n";
  if (global_session_deleted == false) {
    global_session_deleted = true;
    if (global_pointer_to_ecaparams != 0) delete global_pointer_to_ecaparams;
  }
  remove(ecasound_lockfile.c_str());

  exit(1);
}
