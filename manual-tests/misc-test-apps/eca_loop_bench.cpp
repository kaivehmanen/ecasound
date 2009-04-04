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

#include "samplebuffer.h"
#include "eca-version.h"

#include "kvu_procedure_timer.h"
#include "ecatestsuite.h"

int test_sbuf_make_silent(void);
int test_sbuf_copy_ops(void);
int test_sbuf_constructor(void);
int test_sbuf_mix(void);

int main(int argc, char *argv[])
{
  int res = 0;

  std::printf("--------------------------------------------------------\n"
	      "Testing with libecasound *** v%s *** (%s).\n",
	      ecasound_library_version, __FILE__);

  res += test_sbuf_make_silent();
  res += test_sbuf_copy_ops();
  res += test_sbuf_constructor();
  res += test_sbuf_mix();

  return res;
}

void helper_print_one_result(const char *casename, const PROCEDURE_TIMER& t1, int loops)
{
  std::printf("\t%-20.20s:\t%.03fms (%.03fus/loop)\n", 
	      casename,
	      t1.last_duration_seconds() * 1000.0,
	      t1.last_duration_seconds() * 1000000.0 / loops);
}

int test_sbuf_make_silent(void)
{
  const int loops = 10000;
  const int bufsize = 1024;
  const int channels = 12;

  std::printf("sbuf_make_silent with %d loops (bufsize=%d, ch=%d):\n", 
	      loops, 1024, 12);

  PROCEDURE_TIMER t1;
  SAMPLE_BUFFER sbuf (bufsize, channels);

  /* note: make sure code is paged in */
  sbuf.make_silent();
  
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf.make_silent_range(0, bufsize);
  }
  t1.stop();

  helper_print_one_result("make_silent_range", t1, loops);

  /* note: make sure code is paged in */
  sbuf.make_silent_range(0, bufsize);
  t1.reset();

  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf.make_silent();
  }
  t1.stop();
  
  helper_print_one_result("make_silent", t1, loops);
}  

int test_sbuf_copy_ops(void)
{
  const int loops = 10000;
  const int bufsize = 1024;
  const int channels = 12;

  PROCEDURE_TIMER t1;
  SAMPLE_BUFFER sbuf_a (bufsize, channels);
  SAMPLE_BUFFER sbuf_b (bufsize, channels);

  std::printf("sbuf_copy_ops with %d loops (bufsize=%d, ch=%d):\n", 
	      loops, 1024, 12);

#if LIBECASOUND_VERSION >= 22
  /* note: make sure code is paged in */
  sbuf_a.copy_all_content(sbuf_b);

  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy_all_content(sbuf_b);
  }
  t1.stop();

  helper_print_one_result("copy_all_content", t1, loops);

  /* note: make sure code is paged in */
  sbuf_a.copy_matching_channels(sbuf_b);
  t1.reset();

  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy_matching_channels(sbuf_b);
  }
  t1.stop();

  helper_print_one_result("copy_matching_channels", t1, loops);

#else

  /* note: make sure code is paged in */
  sbuf_a.copy(sbuf_b);

  t1.reset();
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy(sbuf_b);
  }
  t1.stop();

  helper_print_one_result("copy (v21-lib)", t1, loops);

#endif
}  

int test_sbuf_constructor(void)
{
  const int loops = 10000;
  const int bufsize = 1024;
  const int channels = 12;

  PROCEDURE_TIMER t1;
  SAMPLE_BUFFER sbuf_a (bufsize, channels);

  std::printf("sbuf constructor with %d loops (bufsize=%d, ch=%d):\n", 
	      loops, 1024, 12);

  /* note: make sure code is paged in */

  t1.start();
  for(int n = 0; n < loops; n++) {
    SAMPLE_BUFFER sbuf_b (bufsize, channels);
  }
  t1.stop();

  helper_print_one_result("constructor", t1, loops);
}  

int test_sbuf_mix(void)
{
  const int loops = 10000;
  const int bufsize = 1024;
  const int channels = 12;

  std::printf("sbuf_mix with %d loops (bufsize=%d, ch=%d):\n", 
	      loops, bufsize, channels);

  PROCEDURE_TIMER t1;
  SAMPLE_BUFFER sbuf_a (bufsize, channels);
  SAMPLE_BUFFER sbuf_b (bufsize, channels);

  /* case 1 */
  sbuf_a.divide_by(1.23456789);
  t1.reset();
  
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.divide_by(1.23456789);
  }
  t1.stop();

  helper_print_one_result("divide_by", t1, loops);

  /* case 2 */

  sbuf_a.add_with_weight(sbuf_b, 2);
  t1.reset();
  
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.add_with_weight(sbuf_b, 2);
  }
  t1.stop();

  helper_print_one_result("add_with_weight", t1, loops);
}  
