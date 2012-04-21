// ------------------------------------------------------------------------
// audiofx_lv2.cpp: Wrapper class for LV2 plugins
// Copyright (C) 2000-2004, 2011 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
//
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

#if ECA_USE_LIBLILV

#include <dlfcn.h>
#include <kvu_utils.h>
#include <kvu_dbc.h>
#include <kvu_numtostr.h>
#include "samplebuffer.h"
#include "audiofx_lv2.h"
#include "audiofx_lv2_world.h"
#include "eca-error.h"
#include "eca-logger.h"



EFFECT_LV2::EFFECT_LV2 (Lilv::Plugin pdesc) throw(ECA_ERROR&) :plugin_desc(pdesc) 
{
  bool inplacebroken=plugin_desc.has_feature(ECA_LV2_WORLD::InPlaceBrokenNode());
  if (inplacebroken) {
    throw(ECA_ERROR("AUDIOFX_LV2", "Inplace-broken plugins not supported."));
  }
  /* FIXME: strip linefeeds and other forbidden characters; write down to
   *        to ECA_OBJECT docs what chars are allowed and what are not... */
  Lilv::Node name(plugin_desc.get_name());
  name_rep = string(name.as_string());
  unique_rep = string(plugin_desc.get_uri().as_string());
  Lilv::Node author(plugin_desc.get_author_name()); 
  if(author) {
    maker_rep = string(author.as_string());
  } else {
    maker_rep = string();
  }
  buffer_repp = 0;
  init_ports();
}

EFFECT_LV2::~EFFECT_LV2 (void)
{
  release();
  
  if (plugin_desc != 0) {
    for(unsigned int n = 0; n < plugins_rep.size(); n++) {
      lilv_instance_deactivate(plugins_rep[n]);
      lilv_instance_free(plugins_rep[n]);
    }
  }
}

std::string EFFECT_LV2::description(void) const
{
  return name_rep + " - Author: '" + maker_rep + "'";
}

void EFFECT_LV2::parameter_description(int param, struct PARAM_DESCRIPTION *pd) const
{
  DBC_CHECK(param >= 0);
  DBC_CHECK(param <= static_cast<int>(param_descs_rep.size()));
  *pd = param_descs_rep[param - 1];
}

EFFECT_LV2* EFFECT_LV2::clone(void) const
{ 
  EFFECT_LV2* result = new EFFECT_LV2(plugin_desc);
  for(int n = 0; n < number_of_params(); n++) {
    result->set_parameter(n + 1, get_parameter(n + 1));
  }
  return result;
}

void EFFECT_LV2::init_ports(void) throw(ECA_ERROR&)
{
  // note: run from plugin constructor

  port_count_rep = plugin_desc.get_num_ports();
  in_audio_ports = 0;
  out_audio_ports = 0;

  for(unsigned long m = 0; m < port_count_rep; m++) {
    Lilv::Port port=plugin_desc.get_port_by_index(m);
    if(port.is_a(ECA_LV2_WORLD::AudioClassNode())) {
      if(port.is_a(ECA_LV2_WORLD::InputClassNode())) {
	    ++in_audio_ports;
      } else if (port.is_a(ECA_LV2_WORLD::OutputClassNode())) {
  	    ++out_audio_ports;
      }
    } else if (port.is_a(ECA_LV2_WORLD::ControlClassNode())) {
      struct PARAM_DESCRIPTION pd; 
      parse_parameter_hint_information(plugin_desc,port, &pd);
      params.push_back(pd.default_value);
      param_descs_rep.push_back(pd);
      if (params.size() > 1) param_names_rep += ",";
      string tmp (kvu_string_search_and_replace(string(pd.description), ",", "\\,"));
      param_names_rep += kvu_string_search_and_replace(tmp, ":", "\\:");
    } else if(!port.has_property(ECA_LV2_WORLD::PortConnectionOptionalNode())){
		  throw(ECA_ERROR("AUDIOFX_LV2", "Plugin has required ports which are not audio or control ports."));
	}
  }
}

