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
#include <stdlib.h>

#include <kvutils/kvu_com_line.h>
#include <kvutils/kvu_temporary_file_directory.h>
#include <kvutils/kvu_numtostr.h>

#include <samplebuffer.h>
#include <eca-logger.h>
#include <eca-error.h>
#include <eca-control.h>
#include <eca-engine.h>
#include <eca-session.h>
#include <audiofx_misc.h>
#include <audiofx_analysis.h>
#include <audioio.h>
#include <eca-version.h>

#include "ecatools_fixdc.h"

static const string ecatools_fixdc_version = "20011009";
static string ecatools_fixdc_tempfile;

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
    std::string filename;
    std::string tempfile;
    std::vector<double> dcfix_value (2);
    EFFECT_DCFIND* dcfind = 0;
    EFFECT_DCFIX* dcfix = 0;

    ECA_SESSION esession;
    ECA_CONTROL ectrl (&esession);
    ECA_AUDIO_FORMAT aio_params;

    TEMPORARY_FILE_DIRECTORY tempfile_dir_rep;
    string tmpdir ("ecatools-");
    char* tmp_p = getenv("USER");
    if (tmp_p != NULL) {
      tmpdir += string(tmp_p);
      tempfile_dir_rep.reserve_directory(tmpdir);
    }
    if (tempfile_dir_rep.is_valid() != true) {
      std::cerr << "---\nError while creating temporary directory \"" << tmpdir << "\". Exiting...\n";
      return(0);
    }

    cline.begin();
    cline.next(); // skip the program name
    while(cline.end() == false) {
      filename = cline.current();

      ecatools_fixdc_tempfile = tempfile_dir_rep.create_filename("fixdc-tmp", ".wav");

      for(int m = 0;m < 2; m++) {
	ectrl.add_chainsetup("default");
	ectrl.add_chain("default");
	if (m == 0) {
	  std::cerr << "Calculating DC-offset for file \"" << filename << "\".\n";
	  ectrl.add_audio_input(filename);
	  if (ectrl.get_audio_input() == 0) {
	    std::cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	    break;
	  }
	  ectrl.set_default_audio_format_to_selected_input();
	  aio_params = ectrl.default_audio_format();
	  ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	  ectrl.add_audio_output(ecatools_fixdc_tempfile);
	  if (ectrl.get_audio_output() == 0) {
	    std::cerr << "---\nError while processing file " << ecatools_fixdc_tempfile << ". Exiting...\n";
	    break;
	  }


	  dcfind = new EFFECT_DCFIND();
	  ectrl.add_chain_operator((CHAIN_OPERATOR*)dcfind);
	}
	else {
	  std::cerr << "Fixing DC-offset \"" << filename << "\"";
	  std::cerr << " (left: " << dcfix_value[SAMPLE_SPECS::ch_left];
	  std::cerr << ", right: " << dcfix_value[SAMPLE_SPECS::ch_right]
	       << ").\n";
	  ectrl.add_audio_input(ecatools_fixdc_tempfile);
	  if (ectrl.get_audio_input() == 0) {
	    std::cerr << "---\nError while processing file " << ecatools_fixdc_tempfile << ". Exiting...\n";
	    break;
	  }
	  ectrl.set_default_audio_format_to_selected_input();
	  aio_params = ectrl.default_audio_format();
	  ectrl.set_chainsetup_parameter("-sr:" + kvu_numtostr(aio_params.samples_per_second()));
	  ectrl.add_audio_output(filename);
	  if (ectrl.get_audio_output() == 0) {
	    std::cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	    break;
	  }

	  dcfix = new EFFECT_DCFIX(dcfix_value[SAMPLE_SPECS::ch_left],
				   dcfix_value[SAMPLE_SPECS::ch_right]);
	  ectrl.add_chain_operator((CHAIN_OPERATOR*)dcfix);
	}
	ectrl.connect_chainsetup();
	if (ectrl.is_connected() == false) {
	  std::cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	  break;
	}
	else {
	  // blocks until processing is done
	  ectrl.run();
	}
	
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
  catch(ECA_ERROR& e) {
    std::cerr << "---\nERROR: [" << e.error_section() << "] : \"" << e.error_message() << "\"\n\n";
  }
  catch(...) {
    std::cerr << "\nCaught an unknown exception.\n";
  }
  return(0);
}

void print_usage(void) {
  std::cerr << "****************************************************************************\n";
  std::cerr << "* ecatools_fixdc, v" << ecatools_fixdc_version;
  std::cerr << " (linked to ecasound v" << ecasound_library_version 
       << ")\n";
  std::cerr << "* (C) 1997-2001 Kai Vehmanen, released under GPL licence \n";
  std::cerr << "****************************************************************************\n";

  std::cerr << "\nUSAGE: ecafixdc file1 [ file2, ... fileN ]\n\n";
}

void signal_handler(int signum) {
  std::cerr << "Unexpected interrupt... cleaning up.\n";
  remove(ecatools_fixdc_tempfile.c_str());
  exit(1);
}
