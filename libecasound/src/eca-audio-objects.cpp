// ------------------------------------------------------------------------
// eca-audio-objects.cpp: A specialized container class for
//                        representing a group of inputs, outputs and
//                        chains. Not meant for general use.
// Copyright (C) 1999 Kai Vehmanen (kaiv@wakkanet.fi)
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

#include <config.h>

#include <string>
#include <vector>
#include <map>

#include <kvutils.h>

#include "audioio.h"
#include "audioio-types.h"
#include "audioio-cdr.h"
#include "audioio-wave.h"
#include "audioio-oss.h"
// #include "audioio-oss_dma.h"
#include "audioio-ewf.h"
#include "audioio-mp3.h"
#include "audioio-mikmod.h"
#include "audioio-alsa.h"
#include "audioio-alsa2.h"
#include "audioio-alsalb.h"
#include "audioio-af.h"
#include "audioio-raw.h"
#include "audioio-null.h"
#include "audioio-rtnull.h"

#include "eca-chain.h"
#include "samplebuffer.h"

#include "eca-debug.h"
#include "eca-audio-objects.h"

ECA_AUDIO_OBJECTS::ECA_AUDIO_OBJECTS(void) 
  : buffersize_rep(0),
    double_buffering_rep (false),
    precise_sample_rates_rep (false),
    output_openmode_rep (si_readwrite),
    selected_chainids (0),
    last_audio_object(0) { }

ECA_AUDIO_OBJECTS::~ECA_AUDIO_OBJECTS(void) {
  ecadebug->msg(1,"ECA_AUDIO_OBJECTS destructor!");

  for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
    ecadebug->msg(1, "(eca-audio-objects) Deleting chain \"" + (*q)->name() + "\".");
    delete *q;
  }
  
  for(vector<AUDIO_IO*>::iterator q = inputs.begin(); q != inputs.end(); q++) {
    ecadebug->msg(1, "(eca-audio-objects) Deleting audio device/file \"" + (*q)->label() + "\".");
    delete *q;
  }
  //  inputs.resize(0);
  
  for(vector<AUDIO_IO*>::iterator q = outputs.begin(); q != outputs.end(); q++) {
    ecadebug->msg(1, "(eca-audio-objects) Deleting audio device/file \"" + (*q)->label() + "\".");
    delete *q;
  }
  //  outputs.resize(0);
}

bool ECA_AUDIO_OBJECTS::is_valid(void) const {
  if (inputs.size() == 0) {
    ecadebug->msg(1, "(eca-audio-objects) No inputs in the current chainsetup.");
    return(false);
  }
  if (outputs.size() == 0) {
    ecadebug->msg(1, "(eca-audio-objects) No outputs in the current chainsetup.");
    return(false);
  }
  if (chains.size() == 0) {
    ecadebug->msg(1, "(eca-audio-objects) No chains in the current chainsetup.");
    return(false);
  }
  for(vector<CHAIN*>::const_iterator q = chains.begin(); q != chains.end();
      q++) {
    if ((*q)->is_valid() == false) return(false);
  }
  return(true);
}

