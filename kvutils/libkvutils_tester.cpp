// ------------------------------------------------------------------------
// libkvutils_tester.cpp: Runs a set of libkvutils unit tests.
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

#include <cstdio>
#include <stdio.h> /* for AIX */
#include <time.h> /* ANSI-C: clock() */

#include "kvu_locks.h"

/* --------------------------------------------------------------------- 
 * Options
 */

#define VERBOSE

/* --------------------------------------------------------------------- 
 * Test util macros
 */

#ifdef VERBOSE
#define ECA_TEST_ENTRY()   printf("\n%s:%d - Test started", __FILE__, __LINE__)
#define ECA_TEST_SUCCESS() printf("\n%s:%d - Test passed\n", __FILE__, __LINE__); return 0
#define ECA_TEST_FAIL(x,y) printf("\n%s:%d - Test failed: \"%s\"\n", __FILE__, __LINE__, y); return x
#define ECA_TEST_CASE()    printf("."); fflush(stdout)
#else
#define ECA_TEST_ENTRY()   ((void) 0)
#define ECA_TEST_SUCCESS() return 0
#define ECA_TEST_FAIL(x,y) return x
#endif

/* --------------------------------------------------------------------- 
 * Type definitions
 */

typedef int (*kvu_test_t)(void);

/* --------------------------------------------------------------------- 
 * Test case declarations
 */

static int kvu_test_1(void);
static void* kvu_test_1_helper(void* ptr);

static kvu_test_t kvu_funcs[] = { 
  kvu_test_1, 
  NULL 
};

/* --------------------------------------------------------------------- 
 * Funtion definitions
 */

int main(int argc, char *argv[])
{
  int n, failed = 0;
  
  for(n = 0; kvu_funcs[n] != NULL; n++) {
    int ret = kvu_funcs[n]();
    if (ret != 0) {
      ++failed;
    }
  }

  return failed;
}

#define KVU_TEST_1_ROUNDS 5
// #define KVU_TEST_1_ROUNDS 60

static void* kvu_test_1_helper(void* ptr)
{
  ATOMIC_INTEGER* i = (ATOMIC_INTEGER*)ptr;

  int stop_after = KVU_TEST_1_ROUNDS * CLOCKS_PER_SEC / 5;
  clock_t prev, now = clock();

  for(int n = 0, m = 0; n < stop_after;) {
    // if (!(++m & 0xffff)) fprintf(stderr, "S");
    int j = i->get();
    if (j < 0) {
      ++j;
      i->set(j);
    }
    j = i->get();
    if (j > 0) { ECA_TEST_FAIL((void*)3, "kvu_test_1_helper access error (1)"); }
    if (j < -1) { ECA_TEST_FAIL((void*)3, "kvu_test_1_helper access error (2)"); }

    prev = now;
    now = clock();
    if (prev > now) 
      n += prev - now;
    else 
      n += now - prev;
  }

  return 0;
}

/**
 * Test for the ATOMIC_INTEGER class defined 
 * in kvu_locks.h. 
 */
static int kvu_test_1(void)
{
  ECA_TEST_ENTRY();

  ATOMIC_INTEGER i (0);

  pthread_t thread;
  pthread_create(&thread, NULL, kvu_test_1_helper, (void*)&i);

  int stop_after = KVU_TEST_1_ROUNDS * CLOCKS_PER_SEC;
  clock_t prev, now = clock();
  
  for(int n = 0, m = 0; n < stop_after;) {
    // if (!(++m & 0xffff)) fprintf(stderr, "M");
    int j = i.get();
    if (j < 0) {
      ++j;
      i.set(j);
    }
    else {
      --j;
      i.set(j);
    }

    j = i.get();
    if (j > 0) { ECA_TEST_FAIL(3, "kvu_test_1 access error (3)"); }
    if (j < -1) { ECA_TEST_FAIL(3, "kvu_test_1 access error (4)"); }

    prev = now;
    now = clock();
    if (prev > now) 
      n += prev - now;
    else 
      n += now - prev;
  }

  ECA_TEST_SUCCESS();
}

