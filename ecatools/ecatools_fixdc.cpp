// ------------------------------------------------------------------------
// ecatools-fixdc.cpp: A simple command-line tools for fixing DC-offset.
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

#include <string>
#include <vector>
#include <cstdio>
#include <signal.h>

#include <kvutils/com_line.h>

#include <samplebuffer.h>
#include <eca-debug.h>
#include <eca-error.h>
#include <eca-control.h>
#include <eca-main.h>
#include <eca-session.h>
#include <audiofx.h>
#include <audiofx_analysis.h>
#include <audioio.h>
#include <eca-version.h>

#include "ecatools_fixdc.h"

int main(int argc, char *argv[])
{
#ifdef NDEBUG
  ecadebug->disable();
#else
  ecadebug->set_debug_level(ECA_DEBUG::info |
			    ECA_DEBUG::module_flow);
#endif

  struct sigaction es_handler;
  es_handler.sa_handler = signal_handler;
  sigemptyset(&es_handler.sa_mask);
  es_handler.sa_flags = 0;

  sigaction(SIGTERM, &es_handler, 0);
  sigaction(SIGINT, &es_handler, 0);
  sigaction(SIGQUIT, &es_handler, 0);
  sigaction(SIGABRT, &es_handler, 0);

  COMMAND_LINE cline = COMMAND_LINE (argc, argv);

  if (cline.size() < 2) {
    print_usage();
    return(1);
  }

  try {
    string filename;
    string tempfile;
    vector<double> dcfix_value (2);
    EFFECT_DCFIND* dcfind = 0;
    EFFECT_DCFIX* dcfix = 0;

    ECA_SESSION esession;
    ECA_CONTROL ectrl (&esession);
    ECA_PROCESSOR emain;
    ECA_AUDIO_FORMAT aio_params;

    ectrl.toggle_interactive_mode(false);
    cline.begin();
    cline.next(); // skip the program name
    while(cline.end() == false) {
      filename = cline.current();

      ecatools_fixdc_tempfile = string(tmpnam(NULL));
      ecatools_fixdc_tempfile += ".wav";
      for(int m = 0;m < 2; m++) {
	ectrl.add_chainsetup("default");
	ectrl.add_chain("default");
	if (m == 0) {
	  cerr << "Calculating DC-offset for file \"" << filename << "\".\n";
	  ectrl.add_audio_input(filename);
	  aio_params = ectrl.get_audio_format();
	  ectrl.set_default_audio_format(aio_params);
	  ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	  ectrl.add_audio_output(ecatools_fixdc_tempfile);

	  dcfind = new EFFECT_DCFIND();
	  ectrl.add_chain_operator((CHAIN_OPERATOR*)dcfind);
	}
	else {
	  cerr << "Fixing DC-offset \"" << filename << "\"";
	  cerr << " (left: " << dcfix_value[SAMPLE_SPECS::ch_left];
	  cerr << ", right: " << dcfix_value[SAMPLE_SPECS::ch_right]
	       << ").\n";
	  ectrl.add_audio_input(ecatools_fixdc_tempfile);
	  aio_params = ectrl.get_audio_format();
	  ectrl.set_default_audio_format(aio_params);
	  ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	  ectrl.set_default_audio_format(aio_params);
	  ectrl.add_audio_output(filename);

	  dcfix = new EFFECT_DCFIX(dcfix_value[SAMPLE_SPECS::ch_left],
				   dcfix_value[SAMPLE_SPECS::ch_right]);
	  ectrl.add_chain_operator((CHAIN_OPERATOR*)dcfix);
	}
	ectrl.connect_chainsetup();
	if (ectrl.is_connected() == false) {
	  cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	  break;
	}
  
	emain.init(&esession);
	emain.exec();
	
	if (m == 0) {
	  for(int nm = 0; nm < 2; nm++) {
	    dcfix_value[nm] = dcfind->get_deltafix(nm);
	  }
	}
	ectrl.disconnect_chainsetup();
	ectrl.select_chainsetup("default");
	ectrl.remove_chainsetup();
      }
      remove(ecatools_fixdc_tempfile.c_str());
      cline.next();
    }
  }
  catch(ECA_ERROR* e) {
    cerr << "---\nERROR: [" << e->error_section() << "] : \"" << e->error_msg() << "\"\n\n";
  }
  catch(...) {
    cerr << "\nCaught an unknown exception.\n";
  }
  return(0);
}

void print_usage(void) {
  cerr << "****************************************************************************\n";
  cerr << "* [1mecatools_fixdc, v" << ecatools_fixdc_version;
  cerr << " (linked to ecasound v" << ecasound_library_version 
       << ")\n";
  cerr << "* (C) 1997-2000 Kai Vehmanen, released under GPL licence[0m \n";
  cerr << "****************************************************************************\n";

  cerr << "\nUSAGE: ecatools_fixdc file1 [ file2, ... fileN ]\n\n";
}

void signal_handler(int signum) {
  cerr << "Unexpected interrupt... cleaning up.\n";
  remove(ecatools_fixdc_tempfile.c_str());
  exit(1);
}