void ECA_AUDIO_OBJECTS::interpret_audioio_device (const string& argu, const string& argu_param) {
  if (argu.size() == 0) return;
  if (argu[0] != '-') return;

  string tname = get_argument_number(1, argu);
  if (tname == "") tname = argu_param;                    // -i and -o
  else if (tname[0] == '-') tname = argu_param;           // -i and -o 
  else {                                                  // -i:
	string temp = tname;
	to_lowercase(temp);
	if (temp == "alsa" ||
	    temp == "alsalb") {                             // -i:alsa,c,d
	  string::const_iterator p = argu.begin();        // -o:alsa,c,d
	  ++p; ++p; ++p;
	  tname = string(p, argu.end());
	}
  }
  ecadebug->msg(5,"(eca-audio-objects) adding file \"" + tname + "\".");
  if (argu.size() < 2) return;  
  if (argu[0] != '-') return;
  switch(argu[1]) {
  case 'i':
    {
      ecadebug->control_flow("Eca-audio-objects/Adding a new input");
      last_audio_object = create_audio_object(tname, si_read, default_audio_format(), buffersize());
      add_input(last_audio_object);
      break;
    }

  case 'o':
    {
      ecadebug->control_flow("Eca-audio-objects/Adding a new output");
      last_audio_object = create_audio_object(tname, output_openmode(), default_audio_format(), buffersize());
      add_output(last_audio_object);
      break;
    }

  case 'y':
    {
      last_audio_object->seek_position_in_seconds(atof(get_argument_number(1, argu).c_str()));
      if (last_audio_object->io_mode() == si_read) {
	input_start_pos[input_start_pos.size() - 1] = last_audio_object->position_in_seconds_exact();
      }
      else {
	output_start_pos[output_start_pos.size() - 1] = last_audio_object->position_in_seconds_exact();
      }

      ecadebug->msg("(eca-audio-objects) Set starting position for audio object \""
		    + last_audio_object->label() 
		    + "\": "
		    + kvu_numtostr(last_audio_object->position_in_seconds_exact()) 
		    + " seconds.");
      break;
    }

  default: { }
  }
}

int ECA_AUDIO_OBJECTS::get_type_from_extension(const string& filename) const {
  int typevar = TYPE_UNKNOWN;
  string teksti = filename;
  to_lowercase(teksti);

  if (strstr(teksti.c_str(),".wav") != 0) typevar = TYPE_WAVE;
  else if (strstr(teksti.c_str(),".ewf") != 0) typevar = TYPE_EWF;
  else if (strstr(teksti.c_str(),".cdr") != 0) typevar = TYPE_CDR;
  else if (strstr(teksti.c_str(),".raw") != 0) typevar = TYPE_RAWFILE;

  else if (strstr(teksti.c_str(),"/dev/dsp") != 0) typevar = TYPE_OSS;
  else if (strstr(teksti.c_str(),"alsalb") != 0) typevar = TYPE_ALSALOOPBACK;
  else if (strstr(teksti.c_str(),"alsa") != 0) typevar = TYPE_ALSA;

  else if (strstr(teksti.c_str(),".mp3") != 0) typevar = TYPE_MP3;
  else if (strstr(teksti.c_str(),".mp2") != 0) typevar = TYPE_MP3;

  else if (strstr(teksti.c_str(),".669") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".amf") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".dsm") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".far") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".gdm") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".imf") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".it") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".m15") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".med") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".mod") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".mod") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".mtm") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".s3m") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".stm") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".stx") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".ult") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".uni") != 0) typevar = TYPE_MIKMOD;
  else if (strstr(teksti.c_str(),".xm") != 0) typevar = TYPE_MIKMOD;

  else if (strstr(teksti.c_str(),"/dev/snd/pcm") != 0) typevar = TYPE_ALSAFILE;
  else if (strstr(teksti.c_str(),".aif") != 0) typevar = TYPE_AUDIOFILE;
  else if (strstr(teksti.c_str(),".au") != 0) typevar = TYPE_AUDIOFILE;
  else if (strstr(teksti.c_str(),".snd") != 0) typevar = TYPE_AUDIOFILE;


  else if (teksti == "stdin") typevar = TYPE_STDIN;
  else if (teksti == "stdout") typevar = TYPE_STDOUT;
  else if (strstr(teksti.c_str(),"rtnull") != 0) typevar = TYPE_RTNULL;
  else if (strstr(teksti.c_str(),"null") != 0) typevar = TYPE_NULL;

  return(typevar);
}

