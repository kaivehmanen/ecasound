// ------------------------------------------------------------------------
// libecasound_tester.cpp: Runs all tests registered to ECA_TEST_REPOSITORY
// Copyright (C) 2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include "eca-logger.h"
#include "eca-test-repository.h"

using namespace std;

int main(int argc, char *argv[]) {
  /**
   * Uncomment to enable libecasound log messages
   */
  // ECA_LOGGER::instance().set_log_level_bitmask(63);

  ECA_TEST_REPOSITORY& repo = ECA_TEST_REPOSITORY::instance();

  repo.run();

  if (repo.success() != true) {
    cerr << "---" << endl;
    cerr << repo.failures().size() << " failed test cases ";
    cerr << "in ECA_TEST_REPOSITORY:" << endl << endl;

    const list<string>& failures = repo.failures();
    list<string>::const_iterator q = failures.begin();
    int n = 1;
    while(q != failures.end()) {
      cerr << n++ << ". " << *q << endl;
      ++q;
    }
    
    cerr << "---" << endl << endl;
    
    return -1;
  }

  return 0;
}
