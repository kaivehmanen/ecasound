// ------------------------------------------------------------------------
// ecatools_convert.cpp: A simple command-line tool for converting
//                       audio files.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <string>
#include <cstdio>

#include <kvutils/com_line.h>

#include <eca-debug.h>
#include <eca-error.h>
#include <eca-control.h>
#include <eca-main.h>
#include <eca-session.h>
#include <eca-version.h>

#include "ecatools_convert.h"

int main(int argc, char *argv[])
{
#ifdef NDEBUG
  ecadebug->disable();
#else
  ecadebug->set_debug_level(ECA_DEBUG::info |
			    ECA_DEBUG::module_flow);
#endif

  COMMAND_LINE cline = COMMAND_LINE (argc, argv);

  if (cline.size() < 2) {
    print_usage();
    return(1);
  }

  try {
    string filename;

    ECA_SESSION esession;
    ECA_CONTROL ectrl (&esession);
    ECA_PROCESSOR emain;
    ECA_AUDIO_FORMAT aio_params;

    cline.begin();
    cline.next(); // skip the program name

    string extension (".raw");
    if (cline.end() != true) {
      extension = cline.current();
      cline.next();
    }

    while(cline.end() != true) {
      filename = cline.current();
      cerr << "Converting file \"" << filename << "\" --> ";
      cerr << "\"" << filename + extension << "\"." << endl;

      ectrl.add_chainsetup("default");
      ectrl.add_chain("default");
      ectrl.add_audio_input(filename);
      if (ectrl.get_audio_input() == 0) {
	cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	break;
      }
      aio_params = ectrl.get_audio_format(ectrl.get_audio_input());
      ectrl.set_default_audio_format(aio_params);
      ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
      ectrl.add_audio_output(filename + extension);
      if (ectrl.get_audio_output() == 0) {
	cerr << "---\nError while processing file " << filename + extension << ". Exiting...\n";
	break;
      }
      ectrl.connect_chainsetup();
      if (ectrl.is_connected() == false) {
	cerr << "---\nError while converting file " << filename << ". Exiting...\n";
	break;
      }
      
      emain.init(&esession);
      emain.exec();

      ectrl.disconnect_chainsetup();
      ectrl.remove_chainsetup();

      cline.next();
    }
  }
  catch(ECA_ERROR& e) {
    cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
  catch(...) {
    cerr << "\nCaught an unknown exception.\n";
  }
  return(0);
}

void print_usage(void) {
  cerr << "****************************************************************************\n";
  cerr << "* ecatools_convert, v" << ecatools_play_version;
  cerr << " (linked to ecasound v" << ecasound_library_version 
       << ")\n";
  cerr << "* (C) 1997-2000 Kai Vehmanen, released under GPL licence \n";
  cerr << "****************************************************************************\n";

  cerr << "\nUSAGE: ecatools_convert .extension file1 [ file2, ... fileN ]\n\n";
}
