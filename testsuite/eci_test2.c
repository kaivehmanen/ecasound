#include <stdio.h>
#include <stdlib.h>

#include "ecasoundc.h"
#include "ecatestsuite.h"

int main(int argc, char *argv[]) {
  eci_handle_t handle;

  ECA_TEST_ENTRY();

  handle = eci_init_r();
  eci_cleanup_r(handle);
  handle = eci_init_r();
  eci_cleanup_r(handle);

  ECA_TEST_SUCCESS();
}
