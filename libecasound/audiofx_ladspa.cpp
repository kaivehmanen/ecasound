// ------------------------------------------------------------------------
// audiofx_ladspa.cpp: Wrapper class for LADSPA plugins
// Copyright (C) 2000-2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
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

#include <assert.h>
#include <dlfcn.h>
#include <kvutils.h>
#include <kvutils/dbc.h>
#include <kvutils/kvu_numtostr.h>
#include "samplebuffer.h"
#include "audiofx_ladspa.h"
#include "eca-error.h"
#include "eca-debug.h"

EFFECT_LADSPA::EFFECT_LADSPA (const LADSPA_Descriptor *pdesc) throw(ECA_ERROR&)
{
  plugin_desc = pdesc;
  if ((plugin_desc->Properties & LADSPA_PROPERTY_INPLACE_BROKEN) ==
      LADSPA_PROPERTY_INPLACE_BROKEN)
    throw(ECA_ERROR("AUDIOFX_LADSPA", "Inplace-broken plugins not supported."));

  name_rep = string(plugin_desc->Name);
  unique_rep = string(plugin_desc->Label);
  unique_number_rep = static_cast<long int>(plugin_desc->UniqueID);
  buffer_repp = 0;

  init_ports();
}

EFFECT_LADSPA::~EFFECT_LADSPA (void)
{
  release();
  
  if (plugin_desc != 0) {
    for(unsigned int n = 0; n < plugins_rep.size(); n++) {
      if (plugin_desc->deactivate != 0) plugin_desc->deactivate(plugins_rep[n]);
      plugin_desc->cleanup(plugins_rep[n]);
    }
  }
}

EFFECT_LADSPA* EFFECT_LADSPA::clone(void) const
{ 
  EFFECT_LADSPA* result = new EFFECT_LADSPA(plugin_desc);
  for(int n = 0; n < number_of_params(); n++) {
    result->set_parameter(n + 1, get_parameter(n + 1));
  }
  return(result);
}

void EFFECT_LADSPA::init_ports(void) {
  port_count_rep = plugin_desc->PortCount;
  in_audio_ports = 0;
  out_audio_ports = 0;

  /* if srate not set, use 44.1kHz (used only for calculating
   * param hint values */
  SAMPLE_SPECS::sample_rate_t srate = samples_per_second();
  if (srate <= 0) { srate = 44100; }

  for(unsigned long m = 0; m < port_count_rep; m++) {
    if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_AUDIO) == LADSPA_PORT_AUDIO) {
      if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_INPUT) == LADSPA_PORT_INPUT)
	++in_audio_ports;
      else
	++out_audio_ports;
    }

    /** 
     * LADSPA API doesn't specify how to initialize control
     * ports to sane initial values, so here we try to 
     * make a educated guess based on the lower (lowb) and
     * upper (upperb) bounds:
     *
     * 1) lowb == x and upperb == n/a
     *    a) x < 0, initval = 0
     *    b) x >= 0, initval = x
     * 2) lowb == n/a and upperb == x
     *    a) x > 0, initval = 0
     *    b) x <= 0, initval = x
     * 3) lowb == x and upperb == y 
     *    a) x < 0 and y > 0, initval = 0
     *    b) x < 0 and y < 0, initval = y
     *    c) x > 0 and y > 0, initval = x
     * 4) lowb == n/a and upperb == n/a, initval = 1
     */
    if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_CONTROL) ==
	LADSPA_PORT_CONTROL) {
      parameter_t init_value, lowb, upperb;

      if (LADSPA_IS_HINT_SAMPLE_RATE(plugin_desc->PortRangeHints[m].HintDescriptor)) 
	lowb = plugin_desc->PortRangeHints[m].LowerBound * srate;
      else
	lowb = plugin_desc->PortRangeHints[m].LowerBound;

      if (LADSPA_IS_HINT_SAMPLE_RATE(plugin_desc->PortRangeHints[m].HintDescriptor)) 
	upperb = plugin_desc->PortRangeHints[m].UpperBound * srate;
      else
	upperb = plugin_desc->PortRangeHints[m].UpperBound;

      /* case 1 */
      if (LADSPA_IS_HINT_BOUNDED_BELOW(plugin_desc->PortRangeHints[m].HintDescriptor) &&
	  !LADSPA_IS_HINT_BOUNDED_ABOVE(plugin_desc->PortRangeHints[m].HintDescriptor)) {

	if (lowb < 0) init_value = 0.0f;
	else init_value = lowb;
      }

      /* case 2 */
      else if (!LADSPA_IS_HINT_BOUNDED_BELOW(plugin_desc->PortRangeHints[m].HintDescriptor) &&
	       LADSPA_IS_HINT_BOUNDED_ABOVE(plugin_desc->PortRangeHints[m].HintDescriptor)) {

	if (upperb > 0) init_value = 0.0f;
	else init_value = upperb;
      }

      /* case 3 */
      else if (LADSPA_IS_HINT_BOUNDED_BELOW(plugin_desc->PortRangeHints[m].HintDescriptor) &&
	       LADSPA_IS_HINT_BOUNDED_ABOVE(plugin_desc->PortRangeHints[m].HintDescriptor)) {

	if (lowb < 0 && upperb > 0) init_value = 0.0f;
	else if (lowb < 0 && upperb < 0) init_value = upperb;
	else init_value = lowb;
      }

      /* case 4 */
      else {
	assert(!LADSPA_IS_HINT_BOUNDED_BELOW(plugin_desc->PortRangeHints[m].HintDescriptor) &&
	       !LADSPA_IS_HINT_BOUNDED_ABOVE(plugin_desc->PortRangeHints[m].HintDescriptor));

	if (LADSPA_IS_HINT_SAMPLE_RATE(plugin_desc->PortRangeHints[m].HintDescriptor)) 
	  init_value = srate;
	else
	  init_value = 1.0f;
      }

      params.push_back(init_value);
      if (params.size() > 1) param_names_rep += ",";
      param_names_rep += string_search_and_replace(string(plugin_desc->PortNames[m]), ',', ' ');;
    }
  }
}

