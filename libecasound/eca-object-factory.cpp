// ------------------------------------------------------------------------
// eca-object-factory.cpp: Class for creating various ecasound objects.
// Copyright (C) 2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include "audioio.h"
#include "eca-static-object-maps.h"
#include "eca-audio-object-map.h"
#include "eca-object-factory.h"

AUDIO_IO* ECA_OBJECT_FACTORY::create_audio_object(const string& arg) {
  assert(arg.empty() != true);
 
  ::register_default_objects();
  string fname = get_argument_number(1, arg);
  if (fname.find(".") != string::npos) {
    fname = string(fname, fname.find_last_of("."), string::npos);
  }

  AUDIO_IO* main_file = 0;
  main_file = ECA_AUDIO_OBJECT_MAP::object(fname);

  if (main_file != 0) {
    main_file = main_file->new_expr();
    main_file->map_parameters();
    ecadebug->msg(ECA_DEBUG::system_objects, "(eca-object-factory) Object \"" + arg + "\" created, type \"" + main_file->name() + "\". Has " + kvu_numtostr(main_file->number_of_params()) + " parameter(s).");
    for(int n = 0; n < main_file->number_of_params(); n++) {
      main_file->set_parameter(n + 1, get_argument_number(n + 1, arg));
    }
  }
  return(main_file);
}