void EFFECT_LV2::parse_parameter_hint_information(Lilv::Plugin plugin, Lilv::Port p, struct PARAM_DESCRIPTION *pd)
{
  /* if srate not set, use 44.1kHz (used only for calculating
   * param hint values */
  SAMPLE_SPECS::sample_rate_t srate = samples_per_second();
  /* FIXME: this is just ugly! */
  if (srate <= 0) { srate = 44100; }
  
  Lilv::Node name=p.get_name();
  
  /* parameter name */
  pd->description =name.as_string();
  
  Lilv::Node deflt(NULL);
  Lilv::Node min(NULL);
  Lilv::Node max(NULL);
  
  lilv_port_get_range(plugin.me,p.me,&deflt.me,&min.me,&max.me);
  

  bool isSRRelative=p.has_property(ECA_LV2_WORLD::PortSamplerateDependentNode());
  
  /* upper and lower bounds */
  if (min) {
    pd->bounded_below = true;
    
	pd->lower_bound=min.as_float();
    if (isSRRelative) {
      pd->lower_bound *= srate;
    }
  }
  else {
    pd->bounded_below = false;
  }

  if (max) {
    pd->bounded_above = true;

   pd->upper_bound=max.as_float();
   if (isSRRelative) {
	pd->upper_bound *= srate;
   }
  }
  else {
    pd->bounded_above = false;
  }

  /* defaults - case 1 */
  if (deflt) {
   pd->default_value=deflt.as_float();
  }

  /* defaults - case 2 */
  else if (min && !max) {

    if (pd->lower_bound < 0) pd->default_value = 0.0f;
    else pd->default_value = pd->lower_bound;
  }

  /* defaults - case 3 */
  else if (!min && max) {

    if (pd->upper_bound > 0) pd->default_value = 0.0f;
    else pd->default_value = pd->upper_bound;
  }

  /* defaults - case 4 */
  else if (max && min) {

    if (pd->lower_bound < 0 && pd->upper_bound > 0) pd->default_value = 0.0f;
    else if (pd->lower_bound < 0 && pd->upper_bound < 0) pd->default_value = pd->upper_bound;
    else pd->default_value = pd->lower_bound;
  }
  
  /* defaults - case 5 */
  else {
    DBC_CHECK(!min && !max);

    if (isSRRelative) 
      pd->default_value = srate;
    else
      pd->default_value = 1.0f;
  }

  if (p.has_property(ECA_LV2_WORLD::PortToggledNode()))
    pd->toggled = true;
  else
    pd->toggled = false;
  
  if (p.has_property(ECA_LV2_WORLD::PortIntegerNode()))
    pd->integer = true;
  else
    pd->integer = false;
  
  if (p.has_property(ECA_LV2_WORLD::PortLogarithmicNode()))
    pd->logarithmic = true;
  else
    pd->logarithmic = false;

  if (p.is_a(ECA_LV2_WORLD::OutputClassNode()))
    pd->output = true;
  else
    pd->output = false;
}


