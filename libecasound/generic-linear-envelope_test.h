// ------------------------------------------------------------------------
// generic-linear-envelope_tests.h: Unit test
// Copyright (C) 2011 Kai Vehmanen
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

#include "generic-linear-envelope.h"

#include "eca-logger.h"
#include "eca-test-case.h"

using namespace std;

/**
 * Unit test for GENERIC_LINEAR_ENVELOPE class.
 */
class GENERIC_LINEAR_ENVELOPE_TEST : public ECA_TEST_CASE {

protected:

  virtual string do_name(void) const { return("GENERIC_LINEAR_ENVELOPE"); }
  virtual void do_run(void);

public:

  virtual ~GENERIC_LINEAR_ENVELOPE_TEST(void) { }

private:
  bool roughly_same(float a, float b);
};

bool GENERIC_LINEAR_ENVELOPE_TEST::roughly_same(float a, float b)
{ 
  const int significant_bits = 24;
  const double diff_threshold = 1.0 / ((1 << significant_bits) - 1);
  double diff = std::fabs(a - b);
  if (diff > diff_threshold)
    return false;
  return true;
}

void GENERIC_LINEAR_ENVELOPE_TEST::do_run(void)
{
  GENERIC_LINEAR_ENVELOPE f;

  bool verbose = false;
  if (getenv("V") != NULL)
    verbose = true;

  f.set_parameter(1, 4);
  f.set_parameter(2, 40.0);// point1
  f.set_parameter(3, 0.1);
  f.set_parameter(4, 100); // point2
  f.set_parameter(5, 0.5);
  f.set_parameter(6, 200); // point3
  f.set_parameter(7, 1);
  f.set_parameter(8, 500); // point4
  f.set_parameter(9, 0.9);
  f.set_parameter(10, 0);  // invalid param
  f.set_parameter(11, 0);  // invalid param
  f.init();

  if (roughly_same(f.value(-10.0), 0.1) != true)
    ECA_TEST_FAILURE("at pos -10.0s (invalid)");
  if (roughly_same(f.value(0.0), 0.1) != true)
    ECA_TEST_FAILURE("at pos 0.0s (start)");
  if (roughly_same(f.value(10.0), 0.1) != true)
    ECA_TEST_FAILURE("at pos 10.0s");
  if (roughly_same(f.value(40.0), 0.1) != true)
    ECA_TEST_FAILURE("at pos 40.0s");

  if (f.value(250) <= 0.9 || f.value(250) >= 1.0)
    ECA_TEST_FAILURE("at pos 250.0s");
  if (f.value(251) <= 0.9 || f.value(251) >= 1.0)
    ECA_TEST_FAILURE("at pos 251.0s");
  if (f.value(60) <= 0.1 || f.value(60) >= 0.5)
    ECA_TEST_FAILURE("at pos 60.0s");
  if (f.value(250) <= 0.9 || f.value(250) >= 1.0)
    ECA_TEST_FAILURE("at pos 250.0s");

  if (f.value(60) <= 0.1 || f.value(60) >= 0.5)
    ECA_TEST_FAILURE("at pos 60.0s");
  if (roughly_same(f.value(10.0), 0.1) != true)
    ECA_TEST_FAILURE("retest pos 10.0s");
  if (f.value(60) <= 0.1 || f.value(60) >= 0.5)
    ECA_TEST_FAILURE("retest pos 60.0s");

  if (roughly_same(f.value(2000.0), 0.9) != true)
    ECA_TEST_FAILURE("at pos 2000.0s (end)");
}
