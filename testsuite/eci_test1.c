#include <stdio.h>

#include "ecasoundc.h"
#include "ecatestsuite.h"

int main(int argc, char *argv[]) {
  ECA_TEST_ENTRY();

  eci_init();
  eci_cleanup();
  eci_init();
  eci_cleanup();
 
  ECA_TEST_SUCCESS();
}