AUDIO_IO* ECA_AUDIO_OBJECTS::create_audio_object(const string& argu, 
						 const SIMODE mode, 
						 const ECA_AUDIO_FORMAT& format,
						 long int buffersize_arg) const throw(ECA_ERROR*) {
  // --------
  // require:
  assert(argu.empty() != true && 
	 buffersize_arg > 0);
  // --------

  if (argu.size() == 0) {
    throw(new ECA_ERROR("ECA-AUDIO_OBJECTS","Tried to create a audio device with null filename."));
  }
  string tname = get_argument_number(1, argu);

  AUDIO_IO* main_file;
  int type = get_type_from_extension(tname);
  switch(type) {
  case TYPE_WAVE:
    main_file = new WAVEFILE (tname, mode, format, double_buffering());
    break;
    
  case TYPE_CDR:
    main_file = new CDRFILE (tname, mode, format);
    break;
    
  case TYPE_OSS:
#ifdef COMPILE_OSS
    if (mode != si_read) 
      main_file = new OSSDEVICE (tname, si_write, format, buffersize_arg, precise_sample_rates());
    else
      main_file = new OSSDEVICE (tname, mode, format, buffersize_arg, precise_sample_rates());
#endif
    break;
    
  case TYPE_EWF:
    main_file = new EWFFILE (tname, mode, format);
    break;
    
    //  case TYPE_OSSDMA:
    // #ifdef COMPILE_OSS
    //    main_file = new OSSDMA (tname, mode, format, buffersize_arg);
    // #endif
    //    break;
    
  case TYPE_MP3:
    if (mode != si_read) 
      main_file = new MP3FILE (tname, si_write, format);
    else
      main_file = new MP3FILE (tname, mode, format);
    break;

  case TYPE_MIKMOD:
    if (mode == si_read) 
      main_file = new MIKMOD_INTERFACE (tname, si_read, format);
    break;

  case TYPE_ALSAFILE:
    {
      string cardstr,devicestr;
      string::const_iterator p = tname.begin();
      while(p != tname.end() && *p != 'C') ++p;
      ++p;
      while(p != tname.end() && isdigit(*p)) {
	cardstr += " ";
	cardstr[cardstr.size() - 1] = *p;
	++p;
      }
      while(p != tname.end() && *p != 'D') ++p;
      ++p;
      while(p != tname.end() && isdigit(*p)) {
	devicestr += " ";
	devicestr[devicestr.size() - 1] = *p;
	++p;
      }
      
      int card = atoi(cardstr.c_str());
      int device = atoi(devicestr.c_str());

#ifdef ALSALIB_050
      if (mode != si_read)
	main_file = new ALSA_PCM2_DEVICE (card, device, si_write, format,
					  buffersize_arg);
      else
	main_file = new ALSA_PCM2_DEVICE (card, device, mode, format, buffersize_arg);
#else
      if (mode != si_read)
	main_file = new ALSA_PCM_DEVICE (card, device, si_write, format,
				    buffersize_arg);
      else
	main_file = new ALSA_PCM_DEVICE (card, device, mode, format, buffersize_arg);
#endif
      break;
    }

  case TYPE_ALSALOOPBACK:
    {
      int card = atoi(get_argument_number(2, argu).c_str());
      int device = atoi(get_argument_number(3, argu).c_str());
      bool loop_mode = true;
      
      if (get_argument_number(4, argu) == "c") {
	loop_mode = false;
      }

#ifdef COMPILE_ALSA
      if (mode != si_read)
	main_file = new ALSA_LOOPBACK_DEVICE (card, device, si_write, format, buffersize_arg, loop_mode);
      else
	main_file = new ALSA_LOOPBACK_DEVICE (card, device, mode, format, buffersize_arg, loop_mode);
#endif
      break;
    }

  case TYPE_ALSA:
    {
      int card = atoi(get_argument_number(2, argu).c_str());
      int device = atoi(get_argument_number(3, argu).c_str());

#ifdef ALSALIB_050
      if (mode != si_read)
	main_file = new ALSA_PCM2_DEVICE (card, device, si_write, format,
					  buffersize_arg);
      else
	main_file = new ALSA_PCM2_DEVICE (card, device, mode, format, buffersize_arg);
#else
      if (mode != si_read)
	main_file = new ALSA_PCM_DEVICE (card, device, si_write, format,
					 buffersize_arg);
      else
	main_file = new ALSA_PCM_DEVICE (card, device, mode, format, buffersize_arg);
#endif
      break;
    }

  case TYPE_AUDIOFILE:
    {
#ifdef COMPILE_AF
      if (mode != si_read) 
	main_file = new AUDIOFILE_INTERFACE (tname, si_write, format);
      else
	main_file = new AUDIOFILE_INTERFACE (tname, mode, format);
#endif
      break;
    }

  case TYPE_RAWFILE:
    {
      main_file = new RAWFILE (tname, mode, format, double_buffering());
      break;
    }

  case TYPE_STDIN:
    {
      main_file = new RAWFILE ("-", si_read, format);
      break;
    }

  case TYPE_STDOUT:
    {
      main_file = new RAWFILE ("-", si_write, format);
      break;
    }

  case TYPE_RTNULL:
    {
      main_file = new REALTIME_NULL ("rtnull", mode, format, buffersize_arg);
      break;
    }

  case TYPE_NULL:
    {
      main_file = new NULLFILE ("null", mode, format);
      break;
    }

  default: 
    {
      throw(new ECA_ERROR("ECA_AUDIO_OBJECTS", "unknown file format; unable to open file " + tname + "."));
    }
  }
  return(main_file);
}

