/* ------------------------------------------------------------------------
 * libecasound_tester.c: Runs a set of ECI C unit tests.
 * Copyright (C) 2002 Kai Vehmanen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 * ------------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ecasoundc.h"
/* #include "eca-control-interface.h" */

/* FIXME: cannot be run on a clean-build as ecasound is not yet
 *        installed */

/* --------------------------------------------------------------------- 
 * Options
 */

#define VERBOSE

/* --------------------------------------------------------------------- 
 * Test util macros
 */

#ifdef VERBOSE
#define ECA_TEST_ENTRY()   do { printf("\n%s:%d - Test started", __FILE__, __LINE__); } while(0)
#define ECA_TEST_SUCCESS() do { printf("\n%s:%d - Test passed\n", __FILE__, __LINE__); return 0; } while(0)
#define ECA_TEST_FAIL(x,y) do { printf("\n%s:%d - Test failed: \"%s\"\n", __FILE__, __LINE__, y); return x; } while(0)
#define ECA_TEST_CASE()    do { printf("."); fflush(stdout); } while(0)
#else
#define ECA_TEST_ENTRY()   ((void) 0)
#define ECA_TEST_SUCCESS() return 0
#define ECA_TEST_FAIL(x,y) return x
#endif

/* --------------------------------------------------------------------- 
 * Type definitions
 */

typedef int (*eci_test_t)(void);

/* --------------------------------------------------------------------- 
 * Test case declarations
 */

static int eci_test_1(void);
static int eci_test_2(void);
static int eci_test_3(void);
static int eci_test_4(void);
static int eci_test_5(void);
/* static int eci_test_6(void); */

static eci_test_t eci_funcs[] = { 
  eci_test_1, 
  eci_test_2, 
  eci_test_3, 
  eci_test_4, 
  eci_test_5, 
  /* eci_test_6,  */
  NULL 
};

/* --------------------------------------------------------------------- 
 * Funtion definitions
 */

int main(int argc, char *argv[])
{
  int n, failed = 0;
  
#if NDEBUG
  putenv("ECASOUND=../ecasound/ecasound");
#else
  putenv("ECASOUND=../ecasound/ecasound_debug");
#endif

  for(n = 0; eci_funcs[n] != NULL; n++) {
    int ret = eci_funcs[n]();
    if (ret != 0) {
      ++failed;
    }
  }

  return failed;
}

static int eci_test_1(void)
{
  ECA_TEST_ENTRY();

  eci_init();
  eci_cleanup();
  eci_init();
  eci_cleanup();
 
  ECA_TEST_SUCCESS();
}

static int eci_test_2(void)
{
  eci_handle_t handle;

  ECA_TEST_ENTRY();

  handle = eci_init_r();
  eci_cleanup_r(handle);
  handle = eci_init_r();
  eci_cleanup_r(handle);

  ECA_TEST_SUCCESS();
}

static int eci_test_3(void)
{
  int count;

  ECA_TEST_ENTRY();

  eci_init();
  eci_command("cs-add default");
  eci_command("cs-selected");
  if (memcmp(eci_last_string(), "default", 7) != 0) {
    ECA_TEST_FAIL(1, "chainsetup addition or selection failed");
  }

  eci_command("cs-set-length 5");

  eci_command("c-add 1,2");
  eci_command("c-select-all");
  eci_command("c-selected");
  count = eci_last_string_list_count();
  if (count != 2) {
    ECA_TEST_FAIL(2, "chain count mismatch");
  }

  eci_command("-i:null");
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

static int eci_test_4(void)
{
  eci_handle_t handle;

  ECA_TEST_ENTRY();

  handle = eci_init_r();
  if (handle == NULL) { ECA_TEST_FAIL(1, "init failed"); }

  eci_command_r(handle, "cs-add test_cs");
  eci_command_r(handle, "c-add test_c");

  eci_command_r(handle, "ai-add null");
  eci_command_r(handle, "ao-add null");

  eci_command_r(handle, "cs-connect");
  if (eci_error_r(handle) != 0) { ECA_TEST_FAIL(2, "cs-connect failed (1)"); }

  eci_command_r(handle, "cs-disconnect");
  eci_command_r(handle, "ai-iselect 1");
  eci_command_r(handle, "ai-remove");
  eci_command_r(handle, "ai-add rtnull");
  eci_command_r(handle, "cs-connect");
  if (eci_error_r(handle) != 0) { ECA_TEST_FAIL(3, "cs-connect failed (2)"); }

  eci_command_r(handle, "cs-disconnect");
  eci_command_r(handle, "ai-iselect 1");
  eci_command_r(handle, "ai-remove");
  eci_command_r(handle, "ai-add null");
  eci_command_r(handle, "ai-add null");
  eci_command_r(handle, "ai-add null");
  eci_command_r(handle, "c-add 1");
  eci_command_r(handle, "ai-add null");
  eci_command_r(handle, "cs-connect");
  if (eci_error_r(handle) == 0) { ECA_TEST_FAIL(3, "cs-connect succeeded when it should fail"); }

  eci_command_r(handle, "c-remove");
  eci_command_r(handle, "c-select test_c");
  eci_command_r(handle, "ai-iselect 4");
  eci_command_r(handle, "ai-remove");
  eci_command_r(handle, "ai-iselect 3");
  eci_command_r(handle, "ai-remove");
  eci_command_r(handle, "ai-iselect 2");
  eci_command_r(handle, "ai-remove");
  eci_command_r(handle, "cs-connect");
  if (eci_error_r(handle) != 0) { ECA_TEST_FAIL(3, "cs-connect failed (3)"); }

  eci_command_r(handle, "cs-disconnect");

  eci_cleanup_r(handle);
  
  ECA_TEST_SUCCESS();
}

static int eci_test_5(void)
{
  eci_handle_t handle;
  int count;

  ECA_TEST_ENTRY();

  handle = eci_init_r();
  if (handle == NULL) { ECA_TEST_FAIL(1, "init failed"); }

  eci_command_r(handle, "cs-remove");
  eci_command_r(handle, "cs-status");
  eci_command_r(handle, "cs-list");
  count = eci_last_string_list_count_r(handle);
  if (count != 0) { ECA_TEST_FAIL(1, "cs-list count not zero"); }

  eci_command_r(handle, "cs-add test_cs2");
  eci_command_r(handle, "cs-list");
  count = eci_last_string_list_count_r(handle);
  if (count != 1) { ECA_TEST_FAIL(2, "cs-list count not one"); }

  if (strncmp(eci_last_string_list_item_r(handle, 0), "test_cs2", 8) != 0) {
    ECA_TEST_FAIL(3, "cs name does not match");
  }

  eci_cleanup_r(handle);
  
  ECA_TEST_SUCCESS();
}

#if 0
static int eci_test_6(void)
{
  ECA_CONTROL_INTERFACE handle;
  int count;

  ECA_TEST_ENTRY();

  if (handle == NULL) { ECA_TEST_FAIL(1, "init failed"); }

  handle.command("cs-remove");
  handle.command("cs-status");
  handle.command( "cs-list");
  count = handle.last_string_list().size();
  if (count != 0) { ECA_TEST_FAIL(1, "cs-list count not zero"); }

  handle.command("cs-add test_cs2");
  handle.command("cs-list");
  count = handle.last_string_list().size;
  if (count != 1) { ECA_TEST_FAIL(2, "cs-list count not one"); }

  if (handle.last_string_list()[0] != "test_cs2") {
    ECA_TEST_FAIL(3, "cs name does not match");
  }

  ECA_TEST_SUCCESS();
}
#endif
