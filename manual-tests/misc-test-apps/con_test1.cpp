#include <iostream>
#include <sstream>
#include <cstdio>
#include <string>
#include <cstdlib>

#include "ecatestsuite.h"

using namespace std;

static const string input_file("foo.wav");

static void eci_execute_test(const string& cmd);

int main(int argc, char *argv[]) {
  ECA_TEST_ENTRY();

  string ecasound_exec ("./ecasound_test");
  const char *ecasound_env = getenv("ECASOUND");
  if (ecasound_env)
    ecasound_exec = string(ecasound_env);

  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -gc:1,0.5 -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -i:" + input_file + " -o:null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -d:255 -z:db -r -z:nointbuf -i:" + input_file + " -o:null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + "  -i:" + input_file + " -o:null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -epp:0 -kf:1,0,100,0.2,0,1 -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -efl:0 -kl:1,400,4000,10  -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -efl:0 -kl2:1,400,4000,5,10  -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -efl:0 -klg:1,400,4000,4,0,0.0,10,1.0,20,0.0,30,1.0  -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -efl:0 -kog:1,400,4000,0.2,1,2,0,1,0.3,1,0.6,0  -o null -t:5 2>/dev/null >/dev/null");
  eci_execute_test(ecasound_exec + " -a:1 -i " + input_file + " -efl:400 -kos:1,200,2000,0.5,0 -o null -t:5 2>/dev/null >/dev/null");
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
    std::stringstream tmp;
    tmp << n;
    many_chains += tmp.str();
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
