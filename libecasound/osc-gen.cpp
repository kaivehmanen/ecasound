// ------------------------------------------------------------------------
// osc-gen.cpp: Generic oscillator
// Copyright (C) 1999-2002,2008,2012 Kai Vehmanen
//
// Attributes:
//     eca-style-version: 3
//
// This program is fre software; you can redistribute it and/or modify
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

#include <vector>
#include <string>

#include <kvu_dbc.h>
#include <kvu_numtostr.h>

#include "eca-object-factory.h"
#include "osc-gen.h"
#include "oscillator.h"
#include "eca-logger.h"

/* For hunting bugs in envelope code */
//#define DEBUG_ENVELOPE_POINTS 

#ifdef DEBUG_ENVELOPE_POINTS
#define _DEBUG_ENVELOPE(x) do { x; } while(0)
#include <iostream>
#else
#define _DEBUG_ENVELOPE(x)
#endif

size_t GENERIC_OSCILLATOR::current_stage(double pos)
{
  size_t start, n;

  // note: like 'std::fmod(pos, loop_length_rep)' but faster
  double loop_pos = pos - static_cast<int>(pos / loop_length_rep) * loop_length_rep;
    
  // note: optimize for the case where position changes
  //       linearly
  start = last_stage_rep;

  for(n = start;;) {

    double pos_scaled = loop_pos / loop_length_rep;
    double p1 = envtable_rep[n].pos;
    double p2 = envtable_rep[n + 1].pos;

    _DEBUG_ENVELOPE(std::cout << 
                    "looking for stage " + kvu_numtostr(n) +
                    ", stages " + kvu_numtostr(envtable_rep.size()) +
                    ", pos " + kvu_numtostr(pos) +
                    ", looppos " + kvu_numtostr(loop_pos) +
                    ", scaled " + kvu_numtostr(pos_scaled) +
                    ", p1 " + kvu_numtostr(p1) +
                    ", p2 " + kvu_numtostr(p2) << "\n");

    if (pos_scaled >= p1 && pos_scaled < p2) {
      _DEBUG_ENVELOPE(std::cout << 
                      "found stage " + kvu_numtostr(n) << "\n");
      last_pos_scaled_rep = pos_scaled;
      break;
    }

    n++;
    if (n + 1 == envtable_rep.size())
      n = 0;

    if (n == start) {
      static bool once = true;
      if (once) {
        ECA_LOG_MSG(ECA_LOGGER::info,
                    "ERROR: invalid envelop");
        once = false;
      }
      break;
    }
  }

  _DEBUG_ENVELOPE(if (last_stage_rep != n) { \
      std::cout << "stage change from " \
                << kvu_numtostr(last_stage_rep)  \
                <<" to " << kvu_numtostr(n) << "\n"; } );
  
  last_stage_rep = n;
  
  return n;
}

CONTROLLER_SOURCE::parameter_t GENERIC_OSCILLATOR::value(double pos)
{
  size_t stage = current_stage(pos);
  double retval = 0.0f;

  /* case: hold/step */
  if (mode_rep == 0) {
    retval = envtable_rep[stage].val;
    _DEBUG_ENVELOPE(std::cout
                    << "hold value " << retval
                    << ", stage " << stage << "\n");

  }
  /* case: linear interpolation */
  else {
    double p1 = envtable_rep[stage].pos;
    double p2 = envtable_rep[stage + 1].pos;
    double v1 = envtable_rep[stage].val;
    double v2 = envtable_rep[stage + 1].val;
    double mult = 1.0 - ((p2 - last_pos_scaled_rep) / (p2 - p1));
    retval = v1 + (mult * (v2 - v1));
    _DEBUG_ENVELOPE(std::cout
                    << "linear value " << retval
                    << ", stage " << stage 
                    << ", v1 " << v1 << ", v2 " << v2 << ", scaledpos " << last_pos_scaled_rep
                    << "\n");
  }

  return retval;
}

GENERIC_OSCILLATOR::GENERIC_OSCILLATOR(double freq, int mode)
  : OSCILLATOR(freq, 0.0)
{
  last_stage_rep = 0;

  set_param_count(0);

  set_parameter(1, get_parameter(1));
  set_parameter(2, mode);
}

void GENERIC_OSCILLATOR::init(void)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects,
              "Generic oscillator init with params: "
              + ECA_OBJECT_FACTORY::operator_parameters_to_eos(this));
  if (envtable_rep.size() < 2)
    envtable_rep.resize(2);
}

GENERIC_OSCILLATOR::~GENERIC_OSCILLATOR (void)
{
}

void GENERIC_OSCILLATOR::set_param_count(int params)
{
  param_names_rep = "freq,mode,pcount,start_val,end_val";
  if (params > 0) {
    for(int n = 0; n < params; n++) {
      std::string num = kvu_numtostr(n + 1);
      param_names_rep += ",pos";
      param_names_rep += num;
      param_names_rep += ",val";
      param_names_rep += num;
    }
  }
}

std::string GENERIC_OSCILLATOR::parameter_names(void) const
{
  return param_names_rep;
}

void GENERIC_OSCILLATOR::prepare_envelope(void)
{
  size_t len = 2 + (params_rep.size() + 1) / 2;
  envtable_rep.resize(len);

  envtable_rep[0].pos = 0.0;
  envtable_rep[0].val = first_value_rep;
  size_t n = 0;
  size_t p1_offset = 1;
  size_t p_end_offset = params_rep.size() / 2 + p1_offset;
  for(; n < params_rep.size(); n++) {
    envtable_rep[n / 2 + p1_offset].pos = params_rep[n];
    if (++n == params_rep.size())
      break;
    envtable_rep[n / 2 + p1_offset].val = params_rep[n];
  } 
  DBC_CHECK(p_end_offset < envtable_rep.size());
  envtable_rep[p_end_offset].pos = 1.0;
  envtable_rep[p_end_offset].val = last_value_rep;

}

void GENERIC_OSCILLATOR::set_parameter(int param, CONTROLLER_SOURCE::parameter_t value)
{
  ECA_LOG_MSG(ECA_LOGGER::user_objects,
              "setting param " + kvu_numtostr(param) 
              + " (" + get_parameter_name(param) + ")"
              + " => " + kvu_numtostr(value));

  switch (param) {
  case 1: 
    frequency(value);
    loop_length_rep = 1.0f / frequency(); // length of one wave in seconds
    break;

  case 2: 
    mode_rep = static_cast<int>(value);
    break;

  case 3: 
    set_param_count(static_cast<int>(value));
    break;

  case 4:
    first_value_rep = value;
    break;

  case 5:
    last_value_rep = value;
    break;

  default: {
      int pointnum = param - 5;
      if (pointnum > 0) {

        if (pointnum > static_cast<int>(params_rep.size()))
          params_rep.resize(pointnum);
        
        params_rep[pointnum - 1] = value;
      }

      prepare_envelope();

      break;
    }
  }
}

CONTROLLER_SOURCE::parameter_t GENERIC_OSCILLATOR::get_parameter(int param) const
{ 
  switch (param) {
  case 1: 
    return frequency();

  case 2:
    return static_cast<parameter_t>(mode_rep);

  case 3:
    return static_cast<parameter_t>((number_of_params() - 5) / 2);

  case 4:
    return static_cast<parameter_t>(first_value_rep);

  case 5:
    return static_cast<parameter_t>(last_value_rep);

  default:
    int pointnum = param - 5;
    if (pointnum > 0) {
      if (pointnum <= static_cast<int>(params_rep.size())) {
        return static_cast<parameter_t>(params_rep[pointnum - 1]);
      }
    }
  }
  return 0.0;
}