void ECA_AUDIO_OBJECTS::add_default_chain(void) {
  // --------
  // require:
  assert(buffersize() >= 0 && chains.size() == 0);
  // --------

  chains.push_back(new CHAIN(buffersize(), SAMPLE_BUFFER::channel_count_default));
  chains.back()->name("default");
  ecadebug->msg(1,"add_default_chain() ");
  selected_chainids.push_back("default");

  // --------
  // ensure:
  assert(chains.back()->name() == "default" &&
	 selected_chainids.back() == "default");
  // --------
}

void ECA_AUDIO_OBJECTS::add_new_chains(const vector<string>& newchains) {
  for(vector<string>::const_iterator p = newchains.begin(); p != newchains.end(); p++) {
    bool exists = false;
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*p == (*q)->name()) exists = true;
    }
    if (exists == false) {
      chains.push_back(new CHAIN(buffersize(), SAMPLE_BUFFER::channel_count_default));
      chains.back()->name(*p);
      ecadebug->msg(1,"add_new_chains() added chain " + *p);
    }
  }
}

void ECA_AUDIO_OBJECTS::remove_chains(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	delete *q;
	chains.erase(q);
	break;
      }
    }
  }
  selected_chainids.resize(0);
}

void ECA_AUDIO_OBJECTS::clear_chains(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->clear();
      }
    }
  }
}

void ECA_AUDIO_OBJECTS::rename_chain(const string& name) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	(*q)->name(name);
	return;
      }
    }
  }
}

void ECA_AUDIO_OBJECTS::select_all_chains(void) {
  vector<CHAIN*>::const_iterator p = chains.begin();
  selected_chainids.resize(0);
  while(p != chains.end()) {
    selected_chainids.push_back((*p)->name());
    ++p;
  }
}

void ECA_AUDIO_OBJECTS::toggle_chain_muting(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	if ((*q)->is_muted()) 
	  (*q)->toggle_muting(false);
	else 
	  (*q)->toggle_muting(true);
      }
    }
  }
}

void ECA_AUDIO_OBJECTS::toggle_chain_bypass(void) {
  for(vector<string>::const_iterator a = selected_chainids.begin(); a != selected_chainids.end(); a++) {
    for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
      if (*a == (*q)->name()) {
	if ((*q)->is_processing()) 
	  (*q)->toggle_processing(false);
	else 
	  (*q)->toggle_processing(true);
      }
    }
  }
}

vector<string>
ECA_AUDIO_OBJECTS::get_connected_chains_to_input(AUDIO_IO* aiod) const{ 
  vector<string> res;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->input_id) {
      res.push_back((*q)->name());
    }
    ++q;
  }
  
  return(res); 
}

