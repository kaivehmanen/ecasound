#include <iostream>
#include <string>
#include <cstdio>
#include <string>
#include <cstdlib>

#include "eca-logger.h"
#include "eca-test-repository.h"

#include "ecatestsuite.h"

using namespace std;

/**
 * See also 'ecasound/libecasound/libecasound_tester.cpp'
 */

int main(int argc, char *argv[]) {
  ECA_TEST_ENTRY();

  /**
   * Uncomment to enable libecasound log messages
   */
  ECA_LOGGER::instance().set_log_level_bitmask(0xff);

  ECA_TEST_REPOSITORY& repo = ECA_TEST_REPOSITORY::instance();

  repo.run();

  if (repo.success() != true) {
    cerr << "---" << endl <<; 
    cerr << repo.failures().size() << " failed test cases ";
    cerr << "in ECA_TEST_REPOSITORY:" << endl << endl;

    const list<string>& failures = repo.failures();
    list<string>::const_iterator q = failures.begin();
    int n = 1;
    while(q != failures.end()) {
      cerr << n++ << ". " << *q << endl;
      ++q;
    }

    ECA_TEST_FAIL(1, "ECA_TEST_REPOSITORY");
  }

  ECA_TEST_SUCCESS();
}
