// ------------------------------------------------------------------------
// ecatools-normalize.cpp: A simple command-line tools for normalizing
//                         sample volume.
// Copyright (C) 1999-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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
#include <signal.h>
#include <stdlib.h>

#include <kvutils/kvu_com_line.h>
#include <kvutils/kvu_temporary_file_directory.h>
#include <kvutils/kvu_numtostr.h>

#include <eca-logger.h>
#include <eca-error.h>
#include <eca-control.h>
#include <eca-engine.h>
#include <eca-session.h>
#include <audiofx_analysis.h>
#include <audiofx_amplitude.h>
#include <audioio.h>
#include <eca-version.h>

#include "ecatools_normalize.h"

using std::cerr;
using std::endl;

static const string ecatools_normalize_version = "20020717-24";
static string ecatools_normalize_tempfile;

int main(int argc, char *argv[])
{
#ifdef NDEBUG
  ECA_LOGGER::instance().disable();
#else
  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::errors, true);
  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::info, true);
  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::subsystems, true);
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
    double multiplier = 1.0f;
    EFFECT_VOLUME_BUCKETS* volume = 0;
    EFFECT_AMPLIFY* amp = 0;
    
    TEMPORARY_FILE_DIRECTORY tempfile_dir_rep;
    string tmpdir ("ecatools-");
    char* tmp_p = getenv("USER");
    if (tmp_p != NULL) {
      tmpdir += string(tmp_p);
      tempfile_dir_rep.reserve_directory(tmpdir);
    }
    if (tempfile_dir_rep.is_valid() != true) {
      cerr << "---\nError while creating temporary directory \"" << tmpdir << "\". Exiting...\n";
      return(0);
    }

    ecatools_normalize_tempfile = tempfile_dir_rep.create_filename("normalize-tmp", ".wav");

    ECA_SESSION esession;
    ECA_CONTROL ectrl (&esession);
    ECA_AUDIO_FORMAT aio_params;

    cline.begin();
    cline.next(); // skip the program name
    while(cline.end() == false) {
      filename = cline.current();

      for(int m = 0;m < 2; m++) {

	ectrl.add_chainsetup("default");
	ectrl.add_chain("default");
	if (m == 0) {
	  cerr << "Analyzing file \"" << filename << "\".\n";
	  ectrl.add_audio_input(filename);
	  if (ectrl.get_audio_input() == 0) {
	    cerr << ectrl.last_error() << endl;
	    cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	    break;
	  }
	  ectrl.set_default_audio_format_to_selected_input();
	  aio_params = ectrl.default_audio_format();
	  ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	  ectrl.add_audio_output(string(ecatools_normalize_tempfile));
	  if (ectrl.get_audio_output() == 0) {
	    cerr << ectrl.last_error() << endl;
	    cerr << "---\nError while processing file " << ecatools_normalize_tempfile << ". Exiting...\n";
	    break;
	  }

	  volume = new EFFECT_VOLUME_BUCKETS();
	  ectrl.add_chain_operator((CHAIN_OPERATOR*)volume);
	}
	else {
	  ectrl.add_audio_input(string(ecatools_normalize_tempfile));
	  if (ectrl.get_audio_input() == 0) {
	    cerr << ectrl.last_error() << endl;
	    cerr << "---\nError while processing file " << ecatools_normalize_tempfile << ". Exiting...\n";
	    break;
	  }
	  ectrl.set_default_audio_format_to_selected_input();
	  aio_params = ectrl.default_audio_format();	  
	  ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	  ectrl.add_audio_output(filename);
	  if (ectrl.get_audio_output() == 0) {
	    cerr << ectrl.last_error() << endl;
	    cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	    break;
	  }
	  
	  amp = new EFFECT_AMPLIFY(multiplier * 100.0);
	  ectrl.add_chain_operator((CHAIN_OPERATOR*)amp);
	}
	ectrl.connect_chainsetup();
	if (ectrl.is_connected() == false) {
	  cerr << ectrl.last_error() << endl;
	  cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	  break;
	}
	else {
	  // blocks until processing is done
	  ectrl.run();
	}

	if (m == 0) {
	  multiplier = volume->max_multiplier();
	  if (multiplier <= 1.0) {
	    cerr << "File \"" << filename << "\" is already normalized.\n";
	    ectrl.disconnect_chainsetup();
	    ectrl.select_chainsetup("default");
	    ectrl.remove_chainsetup();
	    break;
	  }
	  else {
	    cerr << "Normalizing file \"" << filename << "\" (amp-%: ";
	    cerr << multiplier * 100.0 << ").\n";
	  }
	}
	ectrl.disconnect_chainsetup();
	ectrl.select_chainsetup("default");
	ectrl.remove_chainsetup();
      }
      remove(ecatools_normalize_tempfile.c_str());
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
  cerr << "* [1mecatools_normalize, v" << ecatools_normalize_version;
  cerr << " (linked to ecasound v" << ecasound_library_version 
       << ")\n";
  cerr << "* (C) 1997-2000 Kai Vehmanen, released under GPL licence[0m \n";
  cerr << "****************************************************************************\n";

  cerr << "\nUSAGE: ecatools_normalize file1 [ file2, ... fileN ]\n\n";
}

void signal_handler(int signum) {
  cerr << "Unexpected interrupt... cleaning up.\n";
  remove(ecatools_normalize_tempfile.c_str());
  exit(1);
}
