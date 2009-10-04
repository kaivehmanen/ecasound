// ------------------------------------------------------------------------
// ecacompare.cpp: Compare two audio files sample by sample
// Copyright (C) 2009 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3 (see Ecasound Programmer's Guide)
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

#include <kvu_numtostr.h>
#include <audioio.h>
#include <samplebuffer.h>
#include <samplebuffer_functions.h>
#include <eca-object-factory.h>
#include <eca-logger.h>

int main(int argc, char *argv[])
{
  AUDIO_IO *a, *b;
  SAMPLE_BUFFER buf_a, buf_b;
  int buffersize = 1024;
  bool verbose = std::getenv("V") != 0 ? true : false;
 
  if (argc < 3) 
    return 1;

  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::errors, true);
  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::info, true);
  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::subsystems, true);
  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::eiam_return_values, true);
  ECA_LOGGER::instance().set_log_level(ECA_LOGGER::module_names, true);
  if (verbose == true) {
    ECA_LOGGER::instance().set_log_level(ECA_LOGGER::user_objects, true);
    ECA_LOGGER::instance().set_log_level(ECA_LOGGER::system_objects, true);
  }

  a = ECA_OBJECT_FACTORY::create_audio_object(argv[1]);
  b = ECA_OBJECT_FACTORY::create_audio_object(argv[2]);

  if (a == 0 ||
      b == 0)
    return 1;

  a->set_io_mode(AUDIO_IO::io_read);
  b->set_io_mode(AUDIO_IO::io_read);
  a->set_buffersize(buffersize);
  b->set_buffersize(buffersize);

  a->open();
  b->open();

  if (a->is_open() != true ||
      b->is_open() != true) {
    ECA_LOG_MSG(ECA_LOGGER::errors, "Cannot open inputs");
    return 1;
  }
  
  if (a->finite_length_stream() != true ||
      b->finite_length_stream() != true) {
    ECA_LOG_MSG(ECA_LOGGER::errors, "Input not of finite length, cannot compare");
    return 1;
  }

  if (a->length().samples() != 
      b->length().samples()) {
    ECA_LOG_MSG(ECA_LOGGER::errors, "File size mismatch");
    return 1; 
  }

  while(1) {
    a->read_buffer(&buf_a);
    b->read_buffer(&buf_b);

    /* note: use 15bit precision for checks */
    if (SAMPLE_BUFFER_FUNCTIONS::is_almost_equal(buf_a, buf_b, 15, verbose) != true) {
      ECA_LOG_MSG(ECA_LOGGER::errors, "Files differ at pos " +
		  kvu_numtostr(a->position_in_seconds_exact()));
      return 1;
    }

    if (a->finished() == true ||
	b->finished() == true) 
      break;
  }

  a->close();
  b->close();

  return 0;
}
