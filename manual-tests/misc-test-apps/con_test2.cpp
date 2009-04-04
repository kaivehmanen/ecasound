// ------------------------------------------------------------------------
// Copyright (C) 2009 Kai Vehmanen
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
#include <cstdio>
#include <string>
#include <cstdlib>

#include "ecatestsuite.h"

using namespace std;

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
  string ecasound_exec ("./ecasound_test");
  const char *ecasound_env = getenv("ECASOUND");
  if (ecasound_env)
    ecasound_exec = string(ecasound_env);

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