void EFFECT_LADSPA::parameter_description(int param, struct PARAM_DESCRIPTION *pd)
{
  int ctrl_port_n = 0; 

  /* if srate not set, use 44.1kHz (used only for calculating
   * param hint values */
  SAMPLE_SPECS::sample_rate_t srate = samples_per_second();
  if (srate <= 0) { srate = 44100; }

  for(unsigned long m = 0; m < port_count_rep; m++) {
    if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_CONTROL) == LADSPA_PORT_CONTROL) {
      ++ctrl_port_n;
      if (ctrl_port_n == param) {
	pd->default_value = 1;
	pd->description = get_parameter_name(param);

	if (LADSPA_IS_HINT_BOUNDED_ABOVE(plugin_desc->PortRangeHints[m].HintDescriptor)) {
	  pd->bounded_above = true;
	  if (LADSPA_IS_HINT_SAMPLE_RATE(plugin_desc->PortRangeHints[m].HintDescriptor)) 
	    pd->upper_bound = plugin_desc->PortRangeHints[m].UpperBound * srate;
	  else
	    pd->upper_bound = plugin_desc->PortRangeHints[m].UpperBound;
	}
	else
	  pd->bounded_above = false;

	if (LADSPA_IS_HINT_BOUNDED_BELOW(plugin_desc->PortRangeHints[m].HintDescriptor)) {
	  pd->bounded_below = true;
	  if (LADSPA_IS_HINT_SAMPLE_RATE(plugin_desc->PortRangeHints[m].HintDescriptor)) 
	    pd->lower_bound = plugin_desc->PortRangeHints[m].LowerBound * samples_per_second();
	  else
	    pd->lower_bound = plugin_desc->PortRangeHints[m].LowerBound;
	}
	else 
	  pd->bounded_below = false;

	if (LADSPA_IS_HINT_TOGGLED(plugin_desc->PortRangeHints[m].HintDescriptor))
	  pd->toggled = true;
	else
	  pd->toggled = false;

	if (LADSPA_IS_HINT_INTEGER(plugin_desc->PortRangeHints[m].HintDescriptor)) {
	  pd->integer = true;
	}
	else {
	  pd->integer = false;
	}

	if (LADSPA_IS_HINT_LOGARITHMIC(plugin_desc->PortRangeHints[m].HintDescriptor))
	  pd->logarithmic = true;
	else
	  pd->logarithmic = false;

	if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_OUTPUT) == LADSPA_PORT_CONTROL)
	  pd->output = true;
	else
	  pd->output = false;
      }
    }
  }
}

