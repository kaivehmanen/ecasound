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

#include <string>

#include <cstdio>
#include <stdio.h> /* for AIX */
#include <time.h> /* ANSI-C: clock() */

#include "kvu_locks.h"
#include "kvu_numtostr.h"
#include "kvu_utils.h"

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
static int kvu_test_2(void);
static int kvu_test_3(void);
static void* kvu_test_1_helper(void* ptr);

static kvu_test_t kvu_funcs[] = { 
  kvu_test_1, 
  kvu_test_2, 
  kvu_test_3, 
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
    // if (!(m & 0xffff)) fprintf(stderr, "S");
    ++m;
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
 * Tests the ATOMIC_INTEGER class defined 
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
    // if (!(m & 0xffff)) fprintf(stderr, "M");
    ++m;
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

/**
 * Tests the string handling functions defined 
 * in kvu_utils.h. 
 */
static int kvu_test_2(void)
{
  ECA_TEST_ENTRY();

  if (kvu_string_icmp(" foo ", " fOo ") != true) {
    ECA_TEST_FAIL(1, "kvu_test_2 kvu_string_icmp"); 
  }

  std::vector<std::string> vec = kvu_string_to_tokens(" a foo string ");
  if (vec.size() != 3) {
    ECA_TEST_FAIL(2, "kvu_test_2 kvu_string_to_tokens (1)");
  }
  if (vec[2] != "string") {
    ECA_TEST_FAIL(3, "kvu_test_2 kvu_string_to_tokens (2)"); 
  }

  vec = kvu_string_to_tokens_quoted("a foo\\ string");
  if (vec.size() != 2) {
    ECA_TEST_FAIL(4, "kvu_test_2 kvu_string_to_tokens_quoted (1)");
  }
  if (vec[1] != "foo string") {
    ECA_TEST_FAIL(5, "kvu_test_2 kvu_string_to_tokens_quoted (2)"); 
  }

  const std::string argument ("-efoobarsouNd:arg1,arg2,arg3");

  if (kvu_get_argument_prefix(argument) != "efoobarsouNd") {
    ECA_TEST_FAIL(6, "kvu_test_2 kvu_get_argument_prefix"); 
  }

  if (kvu_get_argument_number(3, argument) != "arg3") {
    ECA_TEST_FAIL(7, "kvu_test_2 kvu_get_argument_number"); 
  }

  vec = kvu_get_arguments(argument);
  if (vec.size() != 3) {
    ECA_TEST_FAIL(8, "kvu_test_2 kvu_get_arguments (1)"); 
  }

  if (vec[2] != "arg3" || vec[0] != "arg1") {
    ECA_TEST_FAIL(9, "kvu_test_2 kvu_get_arguments (2)"); 
  }

  if (kvu_get_number_of_arguments(argument) != 3) {
    ECA_TEST_FAIL(10, "kvu_test_2 kvu_get_number_of_arguments"); 
  }

  if (kvu_string_search_and_replace("foo bar", 'f', 'b')
      != "boo bar") {
    ECA_TEST_FAIL(11, "kvu_test_2 kvu_string_search_and_replace"); 
  }

  ECA_TEST_SUCCESS();
}

/**
 * Tests the floating point to text conversion functions.
 */
static int kvu_test_3(void)
{
  ECA_TEST_ENTRY();

  /* 17 digits after decimal point */
  double foo = 0.12345678912345678;
  std::string foostr = kvu_numtostr(foo, 17);
  if (foostr != "0.12345678912345678") {
    // fprintf(stderr, "foo=%.17lf, res=%s.\n", foo, foostr.c_str());
    ECA_TEST_FAIL(1, "kvu_test_3 kvu_numtostr double"); 
  }

  /* 8 digits after decimal point */
  float bar = 0.12345678;
  std::string barstr = kvu_numtostr(bar, 8);
  if (barstr != "0.12345678") {
    // fprintf(stderr, "bar=%.8f, res=%s.\n", bar, barstr.c_str());
    ECA_TEST_FAIL(2, "kvu_test_3 kvu_numtostr float"); 
  }

  ECA_TEST_SUCCESS();
}
