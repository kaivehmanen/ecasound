// ------------------------------------------------------------------------
// audiofx_amplitu_test.h: Unit tests for EFFECT_AMPLIFY* classes
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

#include <string>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "kvu_dbc.h"
#include "kvu_inttypes.h"

#include "audiofx_amplitude.h"
#include "eca-test-case.h"

using namespace std;

/* ignore one bit errors in significand */
static const uint32_t my_diff_threshold_ints = 1;

static void my_fill_random_data(SAMPLE_BUFFER *sbuf)
{
  std::srand(time(0));
  int ch_count = sbuf->number_of_channels();
  int i_count = sbuf->length_in_samples();

  for (int ch = 0; ch < ch_count; ch++) {
    SAMPLE_BUFFER::sample_t *buf = sbuf->buffer[ch];
    for (int i = 0; i < i_count; i++) {
      int foo = std::rand();
      assert(sizeof(SAMPLE_BUFFER::sample_t) <= sizeof(foo));
      std::memcpy(buf, &foo, sizeof(SAMPLE_BUFFER::sample_t));
    }
  }
}

static bool my_verify_content(SAMPLE_BUFFER& a, SAMPLE_BUFFER& b)
{
  if (a.number_of_channels() !=
      b.number_of_channels())
    return false;

  if (a.length_in_samples() !=
      b.length_in_samples())
    return false;
  
  int ch_count = a.number_of_channels();
  int i_count = a.length_in_samples();

  for (int ch = 0; ch < ch_count; ch++) {
    for (int i = 0; i < i_count; i++) {
      if (a.buffer[ch][i] != b.buffer[ch][i]) {
	assert(sizeof(SAMPLE_BUFFER::sample_t) == sizeof(uint32_t));
	uint32_t diff = std::labs(*reinterpret_cast<uint32_t*>(&a.buffer[ch][i]) -
				  *reinterpret_cast<uint32_t*>(&b.buffer[ch][i]));
	if (diff > my_diff_threshold_ints) {
	    std::fprintf(stderr, "%s: ERROR sample ch%d[%d], diff %d (%.03f<->%.03f)\n",
			 __FILE__, ch, i, diff, a.buffer[ch][i], b.buffer[ch][i]);
	    return false;
	}
      }
    }
  }

  return true;
}

/**
 * Unit test for SAMPLE_BUFFER
 */
class EFFECT_AMPLIFY_TEST : public ECA_TEST_CASE {

protected:

  virtual string do_name(void) const { return("EFFECT_AMPLIFY"); }
  virtual void do_run(void);

public:

  virtual ~EFFECT_AMPLIFY_TEST(void) { }

private:

};

void EFFECT_AMPLIFY_TEST::do_run(void)
{
  const int bufsize = 1024;
  const int channels = 12;
  const SAMPLE_BUFFER::sample_t multiplier = 112.1f;

  std::fprintf(stdout, "%s: tests for %s class\n",
	       name().c_str(), __FILE__);

  /* case: multiply_by */
  {
    std::fprintf(stdout, "%s: multiply_by\n",
		 __FILE__);
    SAMPLE_BUFFER sbuf_test (bufsize, channels);
    SAMPLE_BUFFER sbuf_ref (bufsize, channels);

    EFFECT_AMPLIFY amp_test;
    EFFECT_AMPLIFY amp_ref;
    
    my_fill_random_data(&sbuf_ref);
    sbuf_test.copy_all_content(sbuf_ref);

    amp_test.init(&sbuf_test);
    amp_ref.init(&sbuf_ref);
    
    amp_test.set_parameter(1, multiplier);
    amp_ref.set_parameter(1, multiplier);

    amp_test.process();
    amp_ref.process_ref();
    
    if (my_verify_content(sbuf_ref, sbuf_test) != true) {
      ECA_TEST_FAILURE("optimized EFFECT_AMPLIFY");
    }
  }
}