void EFFECT_LADSPA::set_parameter(int param, CHAIN_OPERATOR::parameter_t value)
{
  if (param > 0 && (param - 1 < static_cast<int>(params.size()))) {
    //  cerr << "ladspa: setting param " << param << " to " << value << "." << endl;
    params[param - 1] = value;
  }
}

CHAIN_OPERATOR::parameter_t EFFECT_LADSPA::get_parameter(int param) const 
{
  if (param > 0 && (param - 1 < static_cast<int>(params.size()))) {
    //  cerr << "ladspa: getting param " << param << " with value " << params[param - 1] << "." << endl;
    return(params[param - 1]);
  }
  return(0.0);
}

void EFFECT_LADSPA::init(SAMPLE_BUFFER *insample)
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
      if (plugin_desc->deactivate != 0) plugin_desc->deactivate(plugins_rep[n]);
      plugin_desc->cleanup(plugins_rep[n]);
    }
  }


  // NOTE: the fancy definition :)
  //       if ((in_audio_ports > 1 &&
  //            in_audio_ports <= channels() &&
  //            out_audio_ports <= channels()) ||
  //           (out_audio_ports > 1 &&
  //            in_audio_ports <= channels() &&
  //            out_audio_ports <= channels())) {}

  if (in_audio_ports > 1 ||
      out_audio_ports > 1) {
    plugins_rep.resize(1);
    plugins_rep[0] = reinterpret_cast<LADSPA_Handle*>(plugin_desc->instantiate(plugin_desc, samples_per_second()));
    int inport = 0;
    int outport = 0;
    for(unsigned long m = 0; m < port_count_rep; m++) {
      if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_AUDIO) == LADSPA_PORT_AUDIO) {
	if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_INPUT) == LADSPA_PORT_INPUT) {
	  plugin_desc->connect_port(plugins_rep[0], m, buffer_repp->buffer[inport]);
	  ++inport;
	  if (inport == channels()) inport--;
	}
	else {
	  plugin_desc->connect_port(plugins_rep[0], m, buffer_repp->buffer[outport]);
	  ++outport;
	  if (outport == channels()) outport--;
	}
      }
    }
  } 
  else {
    plugins_rep.resize(channels());
    for(unsigned int n = 0; n < plugins_rep.size(); n++) {
      plugins_rep[n] = reinterpret_cast<LADSPA_Handle*>(plugin_desc->instantiate(plugin_desc, samples_per_second()));

      for(unsigned long m = 0; m < port_count_rep; m++) {
	if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_AUDIO) == LADSPA_PORT_AUDIO) {
	  plugin_desc->connect_port(plugins_rep[n], m, buffer_repp->buffer[n]);
	}
      }
    }
  }

  ecadebug->msg(ECA_DEBUG::system_objects, 
		"(audiofx_ladspa) Instantiated " +
		kvu_numtostr(plugins_rep.size()) + 
		" LADSPA plugin(s), each with " + 
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
    if ((plugin_desc->PortDescriptors[m] & LADSPA_PORT_CONTROL) ==
	LADSPA_PORT_CONTROL) {
      for(unsigned int n = 0; n < plugins_rep.size(); n++) {
	plugin_desc->connect_port(plugins_rep[n], m, &(params[data_index]));
      }
      ++data_index;
    }
  }
  for(unsigned long m = 0; m < plugins_rep.size(); m++)
    if (plugin_desc->activate != 0) plugin_desc->activate(plugins_rep[m]);
}

void EFFECT_LADSPA::release(void)
{
  if (buffer_repp != 0) {
    buffer_repp->release_pointer_reflock();
  }
  buffer_repp = 0;
}

void EFFECT_LADSPA::process(void)
{
  for(unsigned long m = 0; m < plugins_rep.size(); m++)
    plugin_desc->run(plugins_rep[m], buffer_repp->length_in_samples());
}
