#include <cstdio>
#include <string>
#include <stdlib.h>

#include <kvutils/kvu_numtostr.h>
#include "ecatestsuite.h"

using namespace std;

static const string ecasound_exec ("./ecasound_test");
static const string input_file("foo.wav");

static void eci_execute_test(const string& cmd);

int main(int argc, char *argv[]) {
  ECA_TEST_ENTRY();

  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -ggc:1,0.5 -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -i:" + input_file + " -o:null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -d:255 -z:db -r -z:nointbuf -i:" + input_file + " -o:null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + "  -i:" + input_file + " -o:null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -efl:400 -kos:1,200,2000,0.5,0 -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -epp:0 -kf:1,0,100,0.2,0 -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -efl:4000.00 -ea:120.00 -efb:2000.00,4000.00 -kl:1.00,200.00,8000.00,50.00 -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + 
		   " -f:16,1,44100 -a:bus1 -i rtnull  -eac:0,2 " +  
		   " -f:16,1,44100 -a:bus2 -i foo.wav -erc:1,2 -eac:0,1 " + 
		   " -a:bus1,bus2  -o loop,1 " +
		   " -f:16,2,44100 -a:bus3 " +
		   " -i loop,1 -f:16,1,44100 " +
		   " -o null " +
		   " -epp:50 -erm:1" +
		   " -t:5 2>/dev/null >/dev/null");

  string many_chains (ecasound_exec);
  many_chains += " -a:";
  for(int n = 0; n < 128; n++) {
    many_chains += kvu_numtostr(n);
    if (n != 128) many_chains += ",";
  }
  many_chains += " -i:" + input_file;
  many_chains += " -o:null -t:5 2>/dev/null >/dev/null";
  eci_execute_test(many_chains);

  ECA_TEST_SUCCESS();
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
