#include <cstdio>
#include <string>
#include <stdlib.h>

#include "ecatestsuite.h"

using namespace std;

static const string ecasound_exec ("./ecasound_test");
static const string rtobject("rtnull");
static const string nonrtobject("null");

static void eci_execute_test(const string& cmd);
static void eci_combination_test(string arg);

int main(int argc, char *argv[]) {
  ECA_TEST_ENTRY();

  eci_combination_test("-B:auto");
  eci_combination_test("-B:nonrt");
  eci_combination_test("-B:rt");
  eci_combination_test("-B:rtlowlatency");

  eci_combination_test("-b:128 -z:nointbuf -z:db -r:50");
  eci_combination_test("-b:8192 -z:intbuf -z:db -r:50");
  eci_combination_test("-b:1024 -z:nodb -r:50");

  ECA_TEST_SUCCESS();
}

void eci_combination_test(string arg) {
  eci_execute_test(ecasound_exec +
		   " -a:1 -i:" + rtobject + " -o:" + nonrtobject + " " + arg + " -t:2 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + 
		   " -a:1 -i:" + nonrtobject + " -o:" + rtobject + " " + arg + " -t:2 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + 
		   " -a:1 -i:" + nonrtobject + " -o:" + nonrtobject + " " + arg + " -t:2 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + 
		   " -a:1 -i:" + rtobject + " -o:" + rtobject + " " + arg + " -t:2 2>/dev/null >/dev/null");
}

void eci_execute_test(const string& cmd) {
  ECA_TEST_CASE();

  cout << endl << "---" << endl;
  cout << "Running test case:" << endl;
  cout << cmd << endl;

  if (system(cmd.c_str()) != 0) {
    ECA_TEST_FAIL(1, cmd.c_str());
  }
}
