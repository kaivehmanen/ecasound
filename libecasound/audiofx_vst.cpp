// ------------------------------------------------------------------------
// audiofx_vsp.cpp: Wrapper class for VST1.0/2.0 plugins
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef FEELING_EXPERIMENTAL

#include <dlfcn.h>
#include <kvutils.h>

#include "samplebuffer.h"
#include "audiofx_vst.h"

#include "eca-error.h"

long vst_audiomaster(AEffect *effect, 
		     long opcode, 
		     long index, 
		     long value, 
		     void *ptr, 
		     float opt) {
  switch(opcode)
    {
    case audioMasterAutomate: {} 
    case audioMasterVersion: {}
    case audioMasterCurrentId: {}
    case audioMasterIdle: {}
    case audioMasterPinConnected: {}
    default: { }
    }
}

VST_plugin_descriptor create_vst_plugin(const string& fname) throw(ECA_ERROR&);

EFFECT_VST::EFFECT_VST (const string& fname) throw(ECA_ERROR&) {
  cerr << "here!" << endl;
  library_file_rep = fname;
  master_func = create_vst_plugin(fname);
  vst_handles.push_back(master_func(vst_audiomaster));
//    if ((vst_handles.back()->flags & effFlagsCanReplacing) !=
//        effFlagsCanReplacing) 
//      throw(ECA_ERROR("AUDIOFX_VST", "Plugin doesn't support process_replacing()."));

  label_rep = "VST-plugin";
  unique_rep = kvu_numtostr(vst_handles.back()->uniqueID);

  for(int n = 0; n < vst_handles.back()->numParams; n++) {
    param_names_rep += "p" + kvu_numtostr(n + 1);
    if (n != vst_handles.back()->numParams) param_names_rep += ",";
  }
}

EFFECT_VST::~EFFECT_VST (void) { }

void EFFECT_VST::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  for(int n = 0; n < static_cast<int>(vst_handles.size()); n++) {
    vst_handles[n]->setParameter(vst_handles[n], param, value);
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_VST::get_parameter(int param) const { 
  return(vst_handles.back()->getParameter(vst_handles.back(), param));
}

void EFFECT_VST::init(SAMPLE_BUFFER *insample) { 
  buffer = insample;
//    for(int n = static_cast<int>(vst_handles.size()); n <
//  	buffer->number_of_channels(); n++) {
//      vst_handles.push_back(master_func(vst_audiomaster));
//    }
}

void EFFECT_VST::process(void) {
  for(int n = 0; n < static_cast<int>(vst_handles.size()); n++) {
    //    vst_handles[n]->processReplacing(vst_handles[n],
    vst_handles[n]->process(vst_handles[n],
			    &(buffer->buffer[n]), 
			    &(buffer->buffer[n]),
			    buffer->length_in_samples());
  }
}

VST_plugin_descriptor create_vst_plugin(const string& fname) throw(ECA_ERROR&) { 
  cerr << "fname: " << fname << endl;
  void *plugin_handle = dlopen(fname.c_str(), RTLD_LAZY);
  if (plugin_handle == 0) 
    throw(ECA_ERROR("ECA_STATIC_OBJECT_MAPS", "Unable to open VST plugin file."));

  VST_plugin_descriptor desc_func;
  desc_func = (VST_plugin_descriptor)dlsym(plugin_handle, "main_plugin");
  if (desc_func == 0)
    throw(ECA_ERROR("ECA_STATIC_OBJECT_MAPS", "Unable find plugin VST's main_plugin()."));

  return(desc_func);
}

#endif // experimental