void EFFECT_LV2::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
  if (param > 0 && (param - 1 < static_cast<int>(params.size()))) {
    //  cerr << "lv2: setting param " << param << " to " << value << "." << endl;
    params[param - 1] = value;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_LV2::get_parameter(int param) const 
{
  if (param > 0 && (param - 1 < static_cast<int>(params.size()))) {
    //  cerr << "lv2: getting param " << param << " with value " << params[param - 1] << "." << endl;
    return(params[param - 1]);
  }
  return(0.0);
}

int EFFECT_LV2::output_channels(int i_channels) const
{
  // note: We have two separate cases: either one plugin 
  //       is instantiated for each channel, or one plugin
  //       per chain. See EFFECT_LV2::init().

  if (in_audio_ports > 1 ||
      out_audio_ports > 1) {

    return out_audio_ports;
  }
  
  return i_channels;
}

void EFFECT_LV2::init(SAMPLE_BUFFER *insample)
{ 
  EFFECT_BASE::init(insample);

  DBC_CHECK(samples_per_second() > 0);

  if (buffer_repp != insample) {
    release();
    buffer_repp = insample;
    buffer_repp->get_pointer_reflock();
  }

  if (plugin_desc != 0) {
    for(unsigned int n = 0; n < plugins_rep.size(); n++) {
		lilv_instance_deactivate(plugins_rep[n]);
      lilv_instance_free(plugins_rep[n]);
    }
    plugins_rep.clear();
  }
  DBC_CHECK(plugins_rep.size() == 0);

  if (in_audio_ports > 1 ||
      out_audio_ports > 1) {
    //Just insert into the first location
    plugins_rep.push_back(Lilv::Instance(plugin_desc,samples_per_second()));
    int inport = 0;
    int outport = 0;
    for(unsigned long m = 0; m < port_count_rep; m++) {
		Lilv::Port p= plugin_desc.get_port_by_index(m);
		if (p.is_a(ECA_LV2_WORLD::AudioClassNode())) {
			if (p.is_a(ECA_LV2_WORLD::InputClassNode())) {
				if (inport < channels()) {
					plugins_rep[0].connect_port(m, buffer_repp->buffer[inport]);
				}
				++inport;
			} else if(p.is_a(ECA_LV2_WORLD::OutputClassNode())) {
				if (outport < channels()) {
					plugins_rep[0].connect_port(m, buffer_repp->buffer[outport]);
				}
				++outport;
			}
		}
    }
    
    if (inport > channels())
      ECA_LOG_MSG(ECA_LOGGER::info, 
		  "WARNING: chain has less channels than plugin has input ports ("
		  + name() + ").");
    if (outport > channels())
      ECA_LOG_MSG(ECA_LOGGER::info, 
		  "WARNING: chain has less channels than plugin has output ports ("
		  + name() + ").");
  } else {
    for(int n = 0; n < channels(); n++) {
      plugins_rep.push_back(Lilv::Instance(plugin_desc,samples_per_second()));
      for(unsigned long m = 0; m < port_count_rep; m++) {
	    Lilv::Port p= plugin_desc.get_port_by_index(m);
		if (p.is_a(ECA_LV2_WORLD::AudioClassNode())) {
			plugins_rep[n].connect_port(m,buffer_repp->buffer[n]);
		}
      }
    }
  }

  ECA_LOG_MSG(ECA_LOGGER::system_objects, 
		"Instantiated " +
		kvu_numtostr(plugins_rep.size()) + 
		" LV2 plugin(s), each with " + 
		kvu_numtostr(in_audio_ports) + 
		" audio input port(s) and " +
		kvu_numtostr(out_audio_ports) +
		" output port(s), to chain with " +
		kvu_numtostr(channels()) +
		" channel(s) and srate of " +
		kvu_numtostr(samples_per_second()) +
		".");

  int data_index = 0;
  for(unsigned long m = 0; m < port_count_rep; m++) {
	  Lilv::Port p=plugin_desc.get_port_by_index(m);
	  if (p.is_a(ECA_LV2_WORLD::ControlClassNode())) {
		for(unsigned int n = 0; n < plugins_rep.size(); n++) {
			plugins_rep[n].connect_port(m,&(params[data_index]));
		}
		++data_index;
	  }
  }
  for(unsigned long m = 0; m < plugins_rep.size(); m++)
	plugins_rep[m].activate();
	
}

void EFFECT_LV2::release(void)
{
  if (buffer_repp != 0) {
    buffer_repp->release_pointer_reflock();
  }
  buffer_repp = 0;
}

void EFFECT_LV2::process(void)
{
  for(unsigned long m = 0; m < plugins_rep.size(); m++)
    lilv_instance_run(plugins_rep[m], buffer_repp->length_in_samples());
}

#endif /* ECA_USE_LIBLILV */
