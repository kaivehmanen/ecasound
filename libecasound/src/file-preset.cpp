// ------------------------------------------------------------------------
// file_preset.cpp: File based effect preset
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

#include <kvutils.h>

#include "resource-file.h"
#include "eca-debug.h"
#include "eca-error.h"
#include "file-preset.h"

FILE_PRESET::FILE_PRESET(const string& file_name) {
  RESOURCE_FILE pfile (file_name);
  string pname = "empty";
  if (pfile.keywords().size() > 0) pname = pfile.keywords()[0];
  set_name(pname);
  set_filename(file_name);
  parse(pfile.resource(pname));
}
