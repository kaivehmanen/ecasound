#include <stdio.h>

#include "ecasoundc.h"
#include "ecatestsuite.h"

int main(int argc, char *argv[]) {
  int count;

  ECA_TEST_ENTRY();

  eci_init();
  eci_command("cs-add default");
  eci_command("cs-selected");
  if (memcmp(eci_last_string(), "default", 7) != 0) {
    ECA_TEST_FAIL(1, "chainsetup addition or selection failed");
  }

  eci_command("cs-set-length 15");

  eci_command("c-add 1,2");
  eci_command("c-select-all");
  eci_command("c-selected");
  count = eci_last_string_list_count();
  if (count != 2) {
    ECA_TEST_FAIL(2, "chain count mismatch");
  }

  eci_command("-i:foo.wav");
  eci_command("-o:null");
  eci_command("cs-connect");
  eci_command("cs-connected");
  if (memcmp(eci_last_string(), "default", 7) != 0) {
    ECA_TEST_FAIL(3, "can't connect chainsetup");
  }
  
  eci_command("run");
  eci_cleanup();

  ECA_TEST_SUCCESS();
}
