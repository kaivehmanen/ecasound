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
#include <kvutils/com_line.h>
#include <ecasound/eca-error.h>
#include <ecasound/eca-version.h>

#include "version.h"
#include "ecawave.h"
#include "qesession.h"

int main(int argc, char **argv) {
  try {
    COMMAND_LINE cline = COMMAND_LINE (argc, argv);
    parse_command_line(cline);

    QApplication qapp (argc, argv);
    string param;
    if (argc > 1) param = string(argv[1]);
    QESession ewsession (param);
    qapp.setMainWidget(&ewsession);
    ewsession.show();
    
    QObject::connect( &qapp, SIGNAL( lastWindowClosed() ), &qapp, SLOT( quit() ) );
    return(qapp.exec());
  }
  catch(DBC_EXCEPTION* e) { 
    e->print();
    exit(1);
  }
  catch(ECA_ERROR* e) { 
    cerr << "---\nlibecasound error while processing event: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "---\nCaught an unknown exception!\n";
  }
}

void parse_command_line(COMMAND_LINE& cline) {
  cline.begin();
  while(cline.end() == false) {
    if (cline.current() == "--version") {
      cout << "ecawave "
	   << ecawave_version
	   << " ["
	   << ecasound_library_version
	   << "]" << endl;
      cout << "Copyright (C) 1999-2000 Kai Vehmanen" << endl;
      cout << "Ecawave comes with ABSOLUTELY NO WARRANTY." << endl;
      cout << "You may redistribute copies of ecawave under the terms of the GNU General Public License." << endl; 
      cout << "For more information about these matters, see the file named COPYING." << endl;
      exit(0);
    }
    else if (cline.current() == "--help") {
	cout << "USAGE: ecawave [options] filename\n" 
	     << "    --version   print version info" << endl
	     << "    --help      show this help" << endl << endl;
	cout << "For a more detailed documentation, see ecawave(1) man  page." << endl;
	cout << "Report bugs to <k@eca.cx>." << endl;
	exit(0);
      }
    cline.next();
  }
}
