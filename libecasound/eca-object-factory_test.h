// ------------------------------------------------------------------------
// eca-object-factory_test.h: Unit test for ECA_OBJECT_FACTORY
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

#include "audioio-device.h"
#include "eca-object-factory.h"
#include "eca-object-map.h"
#include "eca-test-case.h"

using namespace std;

class ECA_OBJECT_FACTORY_TEST : public ECA_TEST_CASE {

protected:

  virtual string do_name(void) const { return("ECA_OBJECT_FACTORY"); }
  virtual void do_run(void);

public:

  virtual ~ECA_OBJECT_FACTORY_TEST(void) { }
};

void ECA_OBJECT_FACTORY_TEST::do_run(void)
{
  // FIXME: not finished!

  ECA_OBJECT_MAP& rt_map = ECA_OBJECT_FACTORY::audio_io_rt_map();
  const AUDIO_IO_DEVICE* adev = 
    dynamic_cast<const AUDIO_IO_DEVICE*>(rt_map.object("/deev/dsp"));
  if (adev == 0) {
    ECA_TEST_FAILURE("non-rt audio device in audio_io_rt_map");
  }
}