vector<string>
ECA_AUDIO_OBJECTS::get_connected_chains_to_output(AUDIO_IO* aiod) const { 
  vector<string> res;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->output_id) {
      res.push_back((*q)->name());
    }
    ++q;
  }

  return(res); 
}

int ECA_AUDIO_OBJECTS::number_of_connected_chains_to_input(AUDIO_IO* aiod) const {
  int count = 0;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->input_id) {
      ++count;
    }
    ++q;
  }

  return(count); 
}

int ECA_AUDIO_OBJECTS::number_of_connected_chains_to_output(AUDIO_IO* aiod) const {
  int count = 0;
  
  vector<CHAIN*>::const_iterator q = chains.begin();
  while(q != chains.end()) {
    if (aiod == (*q)->output_id) {
      ++count;
    }
    ++q;
  }

  return(count); 
}

vector<string> ECA_AUDIO_OBJECTS::get_connected_chains_to_iodev(const
							     string&
							     filename) const
  {
  vector<AUDIO_IO*>::size_type p;

  p = 0;
  while (p < inputs.size()) {
    if (inputs[p]->label() == filename)
      return(get_connected_chains_to_input(inputs[p]));
    ++p;
  }

  p = 0;
  while (p < outputs.size()) {
    if (outputs[p]->label() == filename)
      return(get_connected_chains_to_output(outputs[p]));
    ++p;
  }
  return(vector<string> (0));
}


void ECA_AUDIO_OBJECTS::add_input(AUDIO_IO* aio) {
  // --------
  // require:
  assert(aio != 0);
  assert(chains.size() > 0);
  // --------

  inputs.push_back(aio);
  input_start_pos.push_back(0);
  ecadebug->msg(audio_object_info(aio));
  attach_input_to_selected_chains(aio->label());

  // --------
  // ensure:
  assert(inputs.size() > 0);
  // --------
}

void ECA_AUDIO_OBJECTS::add_output(AUDIO_IO* aiod) {
  // --------
  // require:
  assert(aiod != 0 && chains.size() > 0);
  // --------

  outputs.push_back(aiod);
  output_start_pos.push_back(0);
  ecadebug->msg(audio_object_info(aiod));
  attach_output_to_selected_chains(aiod->label());

  // --------
  // ensure:
  assert(outputs.size() > 0);
  // --------
}

string ECA_AUDIO_OBJECTS::audio_object_info(const AUDIO_IO* aio) const {
  // --------
  // require:
  assert(aio != 0);
  // --------

  string temp = "(eca-audio-objects) Added audio object \"" + aio->label();
  temp += "\", mode \"";
  if (aio->io_mode() == si_read) temp += "read";
  if (aio->io_mode() == si_write) temp += "write";
  if (aio->io_mode() == si_readwrite) temp += "read/write";
  temp += "\".\n";
  temp += aio->format_info();
  return(temp);
}

void ECA_AUDIO_OBJECTS::remove_audio_input(const string& label) { 
  vector<AUDIO_IO*>::iterator ci = inputs.begin();
  while (ci != inputs.end()) {
    if ((*ci)->label() == label) {
      vector<CHAIN*>::iterator q = chains.begin();
      while(q != chains.end()) {
	if ((*q)->input_id == *ci) (*q)->disconnect_input();
	++q;
      }
      delete *ci;
      (*ci) = new NULLFILE("null", si_read);
      //      ecadebug->msg("(eca-chainsetup) Removing input " + label + ".");
    }
    ++ci;
  }
}

void ECA_AUDIO_OBJECTS::remove_audio_output(const string& label) { 
  vector<AUDIO_IO*>::iterator ci = outputs.begin();
  while (ci != outputs.end()) {
    if ((*ci)->label() == label) {
      vector<CHAIN*>::iterator q = chains.begin();
      while(q != chains.end()) {
	if ((*q)->output_id == *ci) (*q)->disconnect_output();
	++q;
      }
      delete *ci;
      (*ci) = new NULLFILE("null", si_write);
    }
    ++ci;
  }
}

