// ------------------------------------------------------------------------
// eca-sample-conversion_test.h: Unit test for sample conversion routines
// Copyright (C) 2002,2010 Kai Vehmanen
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

#include <cstdlib>
#include <string>

#include "eca-sample-conversion.h"
#include "kvu_numtostr.h"

#include "eca-logger.h"
#include "eca-test-case.h"

using namespace std;

/**
 * Unit test for ECA_SAMPLE-CONVERSION.
 */
class ECA_SAMPLE_CONVERSION_TEST : public ECA_TEST_CASE {

protected:

  virtual string do_name(void) const { return("Unit test for eca-sample-conversion"); }
  virtual void do_run(void);

public:

  virtual ~ECA_SAMPLE_CONVERSION_TEST(void) { }
};

void ECA_SAMPLE_CONVERSION_TEST::do_run(void)
{
  double dmax = 1.0f;
  double dmin = -1.0f;
  double dzero = 0.0f;

  bool verbose = false;
  if (getenv("V") != NULL)
    verbose = true;

  uint8_t u8min = eca_sample_convert_float_to_u8(dmin);
  if (verbose) cout << "-unity to u8 =" << (int)u8min << "\n";
  if (abs((int)(u8min - UINT8_MIN)) > 1) {
    ECA_TEST_FAILURE("abs(u8min - UINT8_MIN) > 1");
  }

  uint8_t u8max = eca_sample_convert_float_to_u8(dmax);
  if (verbose) cout << "unity to u8 =" << (int)u8max << "\n";
  if (abs((int)(u8max - UINT8_MAX)) > 1) {
    ECA_TEST_FAILURE("abs(u8max - UINT8_MAX) > 1");
  }

  uint8_t u8zero = eca_sample_convert_float_to_u8(dzero);
  if (verbose) cout << "zero to u8 =" << (int)u8zero << "\n";
  if (u8zero != 128) {
    ECA_TEST_FAILURE("u8zero != 128");
  }

  int16_t s16min = eca_sample_convert_float_to_s16(dmin);
  if (verbose) cout << "-unity to s16 =" << s16min << "\n";
  if (abs(s16min - INT16_MIN) > 1) {
    ECA_TEST_FAILURE("abs(s16min - INT16_MIN) > 1");
  }

  int16_t s16max = eca_sample_convert_float_to_s16(dmax);
  if (verbose) cout << "unity to s16 =" << s16max << "\n";
  if (abs(s16max - INT16_MAX) > 1) {
    ECA_TEST_FAILURE("abs(s16max - INT16_MAX) > 1");
  }

  int16_t s16zero = eca_sample_convert_float_to_s16(dzero);
  if (verbose) cout << "zero to s16 =" << s16zero << "\n";
  if (s16zero != 0) {
    ECA_TEST_FAILURE("s16zero != 0");
  }

  int32_t s32min = eca_sample_convert_float_to_s32(dmin);
  if (verbose) cout << "-unity to s32 =" << s32min << "\n";
  if (labs(s32min - INT32_MIN) > 1) {
    ECA_TEST_FAILURE("labs(s32min - INT32_MIN) > 1");
  }
  
  int32_t s32max = eca_sample_convert_float_to_s32(dmax);
  if (verbose) cout << "unity to s32 =" << s32max << "\n";
  if (labs(s32max - INT32_MAX) > 1) {
    ECA_TEST_FAILURE("labs(s32max - INT32_MAX) > 1");
  }

  int32_t s32zero = eca_sample_convert_float_to_s32(dzero);
  if (verbose) cout << "zero to s32 =" << s16zero << "\n";
  if (s32zero != 0) {
    ECA_TEST_FAILURE("s32zero != 0");
  }

  float cur = eca_sample_convert_u8_to_float(UINT8_MIN);
    if (verbose) cout << "u8min to float =" << cur << "\n";
  if (cur > -1.0f || cur < -1.0f) {
    ECA_TEST_FAILURE("to_float: u8min");
  }

  cur = eca_sample_convert_u8_to_float(UINT8_MAX);
  if (verbose) cout << "u8max to float =" << cur << "\n";
  if (cur > 1.0f) {
    ECA_TEST_FAILURE("to_float: u8max");
  }

  cur = eca_sample_convert_s16_to_float(INT16_MIN);
  if (verbose) cout << "s16min to float =" << cur << "\n";
  if (cur > -1.0f || cur < -1.0f) {
    if (verbose) cout << "s16min to float: WARNING, suspect value\n";
    // ECA_TEST_FAILURE("to_float: s16min");
  }

  cur = eca_sample_convert_s16_to_float(INT16_MIN + 1);
  if (verbose) cout << "s16min+1 to float =" << cur << "\n";
  if (cur > -1.0f || cur < -1.0f) {
    if (verbose) cout << "s16min+1 to float: WARNING, suspect value\n";
    // ECA_TEST_FAILURE("to_float: s16min");
  }

  cur = eca_sample_convert_s16_to_float(INT16_MAX);
  if (verbose) cout << "s16max to float =" << cur << "\n";
  if (cur > 1.0f) {
    ECA_TEST_FAILURE("to_float: s16max");
  }

  cur = eca_sample_convert_s32_to_float(INT32_MIN + 1);
  if (verbose) cout << "s32min+1 to float =" << cur << "\n";
  if (cur > -1.0f || cur < -1.0f) {
    ECA_TEST_FAILURE("to_float: s32min+1");
  }

  cur = eca_sample_convert_s32_to_float(INT32_MIN);
  if (verbose) cout << "s32min to float =" << cur << "\n";
  if (cur > -1.0f || cur < -1.0f) {
    ECA_TEST_FAILURE("to_float: s32min");
  }

  cur = eca_sample_convert_s32_to_float(INT32_MAX);
  if (verbose) cout << "s32max to float =" << cur << "\n";
  if (cur < 1.0f || cur > 1.0f) {
    ECA_TEST_FAILURE("to_float: s32max");
  }

#define S16INTFLOATINT(y) \
  { \
    float mid = eca_sample_convert_s16_to_float(y); \
    int16_t res = eca_sample_convert_float_to_s16(mid); \
    if (verbose) cout << "s16 ifi " << (y) << " to " << mid << " to " << res << endl; \
    if (res != (y)) { \
      if (verbose) cout << "s16 ifi: WARNING, suspect value\n";	\
      /* report_failure(__FILE__, __LINE__, "s16 ifi" + kvu_numtostr(y)); */ \
    } \
  }

  S16INTFLOATINT(INT16_MAX);
  S16INTFLOATINT(-INT16_MAX);
  S16INTFLOATINT(INT16_MIN);
  S16INTFLOATINT(0);
  S16INTFLOATINT(1);
  S16INTFLOATINT(1 << 8);
  S16INTFLOATINT(-1);
  S16INTFLOATINT(-1 << 8);

#define S32INTFLOATINT(y) \
  { \
    float mid = eca_sample_convert_s32_to_float(y); \
    int32_t res = eca_sample_convert_float_to_s32(mid); \
    cout << "s32 ifi " << (y) << " to " << mid << " to " << res << endl; \
    if (res != (y)) { \
      if (verbose) cout << "s32 ifi: WARNING, suspect value\n";		\
      /*report_failure(__FILE__, __LINE__, "s32 ifi " + kvu_numtostr(y)); */ \
    } \
  }

  S32INTFLOATINT(INT32_MAX);
  S32INTFLOATINT(-INT32_MAX);
  S32INTFLOATINT(INT32_MIN);
  S32INTFLOATINT(0);
  S32INTFLOATINT(1);
  S32INTFLOATINT(1 << 8);
  S32INTFLOATINT(1 << 16);
  S32INTFLOATINT(-1 << 20);
  S32INTFLOATINT(-1);
  S32INTFLOATINT(-1 << 8);
  S32INTFLOATINT(-1 << 16);
  S32INTFLOATINT(-1 << 20);
}
