// ------------------------------------------------------------------------
// ecawave.cpp: Ecawave initialization and startup routines.
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

#include <kvutils/definition_by_contract.h>

#include "qesession.h"

int main(int argc, char **argv) {
  try {
    QApplication qapp (argc, argv);
    QESession* ewsession;
    if (argc > 1) {
      ewsession = new QESession(argv[1]);
    }
    else {
      ewsession = new QESession();
    }
    qapp.setMainWidget(ewsession);
    ewsession->show();
    
    QObject::connect( &qapp, SIGNAL( lastWindowClosed() ), &qapp, SLOT( quit() ) );
    return(qapp.exec());
  }
  catch(DBC_EXCEPTION* e) { 
    e->print();
    exit(1);
  }
  catch(...) {
    cerr << "---\nCaught an unknown exception!\n";
  }
}
