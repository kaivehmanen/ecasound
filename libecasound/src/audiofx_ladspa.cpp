// ------------------------------------------------------------------------
// audiofx_ladspa.cpp: Wrapper class for LADSPA plugins
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

#include <dlfcn.h>

#include <kvutils.h>

#include "ladspa.h"
#include "samplebuffer.h"
#include "audiofx_ladspa.h"

#include "eca-error.h"

EFFECT_LADSPA::EFFECT_LADSPA (const struct LADSPA_Descriptor *pdesc) throw(ECA_ERROR*) {
  plugin_desc = pdesc;
  if ((plugin_desc->Properties & LADSPA_PROPERTY_INPLACE_BROKEN) ==
      LADSPA_PROPERTY_INPLACE_BROKEN)
    throw(new ECA_ERROR("AUDIOFX_LADSPA", "Inplace-broken plugins not supported."));

  label_rep = string(plugin_desc->Name);
  unique_rep = string(plugin_desc->Label);
  port_count_rep = plugin_desc->PortCount;

  for(unsigned long m = 0; m < port_count_rep; m++) {
    if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_CONTROL) ==
	LADSPA_PORT_CONTROL) {
      params.push_back(1.0);
      if (params.size() > 1) param_names_rep += ",";
      param_names_rep += string(plugin_desc->PortNames[m]);
    }
  }
}

EFFECT_LADSPA::~EFFECT_LADSPA (void) {
  if (plugin_desc != 0) {
    for(unsigned int n = 0; n < plugins.size(); n++) {
      if (plugin_desc->deactivate != 0) plugin_desc->deactivate(plugins[n]);
      plugin_desc->cleanup(plugins[n]);
    }
  }
}

void EFFECT_LADSPA::set_parameter(int param, CHAIN_OPERATOR::parameter_type value) {
  if (param > 0 && param < static_cast<int>(params.size() + 1)) {
    params[param - 1] = value;
  }
}

CHAIN_OPERATOR::parameter_type EFFECT_LADSPA::get_parameter(int param) const { 
  if (param > 0 && param < static_cast<int>(params.size() + 1)) {
    return(params[param - 1]);
  }
  return(0.0);
}

void EFFECT_LADSPA::init(SAMPLE_BUFFER *insample) { 
  buffer = insample;
  plugins.resize(buffer->number_of_channels());
  for(unsigned int n = 0; n < plugins.size(); n++) {
    plugins[n] = reinterpret_cast<LADSPA_Handle*>(plugin_desc->instantiate(plugin_desc, buffer->sample_rate()));
  }
  for(unsigned long m = 0; m < port_count_rep; m++) {
    for(unsigned int n = 0; n < plugins.size(); n++) {
      if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_AUDIO) == LADSPA_PORT_AUDIO) {
	plugin_desc->connect_port(plugins[n], m, buffer->buffer[n]);
      }
    }
  }
  int data_index = 0;
  for(unsigned long m = 0; m < port_count_rep; m++) {
    if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_CONTROL) ==
	LADSPA_PORT_CONTROL) {
      for(unsigned int n = 0; n < plugins.size(); n++) {
	plugin_desc->connect_port(plugins[n], m, &(params[data_index]));
      }
      ++data_index;
    }
  }
  for(unsigned long m = 0; m < plugins.size(); m++)
    if (plugin_desc->activate != 0) plugin_desc->activate(plugins[m]);
}

void EFFECT_LADSPA::process(void) {
  for(unsigned long m = 0; m < plugins.size(); m++)
    plugin_desc->run(plugins[m], buffer->length_in_samples());
}
