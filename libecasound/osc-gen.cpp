// ------------------------------------------------------------------------
// osc-gen.cpp: Generic oscillator
// Copyright (C) 1999-2000 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <kvutils/kvu_numtostr.h>
#include <kvutils/message_item.h>

#include "osc-gen.h"
#include "oscillator.h"
#include "eca-debug.h"
#include "eca-error.h"

string GENERIC_OSCILLATOR::filename = "";
void GENERIC_OSCILLATOR::set_preset_file(const string& value) { GENERIC_OSCILLATOR::filename = value; }

CONTROLLER_SOURCE::parameter_type GENERIC_OSCILLATOR::value(void) {
  if (ienvelope.size() == 0) curval = 0.0;

  if (linear) {
    if (current + 1 < ienvelope.size()) { 
      curval = pcounter / pdistance * ienvelope[current + 1];
      curval += (1.0 - pcounter / pdistance) * ienvelope[current];
    }
  }
  else {
    curval = ienvelope[current];
  }

  pcounter += step_length();
  if (pcounter > pdistance) {
    pcounter -= pdistance;
    ++current;
  }

  if (linear) {
    if (current + 1 == ienvelope.size()) current = 0;
  }
  else {
    if (current == ienvelope.size()) current = 0;
  }

  //  cerr << "(gen-osc) new value: " << curval << ".\n";
  return(curval);
}

GENERIC_OSCILLATOR::GENERIC_OSCILLATOR(double freq, int preset_number)
  : OSCILLATOR(freq, 0.0)
{
  set_parameter(1, get_parameter(1));
  set_parameter(2, preset_number);
}

void GENERIC_OSCILLATOR::init(CONTROLLER_SOURCE::parameter_type phasestep) {
  step_length(phasestep);
  pcounter = 0.0;
  current = 0;

  MESSAGE_ITEM m;
  m << "(osc-gen) Generic oscillator created using envelope preset number " << preset_rep << ".";
  ecadebug->msg(m.to_string());
}

GENERIC_OSCILLATOR::~GENERIC_OSCILLATOR (void) {
   while(ienvelope.size() > 0) ienvelope.pop_back();
}

void GENERIC_OSCILLATOR::read_envelope(void) throw(ECA_ERROR&) {
  preset_found = false;
  linear = false;
  ienvelope.resize(0);

  ifstream fin (GENERIC_OSCILLATOR::filename.c_str());

  if (!fin) {
    throw(ECA_ERROR("OSC-GEN", "Unable to open envelope file" +
			GENERIC_OSCILLATOR::filename + " (~/.ecasoundrc)"));
  }

  int curpreset;
  string sana;
  while(fin >> sana) {
    if (sana.size() > 0 && sana[0] == '#') {
      while(fin.get() != '\n' && fin.eof() == false);
      continue;
    }
    else {
      ecadebug->msg(ECA_DEBUG::user_objects, "(osc-gen) Next preset is " + sana + ".");
      curpreset = atoi(sana.c_str());
      if (curpreset == preset_rep) {
	ecadebug->msg(ECA_DEBUG::user_objects, "(osc-gen) Found the right preset!");
	preset_found = true;
	fin >> sana; 
	if (fin.eof()) break; 
	else if (sana.size() > 0 && sana[0] == 'L') {
	  linear = true;
	  ecadebug->msg(ECA_DEBUG::user_objects,"(osc-gen) Using linear-interpolation between envelope points.");
	}
	else {
	  linear = false;
	  ecadebug->msg(ECA_DEBUG::user_objects,"(osc-gen) Using static envelope points.");
	}
	double newpoint; 
	while(fin >> newpoint) {
	  ecadebug->msg(ECA_DEBUG::user_objects, "(osc-gen) Added value: " + kvu_numtostr(newpoint));
	  ienvelope.push_back(newpoint);
	}
      }
      else 
	while(fin.get() != '\n' && fin.eof() == false);
    }
  }
  if (preset_found == false) {
    throw(ECA_ERROR("OSC-GEN", "Preset " +
			kvu_numtostr(preset_rep) + " not found from envelope file " + 
			filename + "."));
  }
}

void GENERIC_OSCILLATOR::set_parameter(int param, CONTROLLER_SOURCE::parameter_type value) {
  switch (param) {
  case 1: 
    frequency(value);
    L = 1.0 / frequency();   // length of one wave in seconds
    break;

  case 2: 
    preset_rep = static_cast<int>(value);
    read_envelope();
    if (linear) {
      if (ienvelope.size() > 1) pdistance = L / (ienvelope.size() - 1);
      else pdistance = L;
    }
    else {
      if (ienvelope.size() > 0) pdistance = L / ienvelope.size();
    }
    
    break;
  }
}

CONTROLLER_SOURCE::parameter_type GENERIC_OSCILLATOR::get_parameter(int param) const { 
  switch (param) {
  case 1: 
    return(frequency());

  case 2:
    return(static_cast<parameter_type>(preset_rep));
  }
  return(0.0);
}
