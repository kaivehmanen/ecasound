// ------------------------------------------------------------------------
// ecatools-fixdc.cpp: A simple command-line tools for fixing DC-offset.
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

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <cassert>

#include <signal.h>
#include <stdlib.h>

#include <kvutils/kvu_com_line.h>
#include <kvutils/kvu_temporary_file_directory.h>
#include <kvutils/kvu_numtostr.h>
#include <kvutils/kvu_utils.h>

#include <eca-control-interface.h>

/**
 * Function declarations
 */

int main(int argc, char *argv[]);
void print_usage(void);
void signal_handler(int signum);

/**
 * Definitions and options 
 */

#define ECAFIXDC_PHASE_ANALYSIS   0
#define ECAFIXDC_PHASE_PROCESSING 1
#define ECAFIXDC_PHASE_MAX        2

using std::cerr;
using std::cout;
using std::endl;
using std::string;

static const string ecatools_fixdc_version = "20021028-28";
static string ecatools_fixdc_tempfile;

/**
 * Function definitions
 */

int main(int argc, char *argv[])
{
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

  std::string filename;
  std::string tempfile;
  std::vector<double> dcfix_values;
  int chcount = 0;

  ECA_CONTROL_INTERFACE eci;

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

  cline.begin();
  cline.next(); // skip the program name
  while(cline.end() == false) {
    filename = cline.current();

    ecatools_fixdc_tempfile = tempfile_dir_rep.create_filename("fixdc-tmp", ".wav");

    for(int m = 0;m < ECAFIXDC_PHASE_MAX; m++) {

      eci.command("cs-add default");
      eci.command("c-add default");

      if (m == ECAFIXDC_PHASE_ANALYSIS) {
	cout << "Calculating DC-offset for file \"" << filename << "\".\n";

	eci.command("ai-add " + filename);
	eci.command("ai-list");
	if (eci.last_string_list().size() != 1) {
	  cerr << eci.last_error() << endl;
	  cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	  break;
	}

	eci.command("ai-get-format");
	string format = eci.last_string();
	cout << "Using audio format -f:" << format << "\n";
	eci.command("cs-set-audio-format " +  format);
	std::vector<std::string> tokens = kvu_string_to_vector(format, ',');
	assert(tokens.size() >= 3);
	chcount = atoi(tokens[1].c_str());
	dcfix_values.resize(chcount);
	cout << "Setting up " << chcount << " separate channels for analysis." << endl;

	cout << "Opening temp file \"" << ecatools_fixdc_tempfile << "\".\n";

	eci.command("ao-add " + ecatools_fixdc_tempfile);
	eci.command("ao-list");
	if (eci.last_string_list().size() != 1) {
	  cerr << eci.last_error() << endl;
	  cerr << "---\nError while processing file " << ecatools_fixdc_tempfile << ". Exiting...\n";
	  break;
	}

	eci.command("cop-add -ezf");
	eci.command("cop-list");
	if (eci.last_string_list().size() != 1) {
	  cerr << eci.last_error() << endl;
	  cerr << "---\nError while adding DC-Find (-ezf) chainop. Exiting...\n";
	  break;
	}
      }
      else {
	// FIXME: list all channels (remember to fix audiofx_misc.cpp dcfix)
	cout << "Fixing DC-offset \"" << filename << "\"";
	cout << " (left: " << kvu_numtostr(dcfix_values[0]);
	cout << ", right: " << kvu_numtostr(dcfix_values[1]) << ").\n";

	eci.command("ai-add " + ecatools_fixdc_tempfile);
	eci.command("ai-list");
	if (eci.last_string_list().size() != 1) {
	  cerr << eci.last_error() << endl;
	  cerr << "---\nError while processing file " << ecatools_fixdc_tempfile << ". Exiting...\n";
	  break;
	}

	eci.command("ai-get-format");
	string format = eci.last_string();
	cout << "Using audio format -f:" << format << "\n";
	eci.command("cs-set-audio-format " +  format);

	eci.command("ao-add " + filename);
	eci.command("ao-list");
	if (eci.last_string_list().size() != 1) {
	  cerr << eci.last_error() << endl;
	  cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	  break;
	}

	string dcfixstr;
	for(int n = 0; n < chcount; n++) {
	  dcfixstr += kvu_numtostr(dcfix_values[n]) + ",";
	}

	eci.command("cop-add -ezx:" + dcfixstr);
	eci.command("cop-list");
	if (eci.last_string_list().size() != 1) {
	  cerr << eci.last_error() << endl;
	  cerr << "---\nError while adding DC-Fix (-ezx) chainop. Exiting...\n";
	  break;
	}
      }

      cout << "Starting processing...\n";	

      eci.command("cs-connect");
      eci.command("cs-connected");
      if (eci.last_string() != "default") {
	cerr << eci.last_error() << endl;
	cerr << "---\nError while processing file " << filename << ". Exiting...\n";
	break;
      }
      else {
	// blocks until processing is done
	eci.command("run");
      }

      if (m == ECAFIXDC_PHASE_ANALYSIS) {
	assert(static_cast<int>(dcfix_values.size()) >= chcount);
	for(int nm = 0; nm < chcount; nm++) {
	  eci.command("cop-select 1");
	  eci.command("copp-select " + kvu_numtostr(nm + 1));
	  eci.command("copp-get");
	  dcfix_values[nm] = eci.last_float();
	  cout << "DC-offset for channel " << nm + 1 << " is " <<
	    dcfix_values[nm] << "." << endl;
	}
      }

      cout << "Processing finished.\n";

      eci.command("cs-disconnect");
      eci.command("cs-select default");
      eci.command("cs-remove");
    }

    remove(ecatools_fixdc_tempfile.c_str());

    cline.next();
  }

  return(0);
}

void print_usage(void)
{
  std::cerr << "****************************************************************************\n";
  std::cerr << "* ecafixdc, v" << ecatools_fixdc_version << "\n";
  std::cerr << "* (C) 1997-2002 Kai Vehmanen, released under the GPL license\n";
  std::cerr << "****************************************************************************\n";

  std::cerr << "\nUSAGE: ecafixdc file1 [ file2, ... fileN ]\n\n";
}

void signal_handler(int signum)
{
  std::cerr << "Unexpected interrupt... cleaning up.\n";
  remove(ecatools_fixdc_tempfile.c_str());
  exit(1);
}
