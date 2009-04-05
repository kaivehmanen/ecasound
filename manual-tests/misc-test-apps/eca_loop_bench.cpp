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

#include "eca-version.h"
#include "samplebuffer.h"
#include "audiofx_amplitude.h"

#include "kvu_procedure_timer.h"
#include "ecatestsuite.h"

int test_sbuf_make_silent(void);
int test_sbuf_copy_ops(void);
int test_sbuf_constructor(void);
int test_sbuf_mix(void);
int test_sbuf_iter(void);

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
  res += test_sbuf_iter();

  return res;
}

void helper_print_one_result(const char *casename, const PROCEDURE_TIMER& t1, int loops, int bsize)
{
  double per_loop = t1.last_duration_seconds() / loops;

  std::printf("\t%-20.20s:\t%.03fms (%.03fus/loop, %.04f%% CPU@48kHz)\n", 
	      casename,
	      t1.last_duration_seconds() * 1000.0,
	      per_loop * 1000000.0,
	      (per_loop / (((double)bsize) / 48000.0)));
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

  helper_print_one_result("make_silent_range", t1, loops, bufsize);

  /* note: make sure code is paged in */
  sbuf.make_silent_range(0, bufsize);
  t1.reset();

  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf.make_silent();
  }
  t1.stop();
  
  helper_print_one_result("make_silent", t1, loops, bufsize);
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

  helper_print_one_result("copy_all_content", t1, loops, bufsize);

  /* note: make sure code is paged in */
  sbuf_a.copy_matching_channels(sbuf_b);
  t1.reset();

  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy_matching_channels(sbuf_b);
  }
  t1.stop();

  helper_print_one_result("copy_matching_channels", t1, loops, bufsize);

#else

  /* note: make sure code is paged in */
  sbuf_a.copy(sbuf_b);

  t1.reset();
  t1.start();
  for(int n = 0; n < loops; n++) {
    sbuf_a.copy(sbuf_b);
  }
  t1.stop();

  helper_print_one_result("copy (v21-lib)", t1, loops, bufsize);

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

  helper_print_one_result("constructor", t1, loops, bufsize);
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
  {
    sbuf_a.divide_by(1.23456789);
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.divide_by(1.23456789);
    }
    t1.stop();
    
    helper_print_one_result("divide_by", t1, loops, bufsize);
  }

  /* case 1b */
  {
#if LIBECASOUND_VERSION >= 22
    sbuf_a.divide_by_ref(1.23456789);
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.divide_by_ref(1.23456789);
    }
    t1.stop();
    
    helper_print_one_result("divide_by_ref", t1, loops, bufsize);
#endif
  }

  /* case 2 */
  {
    sbuf_a.add_with_weight(sbuf_b, 2);
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.add_with_weight(sbuf_b, 2);
    }
    t1.stop();
    
    helper_print_one_result("add_with_weight", t1, loops, bufsize);
  }

  /* case 3 */
  {
#if LIBECASOUND_VERSION >= 22
    sbuf_a.add_matching_channels(sbuf_b);
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.add_matching_channels(sbuf_b);
    }
    t1.stop();
    
    helper_print_one_result("add_matching_channels", t1, loops, bufsize);

    /* case 3b */
    sbuf_a.add_matching_channels_ref(sbuf_b);
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.add_matching_channels_ref(sbuf_b);
    }
    t1.stop();
    
    helper_print_one_result("add_matching_ch...ref", t1, loops, bufsize);

#else
    sbuf_a.add(sbuf_b);
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.add(sbuf_b);
    }
    t1.stop();

    helper_print_one_result("add (v21-lib)", t1, loops, bufsize);
#endif
  }

  /* case 4 */
  {
    sbuf_a.limit_values();
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.limit_values();
    }
    t1.stop();
    
    helper_print_one_result("limit_values", t1, loops, bufsize);

    /* case 4b */
#if LIBECASOUND_VERSION >= 22
    sbuf_a.limit_values_ref();
    t1.reset();
    
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.limit_values_ref();
    }
    t1.stop();
    
    helper_print_one_result("limit_values_ref", t1, loops, bufsize);
#endif
  }
}

int test_sbuf_iter(void)
{
  const int loops = 10000;
  const int bufsize = 1024;
  const int channels = 12;
  const SAMPLE_BUFFER::sample_t multiplier = 100.1f;

  std::printf("sbuf_iter with %d loops (bufsize=%d, ch=%d):\n", 
	      loops, bufsize, channels);

  PROCEDURE_TIMER t1;
  SAMPLE_BUFFER sbuf_a (bufsize, channels);
  EFFECT_AMPLIFY amplify (multiplier);

  /* case 1 */
  {
    amplify.init(&sbuf_a);
    amplify.process();
    
    t1.reset();
    t1.start();
    for(int n = 0; n < loops; n++) {
      amplify.process();
    }
    t1.stop();
  
    helper_print_one_result("effect_amplify", t1, loops, bufsize);
  }

  /* case 2 */
  {
    int ch_count = sbuf_a.number_of_channels();
    int i_count = sbuf_a.length_in_samples();
    t1.reset();
    t1.start();
    for(int n = 0; n < loops; n++) {
      for (int ch = 0; ch < ch_count; ch++) {
	SAMPLE_BUFFER::sample_t *buf = sbuf_a.buffer[ch];
	for (int i = 0; i < i_count; i++) {
	  buf[i] *= multiplier;
	}
      }
    }
    t1.stop();
  
    helper_print_one_result("amplify_memops", t1, loops, bufsize);
  }

  /* case 3+4 */
  {
#if LIBECASOUND_VERSION >= 22
    sbuf_a.multiply_by(multiplier);
    t1.reset();
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.multiply_by(multiplier);
    }
    t1.stop();
    helper_print_one_result("amplify_sbuf", t1, loops, bufsize);

    sbuf_a.multiply_by_ref(multiplier);
    t1.reset();
    t1.start();
    for(int n = 0; n < loops; n++) {
      sbuf_a.multiply_by_ref(multiplier);
    }
    t1.stop();
    helper_print_one_result("amplify_sbuf_ref", t1, loops, bufsize);

#endif
  }

}
