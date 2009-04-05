// ------------------------------------------------------------------------
// samplebuffer_test.h: Unit test for SAMPLE_BUFFER
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

#include "samplebuffer.h"
#include "eca-test-case.h"

using namespace std;

/* ignore one bit errors in significand */
static const SAMPLE_BUFFER::sample_t diff_threshold = 1.0 / (1 << 22);

static void fill_random_data(SAMPLE_BUFFER *sbuf)
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

static bool verify_content(SAMPLE_BUFFER& a, SAMPLE_BUFFER& b)
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
      float diff = fabs(a.buffer[ch][i] - b.buffer[ch][i]);
      if (diff > diff_threshold) {
	std::fprintf(stderr, "%s: error in sample ch%d[%d], diff %.03f\n",
		     __FILE__, ch, i, diff);
	return false;
      }
    }
  }

  return true;
}

/**
 * Unit test for SAMPLE_BUFFER
 */
class SAMPLE_BUFFER_TEST : public ECA_TEST_CASE {

protected:

  virtual string do_name(void) const { return("SAMPLE_BUFFER"); }
  virtual void do_run(void);

public:

  virtual ~SAMPLE_BUFFER_TEST(void) { }

private:

};

void SAMPLE_BUFFER_TEST::do_run(void)
{
  const int bufsize = 1024;
  const int channels = 12;
  const SAMPLE_BUFFER::sample_t multiplier = 100.1f;

  SAMPLE_BUFFER sbuf_orig (bufsize, channels);
  SAMPLE_BUFFER sbuf_ref (bufsize, channels);
  SAMPLE_BUFFER sbuf_test (bufsize, channels);

  cerr << "libecasound_tester: samplebuffer" << endl;

  fill_random_data(&sbuf_orig);

  sbuf_test.copy_all_content(sbuf_orig);
  sbuf_ref.copy_all_content(sbuf_orig);

  sbuf_test.multiply_by(multiplier);
  sbuf_ref.multiply_by_ref(multiplier);

  if (verify_content(sbuf_ref, sbuf_test) != true) {
    ECA_TEST_FAILURE("optimized multiple_by");
  }
}