string ECA_AUDIO_OBJECTS::inputs_to_string(void) const { 
  MESSAGE_ITEM t; 
  t.setprecision(3);
  int p = 0;
  while (p < inputs.size()) {
    t << "-a:";
    vector<string> c = get_connected_chains_to_input(inputs[p]);
    vector<string>::const_iterator cp = c.begin();
    while (cp != c.end()) {
      t << *cp;
      ++cp;
      if (cp != c.end()) t << ",";
    }
    t << " ";
    t << audioio_to_string(inputs[p], "i");

    if (input_start_pos[p] != 0) {
      t << " -y:" << input_start_pos[p];
    }

    t << "\n";
    ++p;
  }

  return(t.to_string());
}

string ECA_AUDIO_OBJECTS::outputs_to_string(void) const { 
  MESSAGE_ITEM t; 
  t.setprecision(3);
  vector<AUDIO_IO*>::size_type p = 0;
  while (p < outputs.size()) {
    t << "-a:";
    vector<string> c = get_connected_chains_to_output(outputs[p]);
    vector<string>::const_iterator cp = c.begin();
    while (cp != c.end()) {
      t << *cp;
      ++cp;
      if (cp != c.end()) t << ",";
    }
    t << " ";
    t << audioio_to_string(outputs[p], "o");

    if (output_start_pos[p] != 0) {
      t << " -y:" << output_start_pos[p];
    }

    t << "\n";
    ++p;

//    if (startpos_map.find(id) != startpos_map.end()) {
//      
//    }
  }

  return(t.to_string());
}

string ECA_AUDIO_OBJECTS::chains_to_string(void) const { 
  MESSAGE_ITEM t;

  vector<CHAIN*>::size_type p = 0;
  while (p < chains.size()) {
    t << "-a:" << chains[p]->name() << " ";
    t << chains[p]->to_string();
    ++p;
    if (p < chains.size()) t << "\n";
  }

  return(t.to_string());
}

string ECA_AUDIO_OBJECTS::audioio_to_string(const AUDIO_IO* aiod, const string& direction) const {
  MESSAGE_ITEM t;

  t << "-f:" << aiod->format_string() << "," <<
    aiod->channels() << ","  << aiod->samples_per_second();
  t << " -" << direction << " ";
  t << aiod->label();

  return(t.to_string());
}

const CHAIN* ECA_AUDIO_OBJECTS::get_chain_with_name(const string& name) const {
  vector<CHAIN*>::const_iterator p = chains.begin();
  while(p != chains.end()) {
    if ((*p)->name() == name) return(*p);
    ++p;
  }
  return(0);
}

void ECA_AUDIO_OBJECTS::attach_input_to_selected_chains(const string& filename) {
  string temp;
  vector<AUDIO_IO*>::size_type c = 0;

  while (c < inputs.size()) {
    if (inputs[c]->label() == filename) {
      temp += "(eca-chainsetup) Assigning file to chains:";
      for(vector<string>::const_iterator p = selected_chainids.begin(); p!= selected_chainids.end(); p++) {
	for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	  if (*p == (*q)->name()) {
	    (*q)->connect_input(inputs[c]);
	    temp += " " + *p;
	  }
	}
      }
    }
    ++c;
  }
  ecadebug->msg(temp);
}

void ECA_AUDIO_OBJECTS::attach_output_to_selected_chains(const string& filename) {
  string temp;
  vector<AUDIO_IO*>::size_type c = 0;

  while (c < outputs.size()) {
    if (outputs[c]->label() == filename) {
      temp += "(eca-chainsetup) Assigning file to chains:";
      for(vector<string>::const_iterator p = selected_chainids.begin(); p!= selected_chainids.end(); p++) {
	for(vector<CHAIN*>::iterator q = chains.begin(); q != chains.end(); q++) {
	  if (*p == (*q)->name()) {
	    (*q)->connect_output(outputs[c]);
	    temp += " " + *p;
	  }
	}
      }
    }
    ++c;
  }
  ecadebug->msg(temp);
}
